> 该项目基于Skiplist-CPP项目进行开发，Skiplist-CPP项目地址：[https://github.com/youngyangyang04/Skiplist-CPP]
> 在其基础上，添加了LRU缓存功能，支持惰性删除和周期性存盘策略等功能，对其Node类和SkipList类有较大修改，新增了LRUCache类。使用Skiplist-CPP项目的测试用例，对其进行了测试。
> 引用项目标明出处即可。


# 基于跳表的键值型数据库

## 1 功能描述

### 1.1 功能列表

1. **功能一：** 支持键值对的增加、删除、修改、查询、设置过期时间、打印跳表操作。
2. **功能二：** 实现了 `LRU` 算法对设置了过期时间的键值对进行管理。
3. **功能三：** 支持存储 `int`, `string` 等基本数据类型，也可以通过运算符重载支持自定义数据类型。
4. **功能四：** 支持对过期键值对进行定向删除和惰性删除。
5. **功能五：** 支持数据持久化和周期性数据存盘策略。

### 1.2 提供接口

* insert_element(插入数据)
* delete_element(删除数据)
* search_element(查找数据)
* display_skiplist(打印跳表)
* dump_file(数据持久化)
* load_file(加载数据)
* periodic_save(周期性持久化)
* stop_periodic_save(停止周期性持久化)
* periodic_cleanup(定期清理过期数据)
* stop_periodic_cleanup(停止定期清理过期数据)

### 1.3 项目内文件

* skiplish.h Skiplist-CPP项目中的跳表实现
* skiplist_cache.h 基于Skiplist-CPP项目的跳表实现，添加了LRU缓存功能、惰性删除、主动删除、周期性存盘策略等功能
* LRU.h LRU缓存实现

* /test/1.跳表的定义.cpp 测试 `skiplist.h` 中跳表的 `Node` 类
* /test/2.跳表的插入.cpp 测试 `skiplist.h` 中跳表的 `insert_element` 操作
* /test/3.跳表的删除.cpp 测试 `skiplist.h` 中跳表的 `delete_element` 和 `search_element` 操作
* /test/4.跳表的打印.cpp 测试 `skiplist.h` 中跳表的 `display_skiplist` 操作
* /test/5.跳表的持久化.cpp 测试 `skiplist.h` 中跳表的 `dump_file` 操作
* /test/6.跳表的加载.cpp 测试 `skiplist.h` 中跳表的 `load_file` 操作
* /test/7.stress_test.cpp `Skiplist-CPP`中的 `stress_test.cpp`
* /test/8.LRU缓存.cpp 测试 `LRU.h` 中的 `LRUCache` 类
* /test/9.LRU中惰性删除的实现.cpp 测试 `LRU.h` 中的 `LRUCache` 类的惰性删除功能
* /test/10.插入带过期时间的元素.cpp 测试 `skiplist_cache.h` 中的 `SkipListWithCache` 类的插入带过期时间的元素功能
* /test/11.生成和读取持久化文件2.cpp 测试 `skiplist_cache.h` 中的 `SkipListWithCache` 类的 `dump_file` 和 `load_file` 功能
* /test/12.数据周期性持久化策略.cpp 测试 `skiplist_cache.h` 中的 `SkipListWithCache` 类的周期性持久化策略
* /test/13.过期数据周期性删除策略.cpp 测试 `skiplist_cache.h` 中的 `SkipListWithCache` 类的过期数据周期性删除策略

* /store/dumpFile `skiplist.h` 中跳表的 `dump_file` 操作生成的持久化文件
* /store/dumpFile_cache `skiplist_cache.h` 中跳表的 `dump_file` 操作加载的持久化文件

* readme.md 项目详细说明文档

## 2 具体实现细节

### 2.1 LRU.h (LRU 缓存实现)

*实现一个支持过期时间功能的 `LRU` 键值对缓存。`LRU` 缓存是一种常用的数据结构，用于在缓存满时，自动淘汰最久未使用的数据。*

#### 2.1.1 **`LRUCache`类**

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

###### `CacheNode` 结构体

* `CacheNode` 是缓存中的每个节点，它保存了键 `key`，值 `value` 和过期时间 `expire_time`。

###### `_capacity`

* `_capacity` 是缓存的最大容量，即最多可以存储多少个键值对。

###### `cache_map`

* `cache_map` 是一个哈希表，用于快速查找键值对在缓存中的位置。

###### `cache_list`

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

###### `LRUCache()`

* 将缓存容量设置为 3。

```cpp
template <typename K, typename V>
LRUCache<K, V>::LRUCache() { 
    this->_capacity = 3; // 默认容量为3
}
```

###### `LRUCache(size_t)`

* 将缓存容量设置为 `capacity`。

```cpp
template <typename K, typename V>
LRUCache<K, V>::LRUCache(size_t capacity) { 
    this->_capacity = capacity;
}
```

###### `get(const K&, V&)`

1. `get()` 用于根据键 `key` 获取对应的值 `value`，并且将该数据标记为“最近使用”。首先在哈希表中查找 `key`。如果没有找到，则返回 `false`。
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

###### `put(const K&, const V&, int)`

1. `put()`用于将键值对插入缓存中，并且设置该键值对的过期时间。
2. 计算过期时间：当前时间加上 `ttl_seconds`（以秒为单位）。
3. 如果 `key` 已经存在于缓存中，更新其值和过期时间，并将其移动到链表头部。
4. 如果缓存已满（`cache_list.size() >= _capacity`），移除最久未使用的元素（链表尾部的节点）。
5. 将新数据插入到链表头部，并更新哈希表中的映射。

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

###### `remove(const K&)`

1. `remove()` 方法用于从缓存中移除指定键的元素。
2. 在哈希表中查找 `key`，如果找到，则从链表和哈希表中删除。

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

###### `display()`

* `display()` 方法用于打印缓存中的所有数据，从最近使用到最久未使用的顺序显示。

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

###### `remove_expired()`

* `remove_expired()` 方法用于批量删除已经过期的数据。遍历缓存的链表尾部，删除所有过期的数据。

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

###### `is_expired()`

1. `is_expired()` 方法用于判断数据是否过期。如果当前时间大于等于数据的过期时间，则返回 `true`，否则返回 `false`。
2. 通过比较当前时间与节点的 `expire_time` 来实现。如果当前时间大于 `expire_time`，则数据已经过期，返回 `true`，否则返回 `false`。

```cpp
template <typename K, typename V>
bool LRUCache<K, V>::is_expired(const TimePoint& expire_time) const{
    return std::chrono::steady_clock::now() > expire_time;
}
```

### 2.2 skiplist_cache.h (基于跳表实现的键值对数据库引擎)

*实现了一个结合跳表（SKip List）和LRU缓存（Least Recently Used Cache）的数据结构。该系统支持过期时间控制、数据持久化以及周期性删除。*

#### 2.2.1 **`NodeWithTTL`类(节点类)**

跳表的每个节点保存了键值对，还支持**过期时间（TTL）**控制

```cpp
template <typename K, typename V>
class NodeWithTTL {
public:
    using TimePoint = std::chrono::steady_clock::time_point;  // 时间点类型
    NodeWithTTL(K k, V v, int level, TimePoint t);  // 构造函数
    ~NodeWithTTL();  // 析构函数

    K getKey() const;  // 获取键
    V getValue() const;  // 获取值
    void setExpireTime(TimePoint t);  // 设置过期时间
    int getRemainingTime() const;  // 获取剩余TTL时间

    NodeWithTTL<K, V>** forward;  // 前向指针数组，指向跳表中的下一个节点
    int node_level;  // 节点层级

private:
    K key;  // 键
    V value;  // 值
    TimePoint expiration_time;  // 过期时间
};
```

##### 2.2.1.1 成员变量

```cpp
template <typename K, typename V>
class NodeWithTTL {
public:
    using TimePoint = std::chrono::steady_clock::time_point;  // 时间点类型
    NodeWithTTL<K, V>** forward;  // 前向指针数组，指向跳表中的下一个节点
    int node_level;  // 节点层级

private:
    K key; // 键
    V value;  // 值
    TimePoint expiration_time;  // 过期时间
}
```

###### `expiration_time`

* `expiration_time`：用于记录数据的过期时间，通过 `TimePoint` 类型表示。

###### `forward`

* `forward`：前向指针数组，表示该节点在跳表的不同层级上的前向指针。跳表中的节点可以在多个层级上存在，以实现快速查找。

###### `node_level`

* `node_level`: 节点层级，用于表示节点在跳表中的层级。

###### `key`

* `key`：键，用于保存节点的键。

###### `value`

* `value`：值，用于保存节点的值。

##### 2.2.1.2 成员函数

```cpp
template <typename K, typename V>
class NodeWithTTL {
public:
    NodeWithTTL() {} // 默认构造函数
    NodeWithTTL(K k, V v, int, TimePoint t); // 构造函数
    ~NodeWithTTL(); // 析构函数

    K getKey() const; // 获取键
    V getValue() const; // 获取值

    void setValue(V); // 设置值
    void setExpireTime(TimePoint t); // 设置过期时间
    TimePoint getExpireTime() const; // 获取过期时间
    int getRemainingTime() const; // 获取剩余时间
}
```

###### `NodeWithTTL()`

`NodeWithTTL()`：默认构造函数，用于创建一个空的节点。

###### `NodeWithTTL(K, V, int, TimePoint)`

* `NodeWithTTL(K k, V v, int level, TimePoint expiration_time)`：构造函数，用于创建一个带有键值对、层级和过期时间的节点。

```cpp
template <typename K, typename V>
NodeWithTTL<K, V>::NodeWithTTL(K key, V value, int level, TimePoint expiration_time) { 
    this->key = key;
    this->value = value;
    this->node_level = level;
    this->expiration_time = expiration_time;
    this->forward = new NodeWithTTL<K, V>*[level + 1];
    memset(forward, 0, sizeof(NodeWithTTL<K, V>*) * (level + 1)); // 初始化前向指针数组
};
```

###### `~NodeWithTTL()`

`~NodeWithTTL()`：析构函数，用于释放前向指针数组。

```cpp
template <typename K, typename V>
NodeWithTTL<K, V>::~NodeWithTTL() {
    delete[] forward;
};
```

###### `getExpireTime()`

`getExpireTime()`：获取节点的过期时间。

```cpp
template <typename K, typename V>
TimePoint NodeWithTTL<K, V>::getExpireTime() const {
    return expiration_time;
}
```

###### `setExpireTime()`

`setExpireTime(TimePoint t)`：设置节点的过期时间。

```cpp
template <typename K, typename V>
void NodeWithTTL<K, V>::setExpireTime(TimePoint expiration_time) {
    this->expiration_time = expiration_time;
};

```

###### `getRemainingTime()`

`getRemainingTime()`：获取节点剩余的TTL时间。

```cpp
template <typename K, typename V>
int NodeWithTTL<K, V>::getRemainingTime() const {
    return std::chrono::duration_cast<std::chrono::seconds>(expiration_time - std::chrono::steady_clock::now()).count();
}
```

###### `getKey()`

`getKey()`：获取节点的键。

```cpp
template <typename K, typename V>
K NodeWithTTL<K, V>::getKey() const {
    return key;
};
```

###### `getValue()`

`getValue()`：获取节点的值。

```cpp
template <typename K, typename V>
V NodeWithTTL<K, V>::getValue() const {
    return value;
};
```

###### `setValue(V)`

`setValue(V value)`：设置节点的值。

```cpp
template <typename K, typename V>
void NodeWithTTL<K, V>::setValue(V value) {
    this->value = value;
};
```

#### 2.2.2 **`SkipListWithCache`类(跳表类)**

*实现了一个带有缓存和数据持久化的跳表。其不仅支持跳表的基本操作，还增加了LRU缓存、惰性删除、过期数据删除以及数据持久化功能。*

##### **成员变量**

```cpp
template <typename K, typename V>
class SkipListWithCache {
private:
    int max_level;  // 跳表的最大层级
    int _skip_list_level;  // 当前跳表的层级
    NodeWithTTL<K, V>* _header;  // 跳表的头节点

    // file operation
    std::ofstream _file_writer; // 文件写入
    std::ifstream _file_reader; // 文件读取

    int _element_count;  // 跳表中元素的数量

    LRUCache<K, V> _cache;  // LRU缓存
}
```

###### `max_level`

* `max_level`：跳表的最大层级。

###### `_skip_list_level`

* `_skip_list_level`：当前跳表的层级。

###### `_header`

* `_header`：跳表的头节点。

###### `_file_writer`

* `_file_writer`：文件写入。用于将跳表数据写入文件。

###### `_file_reader`

* `_file_reader`：文件读取。用于从文件中读取跳表数据。

###### `_element_count`

* `_element_count`：跳表中元素的数量。

###### `_cache`

* `_cache`：LRU缓存。用于缓存跳表中的数据。

##### **成员函数**

```cpp
template <typename K, typename V>
class SkipListWithCache {
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
}
```

###### `SkipListWithCache(int, size_t)`

`SkipListWithCache(int max_level, size_t cache_size)`：构造函数，初始化跳表的最大层级和LRU缓存的大小。

```cpp
template <typename K, typename V>
SkipListWithCache<K, V>::SkipListWithCache(int max_level, size_t cache_capacity) 
    : _max_level(max_level), _skip_list_level(0), _element_count(0), cache(cache_capacity) {
    // 创建头节点
    K k{};
    V v{};
    this->_header = create_node(k, v, max_level, PERMANENT_TTL); // 创建头节点
};
```

* 初始化跳表的最大层级，并创建头节点。
* 初始化 `LRU` 缓存的大小，并创建 `LRU` 缓存。

###### `~SkipListWithCache()`

`~SkipListWithCache()`：析构函数，释放跳表的内存。

```cpp
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
```

* 关闭文件读取和写入。
* 停止周期性删除过期数据和周期性数据持久化策略。
* 递归删除跳表节点。
  * `clear` 函数递归删除跳表节点。

    ```cpp
    template <typename K, typename V>
    void SkipListWithCache<K, V>::clear(NodeWithTTL<K, V>* current) {
        if (current->forward[0] != nullptr) {
            clear(current->forward[0]);
        }
        delete current;
    };

    ```

* delete释放跳表的内存。

###### `insert_element(const K&, const V&, int)`

*双重存储：`insert_element`确保在跳表中插入一个新元素，并将其添加到 `LRU` 缓存中；*

*线程安全：`insert_element`是线程安全的；*

*时间复杂度：插入的平均时间复杂度为$O(\log n)$*

1. 我们首先定义当前节点和更新路径数组

    ```cpp
    NodeWithTTL<K, V>* current = this->_header; // 当前节点
    NodeWithTTL<K, V>* update[_max_level + 1]; // 更新路径数组
    memset(update, 0, sizeof(NodeWithTTL<K, V>*) * (_max_level + 1)); // 初始化更新数组
    ```

    * `current`：从跳表的头节点 `_header` 开始遍历。
    * `update`：数组用于记录从头节点到目标节点的路径。在插入新节点时，需要更新这条路径中的前向指针。
    * 使用 `memset` 将 `update` 数组中的所有指针初始化为`nullptr`。

2. 遍历跳表，寻找插入位置

    ```cpp
    for (int i = _skip_list_level; i >= 0; i--) { 
        while (current->forward[i] != nullptr && current->forward[i]->getKey() < key) {
            current = current->forward[i]; // 向前遍历当前层
        }
        update[i] = current; // 记录路径
    }
    current = current->forward[0]; // 移动到最低层的下一个节点
    ```

    * 从最高层开始遍历跳表，直到最低层，寻找目标插入位置。
    * 如果当前层的前向指针指向的节点的键小于 `key`，继续向前移动。
    * 更新路径：在每一层，记录搜索路径中的节点到 `update` 数组。
    * 最终，`current` 移动到最低层的第一个节点。

3. 判断是否存在重复键

    ```cpp
    if (current != nullptr && current->getKey() == key) {
        mtx.unlock(); // 解锁
        return 1; // 已存在，插入失败
    }
    ```

    * 如果当前节点非空且键与目标 `key` 相同，说明键已经存在于跳表中。
    * 解锁并返回 `1`，表示插入失败。

4. 如果键不存在，创建新节点

    ```cpp
    int random_level = get_random_level(); // 获取随机层级

    if (random_level > _skip_list_level) { // 如果随机层级大于当前层级
        for (int i = _skip_list_level + 1; i < random_level + 1; i++) {
            update[i] = _header; // 将新层级路径指向头节点
        }
        _skip_list_level = random_level; // 更新当前跳表层级
    }

    ```

    * 随机层级：跳表中每个节点随机出现在若干层中。这是跳表高效查找的核心。`get_random_level()` 返回节点的层级。
    * get_random_level()：返回一个随机层级，用于新节点。  

        ```cpp
        template <typename K, typename V>
        int SkipListWithCache<K, V>::get_random_level() {
            int k = 1;
            while (rand() % 2) {
                k++;
            }
            k = (k < _max_level) ? k : _max_level;
            return k;
        };
        ```

    * 更新层级：如果新节点的层级高于当前跳表的最高层级，需要更新跳表层级，并将路径中对应层级指向头节点。

5. 插入新节点

    ```cpp
        NodeWithTTL<K, V>* inserted_node = create_node(key, value, random_level, ttl_seconds); // 创建新节点

        for (int i = 0; i <= random_level; i++) {
            inserted_node->forward[i] = update[i]->forward[i]; // 新节点的前向指针指向更新路径的下一个节点
            update[i]->forward[i] = inserted_node; // 更新路径的前向指针指向新节点
        }

        _element_count++; // 增加元素计数
    ```

    * 创建新节点：使用 `create_node` 方法创建新节点，并指定其层级和过期时间。
    * `create_node`：创建一个新节点，返回指向新节点的指针。

        ```cpp
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
        ```

    * 更新前向指针：在每一层，将新节点的前向指针指向更新路径中的下一个节点。同时，将更新路径中的节点指向新节点。
    * 增加元素计数：更新跳表中的元素数量。

6. 插入缓存

    ```cpp
    cache.put(key, value, ttl_seconds); // 将数据插入缓存
    ```

    * 缓存同步：将插入的数据同步存入 `LRU` 缓存。

###### `search_element(const K&)`

*多层查找结构：利用跳表的多层索引，提高查找速度。*

*缓存与跳表结合：缓存存储最近访问的数据，避免频繁访问跳表，提高性能。*

*过期控制：支持节点的过期删除，保证数据的时效性。*

1. 打印调试信息，初始化当前节点

    ```cpp
    std::cout << "search_element-----------------" << std::endl;
    NodeWithTTL<K, V>* current = this->_header; // 当前节点
    V value; // 存储查询到的值
    ```

    * 打印调试信息，表明已经开始执行查找操作。
    * `current`：初始化为跳表的头节点 `_header`，从头节点开始遍历跳表。
    * `value`：用于临时存储缓存中的值。

2. 优先从缓存中查找

    ```cpp
    if (cache.get(key, value)) { 
        std::cout << "Found key: " << key << ", value: " << value << " from cache" << std::endl;
        return true; // 缓存中存在
    }
    ```

    * 缓存优先查找：
    * 调用缓存的 `get` 方法查询是否存在 `key`。
    * 如果存在，打印调试信息并返回 `true`，表示查找成功。
    * 缓存的优势：通过缓存加速频繁访问的数据查询，避免访问跳表，提高性能。

3. 在跳表中查找目标

   1) 在各层级遍历查找目标位置

        ```cpp
        for (int i = _skip_list_level; i >= 0; i--) { 
            while (current->forward[i] != nullptr && current->forward[i]->getKey() < key) {
                current = current->forward[i]; // 在当前层向前移动
            }
        }
        current = current->forward[0]; // 转到最低层的下一个节点

        ```

        * 从当前跳表的最高层 `_skip_list_level` 开始查找。
        * 在每一层中，不断沿着前向指针 `forward` 向前移动，直到找到比 `key` 大或等于 `key` 的节点。
        * **降层**：如果在当前层未找到目标，继续在下一层查找。
        * **最终位置**：在最低层中，`current` 指向目标位置的前一个节点，或者直接指向目标节点。

   2) 检查节点是否存在

       ```cpp
       if (current != nullptr && current->getKey() == key) {
       // 如果节点过期，删除节点
       }
       
       ```

       * **检查节点是否存在**：如果 `current` 非空且其键等于 `key`，说明找到了目标节点。

