#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <cmath>
#include <mutex>

# define STORE_FILE "store/dump.txt" // 存储文件

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
    void display_list_prettily(); // 以更美观的方式显示跳表
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

private:
    bool is_valid_string(const std::string& str); // 判断字符串是否为有效字符串
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value); // 从字符串中获取键值对
};

/**
 * 构造函数
 * @param max_level 跳表的最大层数
 * @return void 
 */

template <typename K, typename V>
SkipList<K, V>::SkipList(int max_level) {
    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;

    // 创建头节点
    K k{}; // 使用默认初始化，不然编译器可能会报未初始化变量的错误
    V v{}; 
    this->_header = create_node(k, v, max_level);
}

template <typename K, typename V>
SkipList<K, V>::~SkipList() {
    
    if (_file_reader.is_open()) { // 关闭文件读取流
        _file_reader.close();
    }
    if (_file_writer.is_open()) { // 关闭文件写入流
        _file_writer.close();
    }

    // 清空跳表
    if (_header->forward[0] != nullptr) { 
        clear(_header->forward[0]);
    }
    delete _header; // 释放头节点的空间
}

template <typename K, typename V>
int SkipList<K, V>::get_random_level() {
    int k = 1; // 初始化层级，每个节点至少出现在第一层

    while (rand() % 2) { // 生成随机数，如果是奇数，则层级+1
        k++;
    }
    // 返回层级，但不能超过最大层级
    return (k < _max_level) ? k : _max_level;
}

/**
 * 创建节点
 * @param k 节点的键
 * @param v 节点的值
 * @param level 节点的层数
 * @return Node<K, V>* 返回创建的节点
 */
template <typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K key, const V value, const int level) { 
    Node<K, V>* n = new Node<K, V>(key, value, level); // 创建节点
    return n;
}

// Insert given key and value in skip list 
// return 1 means element exists  
// return 0 means insert successfully
/* 
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+

*/

/**
 * 插入元素
 * @param key 要插入的元素的键
 * @param value 要插入的元素的值
 * @return 如果元素已存在，返回 1；否则，进行更新 value 操作并返回 0。
 * @description 插入元素的过程是：
 *                  1. 确定节点层级；
 *                  2. 从头节点开始，查找每一层的节点，找到插入位置；
 *                  3. 更新每一层的节点的指针。
*/
template <typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value) {

    mtx.lock(); 
    Node<K, V>* current = this->_header; // 从头节点开始

    Node<K, V>* update[_max_level + 1]; // 用于记录每一层中待更新指针的节点
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1)); // 初始化 update 数组

    // 从最大层级开始，逐层查找节点
    for (int i = _skip_list_level; i >= 0; i--) { 
        while (current->forward[i] != nullptr && current->forward[i]->getKey() < key) {
            current = current->forward[i];
        }
        // 记录每一层中待更新指针的节点
        update[i] = current; 
    }

    // 移动到最底层的下一个节点，准备插入操作
    current = current->forward[0];
    // 检查待插入节点的键是否已经存在
    if (current != nullptr && current->getKey() == key) { 
        /*
         * 这里可以考虑插入覆盖操作
         * current->set_value(value);
         */
        //std::cout << "Element with key " << key << " already exists." << std::endl;
        mtx.unlock(); 
        return 1; // 元素已存在
    } else { // 键不存在或者当前节点为空
    // current == nullptr || current->get_key() != key
        int random_level = get_random_level(); // 生成随机层级
        // 如果随机层级大于当前跳表的层级，则更新 update 数组
        if (random_level > _skip_list_level) { 
            // 对所有新的更高层级，将头节点设置为它们的前驱节点
            for (int i = _skip_list_level + 1; i < random_level + 1; i++) { 
                update[i] = _header;
            }
            _skip_list_level = random_level; // 更新跳表的层级
        } 

        // 创建新节点
        Node<K, V>* inserted_node = create_node(key, value, random_level);
        // 更新每一层的节点的指针
        for (int i = 0; i <= random_level; i++) { 
            // 新节点指向当前节点的下一个节点
            inserted_node->forward[i] = update[i]->forward[i];
            // 当前节点指向新节点
            update[i]->forward[i] = inserted_node;
        }
        //std::cout << "Element with key " << key << " inserted successfully." << std::endl;
        _element_count++; // 更新元素计数
    }
    mtx.unlock();
    return 0; // 插入成功
}

