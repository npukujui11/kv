#include "skiplist.h"
#include "LRU.h"
#include <chrono>
#include <thread>
#include <mutex>

#define DEFAULT_TTL 3600 // 默认过期时间

// 继承Node类
template <typename K, typename V>
class NodeWithTTL : public Node {
public:
    using TimePoint = std::chrono::steady_clock::time_point;

    NodeWithTTL() {} // 默认构造函数
    NodeWithTTL(K k, V v, int, TimePoint t); // 构造函数
    ~NodeWithTTL() {} // 析构函数
    TimePoint get_expire_time() const; // 获取过期时间
    void set_expire_time(TimePoint t); // 设置过期时间

private:
    K key; // 键
    
    V value; // 值

    TimePoint expiration_time; // 过期时间
};

/*
 * 构造函数
 * @param key 键
 * @param value 值
 * @param level 节点层级
 * @param expiration_time 过期时间
 * @return
 */
template <typename K, typename V>
NodeWithTTL<K, V>::NodeWithTTL(K key, V value, int level, TimePoint expiration_time) { 
    this->key = key;
    this->value = value;
    this->node_level = level;
    this->expire_time = expiration_time;
    this->forward = new NodeWithTTL<K, V>*[level + 1];
    memset(forward, 0, sizeof(NodeWithTTL<K, V>*) * (level + 1));
};

/*
 * 析构函数
 * @return
 */
template <typename K, typename V>
NodeWithTTL<K, V>::~NodeWithTTL() {
    delete[] forward;
};

/*
 * 获取过期时间
 * @return 过期时间
 */
template <typename K, typename V>
typename NodeWithTTL<K, V>::TimePoint NodeWithTTL<K, V>::get_expire_time() const {
    return expiration_time;
};

/*
 * 设置过期时间
 * @param t 过期时间
 * @return
 */
template <typename K, typename V>
void NodeWithTTL<K, V>::set_expire_time(TimePoint expiration_time) {
    this->expiration_time = expiration_time;
};


template <typename K, typename V>
class SkipListWithCache : public SkipList { 
public: 
    // 构造函数
    SkipListWithCache(int) {}
    
    ~SkipListWithCache() {} // 析构函数
    
    int insert_element(const K& key, const V& value, int ttl_seconds); // 插入数据
    NodeWithTTL<K, V>* create_node(const K& key, const V& value, int level, int ttl_seconds);
    bool get(const K& key, V& value); // 获取数据
    bool search(const K& key, V& value); // 查找数据
    bool is_expired(const typename NodeWithTTL<K, V>::TimePoint& expiration_time) const; // 是否过期
    void remove_cache_expired(); // 定期删除缓存数据
    void remove_skiplist_expired(); // 定期删除跳表数据
    void persist_data(); // 数据持久化
    void load_data(); // 数据加载
    void display_skiplist(); // 打印跳表
    void periodic_save(SkipListWithCache<int, std::string>& kv_store); // 周期性数据持久化策略
    void periodic_cleanup(SkipListWithCache<int, std::string>& kv_store); // 周期性删除过期数据
    void clear(NodeWithTTL<K, V>* node); // 递归删除跳表节点
    int size(); // 获取元素个数

private:
    void get_key_value_from_string(const std::string& line, K& key, V& value); // 从字符串中获取键值对
    bool is_valid_string(const std::string& str); // 是否为有效字符串

    int _max_level; // 最大层级
    int _skip_list_level; // 跳表层级
    NodeWithTTL<K, V>* _header; // 头节点

    // file operation
    std::ofstream _file_writer; // 文件写入
    std::ifstream _file_reader; // 文件读取

    // skiplist current element count
    int _element_count; // 元素个数

    LRUCache<K, V> cache; // 缓存
};

/*
 * 构造函数
 * @return
 */
template <typename K, typename V>
SkipListWithCache<K, V>::SkipListWithCache(int max_level) {
    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;
    
    // 创建头节点
    K k;
    V v;
    this->_header = new NodeWithTTL<K, V>(k, v, _max_level, std::chrono::steady_clock::now()); // 创建头节点
};

/*
 * 析构函数
 * @return
 */
template <typename K, typename V>
~SkipListWithCache<K, V>::SkipListWithCache() {
    
    if (_file_reader.is_open()) {
        _file_reader.close();
    }
    if (_file_writer.is_open()) {
        _file_writer.close();
    }

    // 递归删除跳表节点
    if(_header->forward[0] != nullptr) {
        clear(_header->forward[0]);
    }

    delete[] _header->forward;
    
    delete(_header);
    delete(cache);
};

/*
 * 递归删除跳表节点
 * @param node 节点
 * @return
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::clear(NodeWithTTL<K, V>* current) {
    if (current->forward[0] != nullptr) {
        clear(current->forward[0]);
    }
    delete current;
};



/*
 * 创建节点
 * @param key 键
 * @param value 值
 * @param level 层级
 * @param ttl_seconds 过期时间
 * @return 节点
 */