4. 检查节点是否过期

    ```cpp
    if (is_expired(current->getExpireTime())) {
        std::cout << "Found key: " << key << ", value: " << current->getValue() 
                    << " from skip list, but expired" << std::endl;
        delete_element(key); // 删除过期节点
        return false; // 返回false，表示数据已过期
    }
    ```

    * 如果节点存在，但其过期时间已到（即 `is_expired` 返回 `true`），打印过期信息，并执行以下操作：
    * 删除节点：调用 `delete_element` 方法从跳表中删除该节点。
    * 返回 `false`：表明查找失败，因为数据已过期。

5. 返回查询结果

    ```cpp
    std::cout << "Found key: " << key << ", value: " << current->getValue() 
                << " from skip list" << std::endl;
    return true; // 返回true，表示查找成功
    ```

    * 如果节点存在且未过期，打印调试信息并返回 `true`，表示查找成功。

6. 如果未找到节点

    ```cpp
    std::cout << "Not found key: " << key << std::endl;
    return false; // 返回false，表示查找失败

    ```

    * 如果遍历跳表后未找到目标节点，打印“未找到”信息，并返回`false`。

###### `delete_element(const K&)`

**线程安全**：`delete_element()` 是线程安全的；

**时间复杂度**：删除操作的平均时间复杂度为$O(\log n)$，其中 $n$ 是跳表的节点数量

