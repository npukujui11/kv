#include "LRU.h"
#include <iostream>
#include <string>
#include <thread>

using namespace std;

int main() { 
    LRUCache<int, string> cache(3);  // 创建容量为3的LRU缓存

    // insert 3 elements
    cache.put(1, "one", 2);               // 2 seconds   
    cache.put(2, "two", 5);               // 5 seconds
    cache.put(3, "three", 1);             // 1 second

    this_thread::sleep_for(chrono::seconds(3)); // sleep for 3 second

    string value;

    // get element 1, but it is expired
    if (cache.get(1, value) == false) { 
        cout << "Key 1 expired or not found" << endl;
    }

    // get element 2, it is not expired
    if (cache.get(2, value) == true) { 
        cout << "Key 2 value: " << value << endl;
    }

    // get element 3, it is not expired
    if (cache.get(3, value) == false) { 
        cout << "Key 3 expired or not found " << endl;
    }

    return 0;
}