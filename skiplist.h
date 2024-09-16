#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <mutex>

std::mutex mtx; // 互斥锁
std::string delimiter = ":"; // 分隔符

/* ************************************************************************
> 跳表的节点类的实现
> 成员属性：
    > key：节点的键值
    > value：节点的值
    > forward：指针数组，用于指向后继节点
    > node_level：节点的层数
> public方法：
    > 构造函数：初始化节点
    > 析构函数：销毁节点
    > getKey：获取节点的键值
    > getValue：获取节点的值
    > setValue：设置节点的值
 ************************************************************************/

template <typename K, typename V>
class Node { 

public: 

    Node() {} // 默认构造函数

    Node(K k, V v, int); // 构造函数

    ~Node(); // 析构函数

    K getKey() const;  // get key

    V getValue() const; // get value

    void setValue(V);

    Node<K, V> **forward; // 在C++中，二维指针等价于指针数组

    int node_level; // 节点的层数

private:
    K key;
    V value;
};

/* ************************************************************************
> 关键点1. 同一节点的多层跳转：在相同键值、不同层级节点之间的跳转
> 关键点2. 不同节点的单层跳转：在相同层级、不同键值节点之间的跳转
> 通过这两种跳转方式，可以实现跳表的查找、插入、删除等操作

> 关键点1的解决方法：Node<K, V> **forward
> 描述：使用一个指针数组forward，数组中的每个指针对应节点在一个特定层级的后继节点
        通过变更数组下标，可以实现同一节点的多层跳转。
> 关键点2的解决方法：*forward
> 描述：参考单链表中的指针域，通过指针域实现节点之间的连接

> 例子：假设一个节点的层数为3，那么这个节点的forward指针数组的大小为3。其中forward[0]
        指向该节点在第0层的后继节点，forward[1]指向该节点在第1层的后继节点，forward[2]
 ************************************************************************/

template <typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) {
    this->key = k;
    this->value = v;
    this->node_level = level;

    // 申请指针数组的空间
    this->forward = new Node<K, V>*[level + 1]; // 申请指针数组的空间
    
    // Fill forward array with 0(NULL)
    memset(forward, 0, sizeof(Node<K, V>*) * (level + 1)); // 初始化指针数组
}

template <typename K, typename V>
Node<K, V>::~Node() {
    delete [] forward; // 释放指针数组的空间
}

template <typename K, typename V>
K Node<K, V>::getKey() const {
    return key;
}

template <typename K, typename V>
V Node<K, V>::getValue() const {
    return value;
}

template <typename K, typename V>
void Node<K, V>::setValue(V v) {
    this->value = v;
}

/************************************************************************
> 跳表类的实现
> 成员属性：
    > _max_level：跳表中允许的最大层数
    > _header：跳表的头节点，用于指向跳表中的第一个节点
    > _skip_list_level：跳表中的当前层数
    > _element_count：跳表中的节点数量
    > _file_writer & _file_reader：跳表生成持久化文件和读取持久化文件的写入器和读取器
> public方法：
    > 构造函数：初始化跳表
    > 析构函数：销毁跳表
    > get_random_level：生成一个随机层数
    > create_node：创建一个新的节点
    > insert_element：将节点插入到跳表中合适的位置
    > display_list：显示跳表中当前的节点的信息
    > search_element：从跳表中查找指定的元素
    > delete_element：从跳表中删除指定的元素
    > dump_file：将跳表的数据持久化到磁盘中
    > load_file：从磁盘加载持久化的数据到跳表中
    > clear：清空跳表，并回收其内存空间
    > size：返回跳表的元素个数
> private方法：
    > get_key_value_from_string：从字符串中获取键值对
    > is_valid_string：判断字符串是否为有效字符串
 ************************************************************************/

template <typename K, typename V>
class SkipList { 

public:
    SkipList(int);
    ~SkipList();
    int get_random_level(); // 生成随机层数（用于插入元素时决定该元素应该位于跳表的哪一层，是决定性能的关键。）
    Node<K, V>* create_node(K, V, int); // 创建节点
    int insert_element(K, V); // 插入元素
    void display_list(); // 显示跳表
    bool search_element(K); // 查找元素
    void delete_element(K); // 删除元素
    void dump_file(); // 将跳表持久化到文件
    void load_file(); // 从文件中加载跳表

    void clear(Node<K, V>*); // 清空跳表
    int size(); // 返回跳表的元素个数

private:
    int _max_level; // 跳表的最大层数

    int _skip_list_level; // 跳表的当前的最大层数

    Node<K, V> *_header; // 跳表的头节点

    std::ofstream _file_writer; // 文件写入流
    std::ifstream _file_reader; // 文件读取流

    int _element_count; // 跳表的元素个数

    void get_key_value_from_string(const std::string& str, std::string& key, std::string& value); // 从字符串中获取键值对

    bool is_valid_string(const std::string& str); // 判断字符串是否为有效字符串
};