**动态层级调整**：在删除节点后，自动调整跳表的层级，保持跳表的高效性

**缓存同步更新**：删除跳表中的数据时，同步删除缓存中的数据，保证数据的一致性

1. **查找初始化**：初始化当前节点与更新路径

    ```cpp
    NodeWithTTL<K, V>* current = this->_header; // 当前节点
    NodeWithTTL<K, V>* update[_max_level + 1]; // 更新路径数组
    memset(update, 0, sizeof(NodeWithTTL<K, V>*) * (_max_level + 1)); // 初始化更新路径
    ```

    * `current`：指向跳表的头节点，从头节点开始遍历跳表。
    * `update`：数组用于记录从头节点到目标节点的路径。在删除节点时，需要更新这条路径上的前向指针。
    * 使用 `memset` 将 `update` 数组中的所有指针初始化为 `nullptr`。

2. **查找并更新路径**：遍历跳表，寻找目标节点的位置

    ```cpp
    for (int i = _skip_list_level; i >= 0; i--) { 
        while (current->forward[i] != nullptr && current->forward[i]->getKey() < key) {
            current = current->forward[i]; // 向前移动
        }
            update[i] = current; // 记录路径中的节点
        }
        current = current->forward[0]; // 移动到最低层的下一个节点
    ```

    * 从**跳表的最高层**开始逐层查找目标节点。
    * 在每一层中，不断沿着 `forward` 指针向前移动，直到找到比 `key` 大或等于 `key` 的节点。
    * 记录路径：将每一层中的最后一个未越过目标节点的节点存入 `update` 数组。
    * 最后，`current` 指向最低层中的下一个节点。

