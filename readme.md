## 基于跳表的键值型数据库

### 功能描述 

#### 功能列表

1. **功能一：** 支持键值对的增加、删除、修改、查询、设置过期时间、打印跳表操作。
2. **功能二：** 实现了 `LRU` 算法对设置了过期时间的键值对进行管理。
3. **功能三：** 支持存储 `int`, `string` 等基本数据类型，也可以通过运算符重载支持自定义数据类型。
4. **功能四：** 支持对过期键值对进行定向删除和惰性删除。
5. **功能五：** 支持数据持久化和周期性数据存盘策略。

#### 具体功能描述

### 具体实现细节

#### LRU.h

*实现一个支持过期时间功能的 `LRU` 键值对缓存。`LRU` 缓存是一种常用的数据结构，用于在缓存满时，自动淘汰最久未使用的数据。*

**`LRUCache` 类**
```cpp
template <typename K, typename V>
class LRUCache { 
public:
    using TimePoint = std::chrono::steady_clock::time_point; 
    LRUCache();
    LRUCache(size_t capacity);
    bool get(const K& k, V& v);
    void put(const K& k, const V& v, int ttl_seconds);
    bool remove(const K& k);
    void display(); 
    void remove_expired();

private:
    struct CacheNode {
        K key;
        V value;
        TimePoint expire_time; // 过期时间
    };

    size_t _capacity;
    std::unordered_map<K, typename std::list<CacheNode>::iterator> cache_map;
    std::list<CacheNode> cache_list;
    bool is_expired(const TimePoint& expire_time) const;
};
```

##### 成员变量

```cpp
public: 
    using TimePoint = std::chrono::steady_clock::time_point; 
private:
    struct CacheNode {
        K key;
        V value;
        TimePoint expire_time; // 过期时间
    }

    size_t _capacity;
    std::unordered_map<K, typename std::list<CacheNode>::iterator> cache_map;
    std::list<CacheNode> cache_list;
```

* `TimePoint` 是一个时间点类型的别名，用来记录数据的过期时间。我们通过 `using TimePoint = std::chrono::steady_clock::time_piont;` 或者 `typedef TimePoint std::chrono::steady_clock::time_piont;` 定义了 `TimePoint` 类型，用于表示键值对的过期时间。

* `CacheNode` 是缓存中的每个节点，它保存了键 `key`，值 `value` 和过期时间 `expire_time`。

* `_capacity` 是缓存的最大容量，即最多可以存储多少个键值对。

* `cache_map` 是一个哈希表，用于快速查找键值对在缓存中的位置。

* `cache_list` 是一个双向链表，用于维护键值对的访问顺序。链表中的节点按照访问顺序从前往后排列，越靠近链表尾部的节点表示越久未使用。

`LRU` 算法的核心数据结构是哈希表和双向链表，可以使用哈希表**快速查找元素**的特性，通过键来查找对应的值。通过哈希表，我们可以在 `O(1)` 的时间复杂度内查找到某个键是否存在于缓存中，并获取到该键对应的链表节点的迭代器 `std::list<CacheNode>::iterator`，通过这个迭代器，我们可以直接访问链表中的节点，并进行高效的移动操作（例如将节点移动到链表的头部）。这样可以**结合双向链表的效率**，可以在 `O(1)` 的时间内将最近使用的元素移到链表头部，并在缓存满时高效删除最久未使用的元素。

*哈希表解决了快速查找的问题，因为它可以根据键快速定位元素的位置。双向链表解决了维护访问顺序的问题，因为它允许快速的节点移动（更新最近使用的元素顺序）和删除（淘汰最久未使用的元素）。*

##### 成员函数

```cpp
public:
    LRUCache();                           // 默认构造函数
    LRUCache(size_t capacity);            // 含参构造函数
    bool get(const K& k, V& v);
    void put(const K& k, const V& v, int ttl_seconds);
    bool remove(const K& k);
    void display(); 
    void remove_expired();
private:
    bool is_expired(const TimePoint& expire_time) const;
```

