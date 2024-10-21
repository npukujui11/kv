# 基于跳表的键值型数据库

## 1 功能描述 

### 1.1 功能列表

1. **功能一：** 支持键值对的增加、删除、修改、查询、设置过期时间、打印跳表操作。
2. **功能二：** 实现了 `LRU` 算法对设置了过期时间的键值对进行管理。
3. **功能三：** 支持存储 `int`, `string` 等基本数据类型，也可以通过运算符重载支持自定义数据类型。
4. **功能四：** 支持对过期键值对进行定向删除和惰性删除。
5. **功能五：** 支持数据持久化和周期性数据存盘策略。

### 1.2 具体功能描述

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

*跳表的每个节点保存了键值对，还支持过期时间（TTL）控制*

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

##### 成员变量

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

* `expiration_time`：用于记录数据的过期时间，通过`TimePoint`类型表示。

###### `forward`

* `forward`：前向指针数组，表示该节点在跳表的不同层级上的前向指针。跳表中的节点可以在多个层级上存在，以实现快速查找。

###### `node_level`

* `node_level`: 节点层级，用于表示节点在跳表中的层级。

###### `key`

* `key`：键，用于保存节点的键。

###### `value`

* `value`：值，用于保存节点的值。

##### 成员函数

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

*实现了一个带有缓存和数据持久化的跳表。其不仅支持跳表的基本操作，还增加了LRU缓存的、过期数据删除以及数据持久化功能。*

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
* 初始化`LRU`缓存的大小，并创建`LRU`缓存。

######  `~SkipListWithCache()`

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
* delete释放跳表的内存。

###### `insert_element(const K&, const V&, int)`

*双重存储：`insert_element`确保在跳表中插入一个新元素，并将其添加到`LRU`缓存中；*

*线程安全：`insert_element`是线程安全的；*

*时间复杂度：插入的平均时间复杂度为$O(\log n)$*

1. 我们首先定义当前节点和更新路径数组
```cpp
    NodeWithTTL<K, V>* current = this->_header; // 当前节点
    NodeWithTTL<K, V>* update[_max_level + 1]; // 更新路径数组
    memset(update, 0, sizeof(NodeWithTTL<K, V>*) * (_max_level + 1)); // 初始化更新数组
```
* `current`：从跳表的头节点`_header`开始遍历。
* `update`：数组用于记录从头节点到目标节点的路径。在插入新节点时，需要更新这条路径中的前向指针。
* 使用`memset`将`update`数组中的所有指针初始化为`nullptr`。

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
    * 如果当前层的前向指针指向的节点的键小于`key`，继续向前移动。
    * 更新路径：在每一层，记录搜索路径中的节点到`update`数组。
* 最终，`current`移动到最低层的第一个节点。

3. 判断是否存在重复键
```cpp
    if (current != nullptr && current->getKey() == key) {
        mtx.unlock(); // 解锁
        return 1; // 已存在，插入失败
    }
```
* 如果当前节点非空且键与目标`key`相同，说明键已经存在于跳表中。
* 解锁并返回`1`，表示插入失败。

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
* 随机层级：跳表中每个节点随机出现在若干层中。这是跳表高效查找的核心。`get_random_level()`返回节点的层级。
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
* 创建新节点：使用`create_node`方法创建新节点，并指定其层级和过期时间。
* 更新前向指针：在每一层，将新节点的前向指针指向更新路径中的下一个节点。同时，将更新路径中的节点指向新节点。
* 增加元素计数：更新跳表中的元素数量。

6. 插入缓存
```cpp
    cache.put(key, value, ttl_seconds); // 将数据插入缓存
```

* 缓存同步：将插入的数据同步存入`LRU`缓存。

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
* `current`：初始化为跳表的头节点`_header`，从头节点开始遍历跳表。
* `value`：用于临时存储缓存中的值。

2. 优先从缓存中查找
```cpp
    if (cache.get(key, value)) { 
        std::cout << "Found key: " << key << ", value: " << value << " from cache" << std::endl;
        return true; // 缓存中存在
    }
```
* 缓存优先查找：
    * 调用缓存的`get`方法查询是否存在`key`。
    * 如果存在，打印调试信息并返回`true`，表示查找成功。
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
    * **检查节点是否存在**：如果 current 非空且其键等于 key，说明找到了目标节点。

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

```cpp
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

```

###### `dump_file()`

```cpp
template <typename K, typename V>
void SkipListWithCache<K, V>::dump_file() {
    // 获取当前时间并格式化 "yyyyMMddHHmmss" 格式
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now); // 转换为time_t
    std::tm* now_tm = std::localtime(&now_c); // 转换为tm

    char time_str[20]; // 用于保存格式化后的时间字符串
    std::strftime(time_str, sizeof(time_str), "%Y%m%d%H%M%S", now_tm); // 格式化时间字符串

    // 创建包含时间戳的文件名
    std::string filename = "store/dumpFile_cache_" + std::string(time_str);
    
    std::cout << "dump_file-----------------" << std::endl;
    FILE_IO_MUTEX.lock(); // 加锁
    _file_writer.open(filename); // 打开文件

    // 如果文件打开失败
    if (!_file_writer.is_open()) { 
        std::cerr << "Failed to open file: " << filename << std::endl;
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
```

###### `load_file()`

```cpp
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
```

###### `display_skiplist()`

```cpp
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
```

###### `display_cache()`

```cpp
template <typename K, typename V>
void SkipListWithCache<K, V>::display_cache() {
    cache.display();
}
```