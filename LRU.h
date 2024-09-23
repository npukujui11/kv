#include <iostream>
#include <unordered_map> 
#include <list>
#include <chrono>

template <typename K, typename V>
class LRUCache { 
public:
    using TimePoint = std::chrono::steady_clock::time_point; // 时间点(using 在这里等价于 typedef)

    LRUCache(size_t capacity) : _capacity(capacity) {}

    bool get(const K& k, V& v); // 获取数据

    void put(const K& k, const V& v, int ttl_seconds); // 存储数据

    void display(); // 打印缓存中的数据

    void remove_expired(); // 移除过期数据

private:
    // 缓存节点
    struct CacheNode {
        K key;
        V value;
        TimePoint expire_time; // 过期时间
    };
    

    // 缓存容量
    size_t _capacity;

    // key -> list iterator
    // 哈希表（cache_map）：键是缓存数据的键，值是指向双向链表节点的迭代器/指针。
    std::unordered_map<K, typename std::list<CacheNode>::iterator> cache_map;
    
    // list iterator -> key
    // 存储缓存的键值对，最近使用的元素在链表的头部，最久未使用的元素在尾部。
    std::list<CacheNode> cache_list;

    // 判断是否过期
    bool is_expired(const TimePoint& expire_time) const;
};


/**
 * 获取数据，更新访问顺序
 * @param key 键
 * @param value 值
 * @return bool
 */
template <typename K, typename V>
bool LRUCache<K, V>::get(const K& key, V& value) {
    
    auto it = cache_map.find(key);  // 在哈希表中查找键
    
    // 如果键不存在或者已经过期，返回false
    if (it == cache_map.end()) { 
        return false; // 如果键不存在，返回false
    }

    /*
     * 惰性删除策略
     */
    if (is_expired((*it->second).expire_time)) { 
        cache_list.pop_back(); // 删除链表尾部节点
        cache_map.erase(it); // 删除哈希表中对应的项
        return false; // 如果键不存在，返回false
    }

    // 将数据移到链表头部，标识最近使用
    cache_list.splice(cache_list.begin(), cache_list, it->second);
    value = it->second->value; // 获取值

    return true;
}

/**
 * 插入数据，更新访问顺序，支持过期时间
 * @param key 键
 * @param value 值
 * @param ttl_seconds 过期时间
 * @return void 
 */
template <typename K, typename V>    
void LRUCache<K, V>::put(const K& key, const V& value, int ttl_seconds) { 
    
    auto it = cache_map.find(key);

    TimePoint expire_time = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_seconds); // 计算过期时间

    // 如果该键已经存在于缓存中，更新值并将其移动到链表头部
    if (it != cache_map.end()) { 
        // 更新已有节点
        it->second->value = value; // 更新值
        it->second->expire_time = expire_time; // 更新过期时间
        cache_list.splice(cache_list.begin(), cache_list, it->second); // 移动到头部
    } else { 
        // 如果缓存容量已满，移除最久未使用的元素
        if (cache_list.size() >= _capacity) { 
            // 删除最久未使用的数据
            K expired_key = cache_list.back().key; // 获取最久未使用的元素的键
            cache_list.pop_back(); // 删除链表尾部节点
            cache_map.erase(expired_key); // 删除哈希表中对应的项
        }

        // 插入新节点到链表头部
        cache_list.emplace_front(CacheNode{key, value, expire_time}); 
        cache_map[key] = cache_list.begin(); // 更新哈希表中对应的项
    }

    return;
}

/**
 * 打印缓存中的数据
 * @param void
 * @return void 
 */
template <typename K, typename V>
void LRUCache<K, V>::display() { 
    for (const auto& item : cache_list) {
        std::cout << item.key <<  ": " << item.value << std::endl;
    }
    std::cout << std::endl;

    return;
}


/**
 * 移除过期数据
 * @param void
 * @return void
 * @note 主要用于定期清理过期数据
 */
template <typename K, typename V>
void LRUCache<K, V>::remove_expired() { 
    
    auto now = std::chrono::steady_clock::now(); // 获取当前时间

    while (!cache_list.empty() && is_expired(cache_list.back().expire_time)) {
        K expired_key = cache_list.back().key; // 获取最久未使用的元素的键
        cache_list.pop_back(); // 删除链表尾部节点
        cache_map.erase(expired_key); // 删除哈希表中对应的项
    }

    return;
}

/**
 * 判断是否过期
 * @param expire_time 过期时间
 * @return bool
 */
template <typename K, typename V>
bool LRUCache<K, V>::is_expired(const TimePoint& expire_time) const{
    return std::chrono::steady_clock::now() > expire_time;
}