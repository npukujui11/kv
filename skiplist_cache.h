#include "skiplist.h"
#include "LRU.h"
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>

#define DEFAULT_TTL 3600 // 默认过期时间
#define PERMANENT_TTL -1 // 永久过期时间
#define DEFAULT_STORE_FILE "store/dumpFile_cache" // 数据持久化文件

std::atomic<bool> keep_running{true}; // 周期性数据持久化策略
std::atomic<bool> running_cleanup(false); // 用于控制清理线程是否运行

std::mutex FILE_IO_MUTEX; // 文件IO互斥锁

// 带过期时间的跳表节点
template <typename K, typename V>
class NodeWithTTL{
public:
    using TimePoint = std::chrono::steady_clock::time_point;

    NodeWithTTL() {} // 默认构造函数
    NodeWithTTL(K k, V v, int, TimePoint t); // 构造函数
    ~NodeWithTTL(); // 析构函数

    K getKey() const; // 获取键
    V getValue() const; // 获取值

    void setValue(V); // 设置值
    void setExpireTime(TimePoint t); // 设置过期时间
    TimePoint getExpireTime() const; // 获取过期时间
    int getRemainingTime() const; // 获取剩余时间
    NodeWithTTL<K, V>** forward; 
    int node_level; // 节点层级

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
    this->expiration_time = expiration_time;
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
typename NodeWithTTL<K, V>::TimePoint NodeWithTTL<K, V>::getExpireTime() const {
    return expiration_time;
};

/*
 * 设置过期时间
 * @param t 过期时间
 * @return
 */
template <typename K, typename V>
void NodeWithTTL<K, V>::setExpireTime(TimePoint expiration_time) {
    this->expiration_time = expiration_time;
};

/*
 * 获取剩余时间
 * @return 剩余时间
 */
template <typename K, typename V>
int NodeWithTTL<K, V>::getRemainingTime() const {
    return std::chrono::duration_cast<std::chrono::seconds>(expiration_time - std::chrono::steady_clock::now()).count();
}

/*
 * 获取键
 * @return 键
 */
template <typename K, typename V>
K NodeWithTTL<K, V>::getKey() const {
    return key;
};

/*
 * 获取值
 * @return 值
 */
template <typename K, typename V>
V NodeWithTTL<K, V>::getValue() const {
    return value;
};

/*
 * 设置值
 * @param value 值
 * @return
 */
template <typename K, typename V>
void NodeWithTTL<K, V>::setValue(V value) {
    this->value = value;
};

template <typename K, typename V>
class SkipListWithCache{ 
public: 
    // 构造函数
    SkipListWithCache(int, size_t);
    
    ~SkipListWithCache(); // 析构函数
    