**默认构造函数**：将缓存容量设置为 3。
```cpp
template <typename K, typename V>
LRUCache<K, V>::LRUCache() { 
    this->_capacity = 3; // 默认容量为3
}
```

**含参构造函数**：允许通过参数指定缓存的容量。 
```cpp
template <typename K, typename V>
LRUCache<K, V>::LRUCache(size_t capacity) { 
    this->_capacity = capacity;
}
```

**get方法**：用于根据键 `key` 获取对应的值 `value`，并且将该数据标记为“最近使用”。

1. `get()` 方法首先在哈希表中查找 `key`。如果没有找到，则返回 `false`。
2. 如果找到，并且数据未过期，则将该节点移动到链表头部，标识它是最近使用的，并返回 true。
3. 如果找到的数据已经过期，采用惰性删除策略，将该数据从缓存中移除，并返回 false。

```cpp
template <typename K, typename V>
bool LRUCache<K, V>::get(const K& key, V& value) {
    
    auto it = cache_map.find(key);
    if (it == cache_map.end()) { 
        return false; 
    }

    if (is_expired((*it->second).expire_time)) { 
        cache_list.pop_back(); // 删除链表尾部节点
        cache_map.erase(it); // 删除哈希表中对应的项
        return false; // 如果键不存在，返回false
    }

    cache_list.splice(cache_list.begin(), cache_list, it->second);
    value = it->second->value; // 获取值

    return true;
}
```

**put方法**：用于将键值对插入缓存中，并且设置该键值对的过期时间。

1. 计算过期时间：当前时间加上 `ttl_seconds`（以秒为单位）。
2. 如果 `key` 已经存在于缓存中，更新其值和过期时间，并将其移动到链表头部。
3. 如果缓存已满（`cache_list.size() >= _capacity`），移除最久未使用的元素（链表尾部的节点）。
4. 将新数据插入到链表头部，并更新哈希表中的映射。

```cpp
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
```

**remove方法**：用于从缓存中移除指定键的元素

1. `remove()` 方法用于从缓存中移除指定键的元素。
2. 在哈希表中查找`key`，如果找到，则从链表和哈希表中删除。

```cpp
template <typename K, typename V>
bool LRUCache<K, V>::remove(const K& key) { 
    auto it = cache_map.find(key); // 在哈希表中查找键

    // 如果键不存在，返回false
    if (it == cache_map.end()) { 
        return false;
    }

    cache_list.erase(it->second); // 删除链表中的节点
    cache_map.erase(it); // 删除哈希表中对应的项

    return true;
}
```

**display方法**：用于显示缓存中的所有键值对。

1. `display()` 方法用于打印缓存中的所有数据，从最近使用到最久未使用的顺序显示。

```cpp
template <typename K, typename V>
void LRUCache<K, V>::display() { 
    for (const auto& item : cache_list) {
        std::cout << item.key <<  ": " << item.value << std::endl;
    }
    std::cout << std::endl;

    return;
}
```

**remove_expired方法**：用于删除所有过期的数据。

1. `remove_expired()` 方法用于批量删除已经过期的数据。遍历缓存的链表尾部，删除所有过期的数据。

```cpp
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
```

**is_expired方法**：用于判断数据是否过期。

1. `is_expired()` 方法用于判断数据是否过期。如果当前时间大于等于数据的过期时间，则返回 `true`，否则返回 `false`。
2. 通过比较当前时间与节点的 `expire_time` 来实现。如果当前时间大于 `expire_time`，则数据已经过期，返回 `true`，否则返回 `false`。
```cpp
template <typename K, typename V>
bool LRUCache<K, V>::is_expired(const TimePoint& expire_time) const{
    return std::chrono::steady_clock::now() > expire_time;
}
```