// Display skip list
/**
 * 打印跳表
 * @param void
 * @return void
 * @description 遍历每一层的节点，输出节点的键和值 
 */
template <typename K, typename V>
void SkipList<K, V>::display_list() { 
    std::cout << "\n*****Skip List*****" << "\n";
    // 遍历每一层
    for (int i = _skip_list_level - 1; i >= 0; i--) { 
        Node<K, V>* node = this->_header->forward[i];
        std::cout << "Level " << i << ": _header ";
        // 遍历每一层的节点
        while (node != nullptr) { 
            std::cout << node->getKey() << ":" << node->getValue() << " ";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}


// Display skip list prettily
/**
 * 整齐打印跳表
 * @param void
 * @return void
 * @description 遍历每一层的节点，输出节点的键和值 
 */
template <typename K, typename V>
void SkipList<K, V>::display_list_prettily() { 
    
    std::cout << "\n*****Skip List*****" << "\n";
    // 遍历每一层
    for (int i = _skip_list_level - 1; i >= 0; i--) { 
        Node<K, V>* node = this->_header->forward[i];
        std::cout << "Level " << i << ": _header ";
        Node<K, V>* prev_node = this->_header;

        // 遍历每一层的节点
        while (node != nullptr) { 

            int key_diff = 0;                          // 计算相邻节点之间的 key 差值
            int total_length_between_nodes = 0;        // 计算中间节点之间的总长度
            Node<K, V>* temp_node = prev_node;         // 临时节点

            while (temp_node != node) { 

                std::stringstream temp_ss_key, temp_ss_value;
                temp_ss_key << temp_node->getKey();
                temp_ss_value << temp_node->getValue();

                key_diff++;
                if (key_diff > 1) {
                    total_length_between_nodes += temp_ss_key.str().length() + temp_ss_value.str().length();
                } else {
                    total_length_between_nodes = 0;
                }
                                       
                temp_node = temp_node->forward[0];
            }

            int arrow_length = 0;
            // 计算箭头长度：假设可以用 key 值之间的差值来决定距离
            if (key_diff == 1) {
                arrow_length = 2;
            } else {
                arrow_length = 2 * key_diff + 4 * (key_diff - 1) + total_length_between_nodes;
            }
                
            std::string arrows(arrow_length, '-');     // 生成箭头字符串

            std::cout << arrows << "> " << node->getKey() << ":" << node->getValue() << " ";
            
            prev_node = node;                           // 更新前一个节点
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

// Search for element in skip list 
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/

/**
 * 查找元素
 * @param key 要查找的元素的键
 * @return bool 如果找到返回true，否则返回false
*/
template <typename K, typename V>
bool SkipList<K, V>::search_element(K key) {

    //std::cout << "search_element-----------------" << std::endl;
    // 定义一个指针 current，初始化为跳表的头节点 _header
    Node<K, V>* current = _header;

    for (int i = _skip_list_level; i >= 0; i--) { // 从跳表的最高层开始查找
        // 遍历当前层级，直到下一个节点的键值大于要查找的键值
        while (current->forward[i] != nullptr && current->forward[i]->getKey() < key) {
            // 移动到下一个节点
            current = current->forward[i];
        }
        // 当前节点的下一个节点的键值大于待查找的键值时，进行下层操作
    }
    // 检查当前层（最底层）的下一个节点的键值是否为要查找的键值
    current = current->forward[0];
    if (current != nullptr && current->getKey() == key) { 
        //std::cout << "Found key: " << key << ", value: " << current->getValue() << std::endl;
        return true; // 找到了
    }

    //std::cout << " Not found key: " << key << std::endl;
    return false; // 没找到
}

/**
 * 删除跳表中的节点
 * @param key 要删除的节点的键
 * @return void 
 * @description 删除元素的过程是：
 *                  1. 定位待删除节点：通过搜索确定需要删除的节点位置；
 *                  2. 更新指针关系：调整相关节点的指针，以从跳表中移除目标节点；
 *                  3. 内存回收：释放被删除节点所占用的资源。
 */
template <typename K, typename V>
void SkipList<K, V>::delete_element(K key) { 
    mtx.lock(); // 加锁
    Node<K, V>* current = _header; // 从头节点开始

    Node<K, V>* update[_max_level + 1]; // 用于记录每一层中待更新指针的节点
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1)); // 初始化 update 数组

    // 从最大层级开始向下搜索待删除结点
    for (int i = _skip_list_level; i >= 0; i--) { 
        while (current->forward[i] != nullptr && current->forward[i]->getKey() < key) { 
            current = current->forward[i];
        }
        update[i] = current; // 记录每一层中待更新指针的节点
    }

    current = current->forward[0]; // 移动到底层的下一个节点
    // 检查待删除节点的键是否存在
    if (current != nullptr && current->getKey() == key) { 
        // 更新每一层的指针
        for (int i = 0; i <= _skip_list_level; i++) { 
            // 如果当前层的节点的下一个节点是待删除节点
            if (update[i]->forward[i] == current) { 
                // 将当前层的节点的下一个节点指向待删除节点的下一个节点
                update[i]->forward[i] = current->forward[i];
            } else { 
                break; // 如果当前层的节点的下一个节点不是待删除节点，说明待删除节点不存在，直接退出
            }
        }

        // 如果删除节点后，跳表的最高层没有节点了，降低跳表的层级
        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == nullptr) { 
            _skip_list_level--;
        }

        //std::cout << "Element with key " << key << " deleted successfully." << std::endl;
        delete current; // 释放被删除节点的内存
        _element_count--; // 更新元素计数
    }
    mtx.unlock(); // 解锁
    return;
}

// Dump data in memory to file
template <typename K, typename V>
void SkipList<K, V>::dump_file() { 
    
    std::cout << "Dumping data to file..." << std::endl;
    mtx.lock(); // 加锁
    _file_writer.open(STORE_FILE); // 打开文件，STORE_FILE是路径
    Node<K, V>* current  = this->_header->forward[0]; // 从头节点开始遍历

    while (current != nullptr) { 
        _file_writer << current->getKey() << ":" << current->getValue() << std::endl; // 将节点的键值对写入文件
        current = current->forward[0]; // 移动到下一个节点
    }
    mtx.unlock(); // 解锁
    _file_writer.flush(); // 刷新文件
    _file_writer.close(); // 关闭文件
    
    return;
}

// 验证字符串的合法性
template <typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) { 
    // 如果字符串str非空，并且字符串中包含分隔符delimiter，那么返回true
    // find()函数返回字符串中第一个匹配的位置，如果没有找到匹配的位置，则返回std::string::npos
    if (!str.empty()&& str.find(delimiter) != std::string::npos) { 
        return true;
    }

    return false;
}

// 将字符串分割为键和值
template <typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) { 
    
    if (!is_valid_string(str)) { 
        return;
    }

    *key = str.substr(0, str.find(delimiter)); // 获取键
    *value = str.substr(str.find(delimiter) + 1); // 获取值

    return;
}

template <typename K, typename V>
void SkipList<K, V>::load_file() {
    _file_reader.open(STORE_FILE); // 打开文件
    std::string line; // 用于存储文件中的每一行数据
    std::string *key = new std::string(); // 用于存储键
    std::string *value = new std::string(); // 用于存储值

    while (getline(_file_reader, line)) { // 逐行读取文件
        get_key_value_from_string(line, key, value); // 将每一行数据分割为键和值
        if (key->empty() || value->empty()) { // 如果键或值为空，跳过
            continue;
        }

        // stoi()函数将字符串转换为整数
        insert_element(stoi(*key), *value); // 将键值对插入跳表
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }

    delete key; // 释放内存
    delete value; // 释放内存
    _file_reader.close(); // 关闭文件
}