template <typename K, typename V>
NodeWithTTL<K, V>* SkipListWithCache<K, V>::create_node(const K& key, const V& value, int level, int ttl_seconds) { 
    expiration_time = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_seconds);
    NodeWithTTL<K, V>* n = new NodeWithTTL<K, V>(key, value, level, expiration_time);
    return n;
}

/*
 * 插入元素，同时插入缓存
 * @param key 键
 * @param value 值
 * @param ttl_seconds 过期时间
 * @return void
 * @remark 插入数据到跳表和缓存，并设置过期时间
 */
template <typename K, typename V>
int SkipListWithCache<K, V>::insert_element(const K& key, const V& value, int ttl_seconds) {
    
    mtx.lock(); // 加锁

    NodeWithTTL<K, V>* current = this->_header; // 当前节点
    NodeWithTTL<K, V>* update[_max_level + 1]; // 更新节点
    memset(update, 0, sizeof(NodeWithTTL<K, V>*) * (_max_level + 1)); // 初始化更新节点

    // start from highest level of skip list
    for (int i = _skip_list_level; i >= 0; i--) { 
        while (current->forward[i] != nullptr && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];

    if (current != nullptr && current->get_key() == key) {
        std::cout << "key: " << key << ", exists" << std::endl;
        mtx.unlock(); // 解锁
        return 1;
    }

    if (current == nullptr || current->get_key() != key) { 
        int random_level = get_random_level(); // 随机层级

        if (random_level > _skip_list_level) { // 如果随机层级大于当前层级
            for (int i = _skip_list_level + 1; i < random_level + 1; i++) {
                update[i] = _header; // 更新节点
            }
            _skip_list_level = random_level;
        }

        NodeWithTTL<K, V>* inserted_node = create_node(key, value, random_level, ttl_seconds); // 创建节点

        for (int i = 0; i <= random_level; i++) {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        std::cout << "Successfully inserted key: " << key << ", value: " << value << ", level: " << random_level << ", ttl: " << ttl_seconds << std::endl;
        _element_count++; // 元素个数加1
    }

    mtx.unlock(); // 解锁
    cache.put(key, value, ttl_seconds); // 插入数据到缓存

    return 0;
};

/*
 * 获取元素，先从缓存中获取，如果缓存中没有，再从跳表中获取
 * @param key 键
 * @param value 值
 * @return bool
 * @remark 从缓存中获取数据，如果缓存中没有，再从跳表中获取
 */
template <typename K, typename V>
bool SkipListWithCache<K, V>::get(const K& key, V& value) {
    // 从缓存中获取数据
    if (cache.get(key, value)) { 
        return true;
    }

    // 从跳表中获取数据
    if (skiplist.search_element(key)) { 
        value = skiplist.getKey();
        // 插入数据到缓存
        cache.put(key, value, DEFAULT_TTL);
        return true;
    }

    return false;
};

/*
 * 删除过期缓存数据
 * @return void
 * @remark 删除过期缓存数据
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::remove_cache_expired() {
    cache.remove_expired();
};

/*
 * 判断节点是否过期
 * @param expiration_time 过期时间
 * @return bool
 * @remark 判断节点是否过期
 */
template <typename K, typename V>
bool SkipListWithCache<K, V>::is_expired(const typename NodeWithTTL<K, V>::TimePoint& expiration_time) const {
    return expiration_time < std::chrono::system_clock::now();
};

/*
 * 删除过期跳表数据
 * @return void
 * @remark 删除过期跳表数据
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::remove_skiplist_expired() {
    
}

/*
 * 数据持久化
 * @return void
 * @remark 数据持久化
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::persist_data() {
    skiplist.persist_data();
}

/*
 * 打印跳表
 * @return void
 * @remark 打印跳表
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::display_skiplist() {
    skiplist.display();
}

/*
 * 周期性数据持久化策略
 * @param kv_store kv存储
 * @return void
 * @remark 周期性数据持久化策略
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::periodic_save(SkipListWithCache<int, std::string>& kv_store) {
    while (true) {
        this_thread::sleep_for(chrono::minutes(1)); // 每1分钟存盘
        kv_store.persist_data(); // 数据定期持久化
    }
}

/*
 * 周期性删除过期数据
 * @param kv_store kv存储
 * @return void
 * @remark 周期性删除过期数据
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::periodic_cleanup(SkipListWithCache<int, std::string>& kv_store) {
    while (true) {
        this_thread::sleep_for(chrono::minutes(1)); // 每1分钟删除过期数据
        kv_store.remove_skiplist_expired(); // 删除过期数据
    }
}