3. **删除节点**：检查节点是否存在，并进行删除

    ```cpp
    if (current != nullptr && current->getKey() == key)
    ```

    * 如果 `current` 节点非空且键等于 `key`，说明找到了目标节点。

    1) **更新前向指针，删除节点**

        ```cpp
        for (int i = 0; i <= _skip_list_level; i++) { 
            if (update[i]->forward[i] != current) {
                break; // 如果当前层的前向指针不指向目标节点，则停止
            }
            update[i]->forward[i] = current->forward[i]; // 跳过目标节点
        }
        ```

       * 逐层更新前向指针：
           * 在每一层中，将路径数组中的节点的前向指针直接指向目标节点的下一个节点（跳过目标节点）。
           * 如果某一层的前向指针不指向目标节点，则停止更新该层及其以上的层。

    2) **检查并更新跳表的层级**

        ```cpp
        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == nullptr) {
                _skip_list_level--; // 如果最高层为空，减少跳表的层级
        }
        ```

       * 如果最高层级为空（即该层没有节点），则减少跳表的层级。这是跳表的动态层级调整机制。

    3) **打印成功信息并减少元素计数**

        ```cpp
        std::cout << "Successfully deleted key: " << key << std::endl;
        _element_count--; // 元素个数减1
        
        ```

       * 打印成功信息，并减少跳表的元素计数。

