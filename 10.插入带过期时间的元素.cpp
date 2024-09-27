#include <iostream>
#include <tuple>
#include <vector>
#include <chrono>
#include <thread>
#include "skiplist_cache.h"

/*
 * Test case for skip list with cache
8
3
1 absurd 10
2 appropriate 5
3 barren 15
4 commentary 20
5 consistent 30
6 conspicuous 10
7 confidential 15
8 decent 5
8
4
3

 */

using namespace std;
using TimePoint = std::chrono::steady_clock::time_point;

int main() { 
    int N = 0; // N: number of elements
    int M = 0; // M: number of queries

    // max_level = 16, Cache capacity = 3 
    SkipListWithCache<int, string> *skipList = new SkipListWithCache<int, string>(16, 3);

    cin >> N >> M;

    vector<tuple<int, string, int>> elements(N); // 用于插入的元素

    vector<int> keys(M); // 用于查找的键

    for (int i = 0; i < N; i++) { 
        int key, expire_time;
        string value;
        cin >> key >> value >> expire_time;
        elements[i] = make_tuple(key, value, expire_time);
    }

    for (int i = 0; i < M; i++) { 
        int key;
        cin >> key;
        keys[i] = key;
    }

    // insert elements
    for (int i = 0; i < N; i++) { 
        int key = get<0>(elements[i]);
        string value = get<1>(elements[i]);
        int expire_time = get<2>(elements[i]);

        if (skipList->insert_element(key, value, expire_time) == 0) { 
            cout << "Insert Success\n" << endl;
        } else { 
            cout << "Insert Failed\n" << endl;
        }
    }

    // display the skip list
    skipList->display_skiplist();
    // display the cache
    skipList->display_cache();

    this_thread::sleep_for(chrono::seconds(15)); // sleep for 15 seconds

    // display the skip list
    skipList->display_skiplist();
    // display the cache
    skipList->display_cache();

    // search elements
    for (int i = 0; i < M; i++) { 
        int key = keys[i];
        if (skipList->search_element(key)) { 
            cout << "Search Success\n" << endl;
        } else { 
            cout << "Search Failed\n" << endl;
        }
    }

    skipList->display_skiplist();
    skipList->display_cache();
    return 0;
}