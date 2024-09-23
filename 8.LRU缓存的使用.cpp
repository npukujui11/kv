#include "LRU.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// 定义ttl
#define TTL 3600

int main() { 
    LRUCache<int, string> cache(3);  // 创建容量为3的LRU缓存

    cache.put(1, "one", TTL);
    cache.put(2, "two", TTL);
    cache.put(3, "three", TTL);
    
    cache.display(); // output: 3:three 2:two 1:one

    string value;
    cache.get(2, value); // 访问 key 2，将其移动到头部
    cache.display(); // output: 2:two 3:three 1:one

    cache.put(4, "four", TTL); // 添加新的 key 4

    cache.display(); // output: 4:four 2:two 3:three
    return 0;
}