4. **同步删除缓存数据**：删除缓存中的数据

   ```cpp
    cache.remove(key); // 删除缓存中的数据
   ```

   * **解锁**：操作完成后释放锁，允许其他线程访问。
   * **同步删除缓存中的数据**：调用缓存的 `remove` 方法，将对应的键值从缓存中删除。

###### `dump_file()`

`dump_file()`实现了一个跳表的持久化功能，即将跳表中的数据保存到文件中。

**过期控制**：使用时间戳命名的文件来保存当前跳表中未过期的节点

**线程安全的**：`dump_file()`是线程安全的

1. 获取当前时间并格式化为 "yyyyMMddHHmmss" 格式

    ```cpp
    auto now = std::chrono::system_clock::now(); // 获取当前系统时间
    std::time_t now_c = std::chrono::system_clock::to_time_t(now); // 转换为 time_t 类型
    std::tm* now_tm = std::localtime(&now_c); // 转换为 tm 结构体

    char time_str[20]; // 用于保存格式化后的时间字符串
    std::strftime(time_str, sizeof(time_str), "%Y%m%d%H%M%S", now_tm); // 格式化时间
    ```

    * 这段代码获取当前系统时间，并将其格式化为 `"yyyyMMddHHmmss"` 格式的字符串。
    * `strftime` 用于将时间格式化为易于阅读的形式，例如：`20241012153045`，表示 `2024年10月12日15:30:45`。

