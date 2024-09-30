#include <iostream>
#include <tuple>
#include <vector>
#include <chrono>
#include <thread>
#include "skiplist_cache.h"

/*
 * Test case 1:
8
1 absurd 20
2 appropriate 15
3 barren 30
4 commentary 15
5 consistent 19
6 conspicuous 11
7 confidential 25
8 decent 30
 */

using namespace std;

int main() {    
    int N = 0; // N: number of elements
    
    // max_level = 16, Cache capacity = 3
    SkipListWithCache<string, string> *skipList = new SkipListWithCache<string, string>(16, 3);

    cin >> N;

    vector<tuple<string, string, int>> elements(N); // elements to be inserted

    for (int i = 0; i < N; i++) {
        string key, value;
        int expiration_time;
        cin >> key >> value >> expiration_time;
        elements[i] = make_tuple(key, value, expiration_time);
    }

    // 插入元素
    for (int i = 0; i < N; i++) { 
        string key = get<0>(elements[i]);
        string value = get<1>(elements[i]);
        int expiration_time = get<2>(elements[i]);

        if (skipList->insert_element(key, value, expiration_time) == 0) {
            cout << "Insert Success" << endl;
        } else { 
            cout << "Insert Failed" << endl;
        }
    }

    // 开始周期性数据持久化，每隔10秒持久化一次
    skipList->periodic_save(10);

    // 模拟主线程的其他操作（此处让主线程运行30秒，验证后台持久化是否正常运行）
    std::this_thread::sleep_for(std::chrono::seconds(30));
    /*
     * 会存盘 3 次，产生三个文件
     */

    // 停止周期性数据持久化
    skipList->stop_periodic_save();

    // 验证停止后台
    std::this_thread::sleep_for(std::chrono::seconds(30));
    /*
     * 不会再存盘
     */

    return 0;
}