    int get_random_level(); // 获取随机层级
    int insert_element(const K& key, const V& value, int ttl_seconds); // 插入数据
    NodeWithTTL<K, V>* create_node(const K& key, const V& value, int level, int ttl_seconds);
    bool search_element(const K& key); // 查找数据
    void delete_element(const K& key); // 删除数据
    bool is_expired(const typename NodeWithTTL<K, V>::TimePoint& expiration_time) const; // 是否过期
    void remove_cache_expired(); // 定期删除缓存数据
    void remove_skiplist_expired(); // 定期删除跳表数据
    void dump_file(); // 数据持久化
    void load_file(); // 数据加载
    void display_skiplist(); // 打印跳表
    void display_cache(); // 打印缓存
    void periodic_save(int t); // 周期性数据持久化策略
    void stop_periodic_save(); // 停止周期性数据持久化策略
    void periodic_cleanup(int t); // 周期性删除过期数据
    void stop_periodic_cleanup(); // 停止周期性删除过期数据
    void clear(NodeWithTTL<K, V>* node); // 递归删除跳表节点
    int size(); // 获取元素个数

private:
    void get_key_value_from_string(const std::string& line, std::string* key, std::string* value, std::string* expiration_time); // 从字符串中获取键值对
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
 * @param max_level 最大层级
 * @return
 */
template <typename K, typename V>
SkipListWithCache<K, V>::SkipListWithCache(int max_level, size_t cache_capacity) 
    : _max_level(max_level), _skip_list_level(0), _element_count(0), cache(cache_capacity) {
    this->_skip_list_level = 0;
    this->_element_count = 0;
    this->cache = LRUCache<K, V>(cache_capacity); // 创建缓存
    
    // 创建头节点
    K k{};
    V v{};
    this->_header = create_node(k, v, max_level, PERMANENT_TTL); // 创建头节点
};

/*
 * 析构函数
 * @return
 */
template <typename K, typename V>
SkipListWithCache<K, V>::~SkipListWithCache() {
    
    if (_file_reader.is_open()) {
        _file_reader.close();
    }
    if (_file_writer.is_open()) {
        _file_writer.close();
    }

    stop_periodic_cleanup(); // 停止周期性删除过期数据 
    stop_periodic_save(); // 停止周期性数据持久化策略

    // 递归删除跳表节点
    if(_header->forward[0] != nullptr) {
        clear(_header->forward[0]);
    }

    delete[] _header->forward;
    
    delete(_header);
    delete(cache);
};

/*
 * 获取随机层级
 * @return 随机层级
 */
template <typename K, typename V>
int SkipListWithCache<K, V>::get_random_level() {
    int k = 1;
    while (rand() % 2) {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
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
    typename NodeWithTTL<K, V>::TimePoint expiration_time;
    // 如果过期时间为永久
    if (ttl_seconds ==  PERMANENT_TTL) {
        // 过期时间为最大时间
        expiration_time = std::chrono::steady_clock::time_point::max();
    } else {
        // 过期时间为当前时间加上过期时间
        expiration_time = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_seconds);
    }
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
        while (current->forward[i] != nullptr && current->forward[i]->getKey() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];

    if (current != nullptr && current->getKey() == key) {
        //std::cout << "key: " << key << ", exists" << std::endl;
        mtx.unlock(); // 解锁
        return 1; // 已存在
    }

    if (current == nullptr || current->getKey() != key) { 
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

        //std::cout << "Successfully inserted key: " << key << ", value: " << value << ", level: " << random_level << ", ttl: " << ttl_seconds << std::endl;
        _element_count++; // 元素个数加1
    }

    mtx.unlock(); // 解锁
    cache.put(key, value, ttl_seconds); // 插入数据到缓存

    return 0; // 插入成功
};


/*
 * 查找元素，先从缓存中查找，如果缓存中没有，再从跳表中获取
 * @param key 键
 * @param value 值
 * @return bool
 * @remark 从缓存中获取数据，如果缓存中没有，再从跳表中获取，在跳表中查询时，惰性删除
 */
template <typename K, typename V>
bool SkipListWithCache<K, V>::search_element(const K& key) {

    std::cout << "search_element-----------------" << std::endl;
    NodeWithTTL<K, V>* current = this->_header; // 当前节点

    V value;

    // 从缓存中获取数据
    if (cache.get(key, value)) { 
        std::cout << "Found key: " << key << ", value: " << value << " from cache" << std::endl;
        return true; // 缓存中存在
    }

    // 从跳表中获取数据
    for (int i = _skip_list_level; i >= 0; i--) { 
        while (current->forward[i] != nullptr && current->forward[i]->getKey() < key) {
            current = current->forward[i];
        }
    }
    current = current->forward[0];
    if (current != nullptr && current->getKey() == key) { 
        // 如果节点过期，删除节点
        if (is_expired(current->getExpireTime())) {
            std::cout << "Found key: " << key << ", value: " << current->getValue() << " from skip list, but expired" << std::endl;
            delete_element(key);
            return false;
        }
        std::cout << "Found key: " << key << ", value: " << current->getValue() << " from skip list" << std::endl;
        return true;
    }
    
    std::cout << "Not found key: " << key << std::endl;
    return false; // 未找到
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
    return expiration_time < std::chrono::steady_clock::now();
};

/*
 * 删除过期跳表数据
 * @return void
 * @remark 删除过期跳表数据
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::remove_skiplist_expired() {
    NodeWithTTL<K, V>* current = this->_header;

    while (current->forward[0] != nullptr) { 
        if (is_expired(current->forward[0]->getExpireTime())) {
            NodeWithTTL<K, V>* expired_node = current->forward[0];
            // 删除节点
            delete_element(expired_node->getKey());
        } else {
            current = current->forward[0]; // 下一个节点
        }
    }
};


/*
 * 删除元素
 * @param key 键
 * @return void
 * @remark 删除元素
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::delete_element(const K& key) { 
    std::cout << "delete_element-----------------" << std::endl;
    mtx.lock(); // 加锁

    NodeWithTTL<K, V>* current = this->_header; // 当前节点
    NodeWithTTL<K, V>* update[_max_level + 1]; // 更新节点
    memset(update, 0, sizeof(NodeWithTTL<K, V>*) * (_max_level + 1));

    for (int i = _skip_list_level; i >= 0; i--) { 
        while (current->forward[i] != nullptr && current->forward[i]->getKey() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];

    if (current != nullptr && current->getKey() == key) { 
        for (int i = 0; i <= _skip_list_level; i++) { 
            if (update[i]->forward[i] != current) {
                break;
            }
            update[i]->forward[i] = current->forward[i];
        }

        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == nullptr) {
            _skip_list_level--;
        }

        std::cout << "Successfully deleted key: " << key << std::endl;
        _element_count--; // 元素个数减1
    }

    mtx.unlock(); // 解锁
    cache.remove(key);// 删除缓存中的数据
};

/*
 * 数据持久化
 * @return void
 * @remark 数据持久化
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::dump_file() {
    
    std::cout << "dump_file-----------------" << std::endl;
    FILE_IO_MUTEX.lock(); // 加锁
    _file_writer.open(DEFAULT_STORE_FILE); // 打开文件

    if (!_file_writer.is_open()) { 
        std::cerr << "Failed to open file: " << DEFAULT_STORE_FILE << std::endl;
        FILE_IO_MUTEX.unlock(); // 解锁
        return;
    }

    NodeWithTTL<K, V>* node = this->_header->forward[0]; // 当前节点

    while (node != nullptr) { 
        if (!is_expired(node->getExpireTime())) {
            _file_writer << node->getKey() << ":" << node->getValue() << ":" << node->getRemainingTime() << "\n";
            std::cout << node->getKey() << ":" << node->getValue() << ":" << node->getRemainingTime() << ";\n";
        }
        node = node->forward[0];
    }

    _file_writer.flush(); // 刷新文件
    _file_writer.close(); // 关闭文件

    FILE_IO_MUTEX.unlock(); // 解锁
    return;
}

/*
 * 从文件中加载数据
 * @return void
 * @remark 从文件中加载数据
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::load_file() { 

    FILE_IO_MUTEX.lock(); // 加锁
    std::cout << "Loading data from file..." << std::endl;
    _file_reader.open(DEFAULT_STORE_FILE); // 打开文件

    if (!_file_reader.is_open()) { 
        std::cerr << "Failed to open file: " << DEFAULT_STORE_FILE << std::endl;
        FILE_IO_MUTEX.unlock(); // 解锁
        return;
    }

    std::string line; // 行数据
    K* key = new K(); // 键
    V* value = new V(); // 值
    std::string* expiration_time = new std::string(); // 剩余时间 

    while (getline(_file_reader, line)) { 
        get_key_value_from_string(line, key, value, expiration_time); // 从字符串中获取键值对
        if (key->empty() || value->empty() || expiration_time->empty()) {
            continue;
        }

        insert_element(*key, *value, stoi(*expiration_time)); // 插入元素
        std::cout << "key: " << *key << ", " << "value: " << *value << ", " << "expiration_time: " << *expiration_time << std::endl;
    } 

    delete key; // 删除键
    delete value; // 删除值
    delete expiration_time; // 删除剩余时间

    _file_reader.close(); // 关闭文件
    FILE_IO_MUTEX.unlock(); // 解锁

    return;
}

/*
 * 验证字符串的合法性
 * @param str 字符串
 * @return bool
 * @remark 验证字符串的合法性
 */
template <typename K, typename V>
bool SkipListWithCache<K, V>::is_valid_string(const std::string& str) { 
    // 如果字符串str非空，并且字符串中包含分隔符delimiter，那么返回true
    // find()函数返回字符串中第一个匹配的位置，如果没有找到匹配的位置，则返回std::string::npos
    if (!str.empty()&& str.find(delimiter) != std::string::npos) { 
        return true;
    }

    return false;
}

/*
 * 从字符串中获取键值对
 * @param line 字符串
 * @param key 键
 * @param value 值
 * @param expiration_time 剩余时间
 * @return void
 * @remark 从字符串中获取键值对
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value, std::string* expiration_time) { 
    
    if (!is_valid_string(str)) { 
        return;
    }
    
    size_t pos1 = str.find(delimiter); // 查找第一个分隔符的位置
    size_t pos2 = str.find(delimiter, pos1 + 1); // 查找第二个分隔符的位置


    *key = str.substr(0, pos1); // 获取键
    *value = str.substr(pos1 + 1, pos2 - pos1 - 1); // 获取值
    *expiration_time = str.substr(pos2 + 1); // 获取剩余时间

    return;
}

/*
 * 打印跳表
 * @return void
 * @remark 打印跳表
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::display_skiplist() {
    
    std::cout << "\n*****Skip List*****"<<"\n"; 
    for (int i = 0; i <= _skip_list_level; i++) {
        NodeWithTTL<K, V> *node = this->_header->forward[i]; 
        std::cout << "Level " << i << ": ";
        while (node != NULL) {
            std::cout << node->getKey() << ":" << node->getValue() << ";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

/*
 * 打印缓存
 * @return void
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::display_cache() {
    cache.display();
}

/*
 * 周期性数据持久化策略
 * @param kv_store kv存储
 * @return void
 * @remark 周期性数据持久化策略
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::periodic_save(int interval_seconds) {
    // 启动后台线程
    std::thread([this, interval_seconds]() {
        while (keep_running) { 
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds)); // 每隔interval_seconds秒执行一次
            dump_file(); // 数据持久化
        }
    }).detach();
};

/*
 * 停止周期性数据持久化策略
 * @return void
 * @remark 停止周期性数据持久化策略
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::stop_periodic_save() {
    keep_running = false; // 停止后台线程
};

/*
 * 周期性删除过期数据
 * @param kv_store kv存储
 * @return void
 * @remark 周期性删除过期数据
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::periodic_cleanup(int interval_seconds) {
    running_cleanup = true; // 运行清理
    std::thread([this, interval_seconds]() {
        while (running_cleanup) { 
            remove_skiplist_expired(); // 删除过期数据
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds)); // 每隔interval_seconds秒执行一次
        }
    }).detach();
};

/*
 * 停止周期性删除过期数据
 * @return void
 * @remark 停止周期性删除过期数据
 */
template <typename K, typename V>
void SkipListWithCache<K, V>::stop_periodic_cleanup() {
    running_cleanup = false; // 停止清理
};