2. 创建包含时间戳的文件名

    ```cpp
    std::string filename = "store/dumpFile_cache_" + std::string(time_str);
    ```

   * 创建文件名，并包含时间戳，以确保每次调用该函数时都生成唯一的文件名。
   * 文件将存储在 `"store"` 目录下，并命名为类似 `dumpFile_cache_20241012153045` 的格式。

3. 打印调试信息并加锁

    ```cpp
    std::cout << "dump_file-----------------" << std::endl;
    FILE_IO_MUTEX.lock(); // 加锁
    ```

   * 打印调试信息，表明已进入持久化操作。
   * **加锁**：使用 `mutex` 锁住文件 `I/O` 操作，确保在多线程环境下不会出现文件访问冲突。

4. 打开文件并检查是否打开成功

    ```cpp
    _file_writer.open(filename); // 打开文件

    if (!_file_writer.is_open()) { 
        std::cerr << "Failed to open file: " << filename << std::endl;
        FILE_IO_MUTEX.unlock(); // 解锁
        return;
    }
    ```

   * 打开文件以进行写入操作。如果文件打开失败，打印错误信息并解锁。

5. 遍历跳表，将未过期数据写入文件

    ```cpp
    NodeWithTTL<K, V>* node = this->_header->forward[0]; // 从头节点的第一个元素开始遍历

    while (node != nullptr) { 
        if (!is_expired(node->getExpireTime())) {
            _file_writer << node->getKey() << ":" << node->getValue() << ":" << node->getRemainingTime() << "\n";
            std::cout << node->getKey() << ":" << node->getValue() << ":" << node->getRemainingTime() << ";\n";
        }
        node = node->forward[0]; // 移动到下一个节点
    }
    ```

   * **遍历跳表**：从头节点的第一个前向节点开始，逐个遍历跳表中的所有节点。
   * **检查过期状态**：使用 `is_expired` 方法判断节点是否过期。如果未过期，将数据写入文件中。
     * `is_expired` 方法用于检查节点是否过期，如果过期时间早于当前时间，则返回 `true`，否则返回 `false`。

    ```cpp
    template <typename K, typename V>
    bool SkipListWithCache<K, V>::is_expired(const typename NodeWithTTL<K, V>::TimePoint& expiration_time) const {
        return expiration_time < std::chrono::steady_clock::now();
    };
    ```

   * **写入格式**：每行的数据格式为 `key:value:remaining_time`。
   * 同时打印该节点的键值信息，便于调试。

6. 刷新文件并关闭

    ```cpp
    _file_writer.flush(); // 刷新文件，确保数据写入磁盘
    _file_writer.close(); // 关闭文件
    ```

   * **刷新文件**：调用 `flush` 方法，将缓冲区中的数据立即写入磁盘。
   * **关闭文件**：写入完成后关闭文件，释放资源。

7. 解锁并返回

    ```cpp
    FILE_IO_MUTEX.unlock(); // 解锁
    return;
    ```

   * **解锁**：释放互斥锁，允许其他线程进行文件操作。
   * **返回**：结束函数。

###### `load_file()`

* **功能**：`load_file()` 实现了从文件中加载数据到跳表的功能。确保程序重启或系统崩溃时，可以从持久化文件恢复跳表中的数据，并将这些数据重新插入到跳表中。

* **线程安全**：`load_file()` 是线程安全的，使用了 `mutex` 对文件 `I/O` 操作进行了加锁。

1. 加锁，打印调试信息，并打开文件。

    ```cpp
    FILE_IO_MUTEX.lock(); // 加锁
    std::cout << "Loading data from file..." << std::endl;
    _file_reader.open(DEFAULT_STORE_FILE); // 打开文件
    ```

   * **加锁**：使用 `mutex` 锁定文件操作，避免多个线程同时读取文件造成竞争条件。
   * **调试信息**：打印“加载数据”信息，表明数据加载过程已经开始。
   * **打开文件**：尝试打开持久化文件 `DEFAULT_STORE_FILE`。

2. 检查文件是否成功打开

    ```cpp
    if (!_file_reader.is_open()) { 
        std::cerr << "Failed to open file: " << DEFAULT_STORE_FILE << std::endl;
        FILE_IO_MUTEX.unlock(); // 解锁
        return;
    }
    ```

   * 如果文件打开失败，则打印错误信息，并立即解锁和返回，终止加载过程。

3. 初始化临时变量

    ```cpp
    std::string line; // 存储文件中的一行数据
    K* key = new K(); // 动态分配键
    V* value = new V(); // 动态分配值
    std::string* expiration_time = new std::string(); // 动态分配过期时间
    ```

   * `line`：用于存储从文件中读取的每一行数据。
   * `key`、`value`、`expiration_time`：分别用于保存从文件中解析出的键、值和剩余时间。

4. 从文件中逐行读取数据

    ```cpp
    while (getline(_file_reader, line)) { 
        get_key_value_from_string(line, key, value, expiration_time); // 从字符串中提取键值对
        if (key->empty() || value->empty() || expiration_time->empty()) {
            continue; // 跳过无效行
        }

        insert_element(*key, *value, stoi(*expiration_time)); // 插入元素到跳表
        std::cout << "key: " << *key << ", " << "value: " << *value 
                  << ", " << "expiration_time: " << *expiration_time << std::endl;
    } 
    ```

   * **逐行读取数据**：使用 `getline` 从文件中读取每一行内容。
   * **解析键值对**：调用 `get_key_value_from_string` 函数，将读取的字符串解析为键、值和过期时间。
     * `get_key_value_from_string` 函数用于从字符串中提取键、值和过期时间。

        ```cpp
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
        ```

        * `is_valid_string` 函数用于检查字符串是否有效，如果字符串为空或长度小于等于1，则返回 `false`，否则返回 `true`。  

            ```cpp
            template <typename K, typename V>
            bool SkipListWithCache<K, V>::is_valid_string(const std::string& str) { 
                // 如果字符串str非空，并且字符串中包含分隔符delimiter，那么返回true
                // find()函数返回字符串中第一个匹配的位置，如果没有找到匹配的位置，则返回std::string::npos
                if (!str.empty()&& str.find(delimiter) != std::string::npos) { 
                    return true;
                }

                return false;
            }
            ```

   * **跳过无效数据**：如果解析后的键、值或过期时间为空，则跳过该行。
   * **插入数据**：将解析出的键值对和过期时间插入到跳表中。
   * **打印调试信息**：打印当前行的键、值和剩余时间，便于跟踪加载过程。

5. 释放动态分配的内存

    ```cpp
    delete key; // 删除键
    delete value; // 删除值
    delete expiration_time; // 删除剩余时间
    ```

   * 使用 `new` 分配的内存需要手动释放，避免内存泄漏。

6. 关闭文件并解锁

    ```cpp
    _file_reader.close(); // 关闭文件
    FILE_IO_MUTEX.unlock(); // 解锁
    ```

   * **关闭文件**：释放文件资源。
   * **解锁**：释放互斥锁，允许其他线程访问文件。

###### `display_skiplist()`

**打印跳表**: `display_skiplist` 函数用于打印跳表的所有节点信息，包括键、值和过期时间。

1. 遍历跳表的每一层

    ```cpp
    for (int i = 0; i <= _skip_list_level; i++) {
        NodeWithTTL<K, V> *node = this->_header->forward[i]; 
        std::cout << "Level " << i << ": ";
    ```

    * `for` 循环：遍历跳表的每一层，从第 `0` 层到当前跳表的最高层 `_skip_list_level`。
    * `this->_header->forward[i]`：在每一层，从头节点的第 `i` 层前向指针开始遍历。
    * 打印层级信息，如 `"Level 0: "`，表示当前正在展示第 `0` 层的节点信息。

2. 遍历每一层中的节点

    ```cpp
    while (node != NULL) {
        std::cout << node->getKey() << ":" << node->getValue() << ";";
        node = node->forward[i];
    }
    ```

   * `while` 循环：沿着每一层的前向指针遍历该层的所有节点。
   * 打印节点信息：
     * `node->getKey()`：获取当前节点的键。
     * `node->getValue()`：获取当前节点的值。
     * 打印格式为：`key:value`;，多个节点之间用 ; 分隔。
   * 移动到下一个节点：`node = node->forward[i]`，在同一层向前移动到下一个节点。

###### `display_cache()`

打印缓存中的所有键值对

* 调用 `cache.display()` 函数，打印缓存中的所有键值对。

```cpp
template <typename K, typename V>
void SkipListWithCache<K, V>::display_cache() {
    cache.display();
}
```

###### 周期性数据持久化策略

`periodic_save()`

*启动一个后台线程，每隔指定的时间间隔（interval_seconds 秒）执行一次数据持久化操作。*

**持久化方法**：调用 dump_file() 将跳表的数据保存到文件中。*

```cpp
template <typename K, typename V>
void SkipListWithCache<K, V>::periodic_save(int interval_seconds) {
    // 启动后台线程
    std::thread([this, interval_seconds]() {
        while (keep_running) { 
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds)); // 每隔interval_seconds秒执行一次
            dump_file(); // 数据持久化
        }
    }).detach();
}
```

1. `std::thread`：创建一个新的线程来执行周期性任务；
2. `sleep_for`：线程进入休眠状态，等待指定的时间间隔；
3. `detach`：线程与主线程分离，后台独立运行。这意味着主线程不会等待该线程完成；
4. `keep_running`：使用 `std::atomic<bool>` 变量控制线程的运行状态。当设置为 `false` 时，停止循环。

`stop_periodic_save()`

**停止周期性持久化策略**: `stop_periodic_save` 函数用于停止周期性持久化策略。

```cpp
template <typename K, typename V>
void SkipListWithCache<K, V>::stop_periodic_save() {
    keep_running = false; // 停止后台线程
}
```

1. 设置控制标志 `keep_running` 为 `false`，使持久化线程中的 `while` 循环条件不再成立，从而停止持久化线程的执行。

###### 定期删除策略

`periodic_cleanup()`

*启动一个后台线程，每隔指定的时间间隔（interval_seconds 秒）执行一次数据清理操作。*

```cpp
template <typename K, typename V>
void SkipListWithCache<K, V>::periodic_cleanup(int interval_seconds) {
    running_cleanup = true; // 运行清理
    std::thread([this, interval_seconds]() {
        while (running_cleanup) { 
            remove_skiplist_expired(); // 删除过期数据
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds)); // 每隔interval_seconds秒执行一次
        }
    }).detach();
}
```

1. `running_cleanup`：使用 `std::atomic<bool>` 变量控制线程的运行状态；
2. `detach`：与主线程分离，允许后台线程独立运行；

3. `remove_skiplist_expired()`：执行过期数据的删除操作，确保跳表中没有无效数据。
   1) `remove_skiplist_expired()`：删除跳表中的过期数据。

   ```cpp
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
   ```

`stop_periodic_cleanup()`

**停止周期性删除过期数据**：`stop_periodic_cleanup` 函数用于停止周期性删除过期数据的策略。

```cpp
template <typename K, typename V>
void SkipListWithCache<K, V>::stop_periodic_cleanup() {
    running_cleanup = false; // 停止清理
}
```

* 设置控制标志 `running_cleanup` 为 `false`，使清理线程中的 `while` 循环条件不再成立，从而停止该线程的执行。
