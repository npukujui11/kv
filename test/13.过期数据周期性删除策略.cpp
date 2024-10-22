#include <iostream>
#include <tuple>
#include <vector>
#include <chrono>
#include <thread>
#include "skiplist_cache.h"

/*
 * Test case 1:
21 
1 absurd 5
2 appropriate 5
3 barren 5
4 commentary 5
5 consistent 5
6 conspicuous 10
7 confidential 10
8 decent 10
9 delicate 10
10 energetic 10
11 exclusive 15
12 feeble 15
13 greasy 15
14 inadequate 15
15 intent 15
16 intricate 20
17 notorious 20
18 obscure 20
19 pathetic 20
20 reluctant 20
21 solemn 30
 */

using namespace std;

int main() { 
    int N = 0; // N: number of elements

    SkipListWithCache<string, string> *skipList = new SkipListWithCache<string, string>(16, 3);

    cin >> N;

    vector<tuple<string, string, int>> elements(N); // elements to be inserted

    for (int i = 0; i < N; i++) {
        string key, value;
        int expiration_time;
        cin >> key >> value >> expiration_time;
        elements[i] = make_tuple(key, value, expiration_time);
    }

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

    // 开始周期性删除过期数据，每隔5秒删除一次
    skipList->periodic_cleanup(5);

    // 模拟主线程的其他操作（此处让主线程运行25秒，验证后台删除是否正常运行）
    std::this_thread::sleep_for(std::chrono::seconds(25));

    // 显示跳表中的所有元素
    skipList->display_skiplist();

    // 停止周期性删除过期数据
    skipList->stop_periodic_cleanup();

    // 模拟主线程的其他操作（此处让主线程运行15秒，验证后台删除是否已经停止）
    std::this_thread::sleep_for(std::chrono::seconds(15));

    // 显示跳表中的所有元素
    skipList->display_skiplist();

    return 0;
}

/*
 * Output:
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
delete_element-----------------
Successfully deleted key: 1
delete_element-----------------
Successfully deleted key: 2
delete_element-----------------
Successfully deleted key: 3
delete_element-----------------
Successfully deleted key: 4
delete_element-----------------
Successfully deleted key: 5
delete_element-----------------
Successfully deleted key: 10
delete_element-----------------
Successfully deleted key: 6
delete_element-----------------
Successfully deleted key: 7
delete_element-----------------
Successfully deleted key: 8
delete_element-----------------
Successfully deleted key: 9
delete_element-----------------
Successfully deleted key: 11
delete_element-----------------
Successfully deleted key: 12
delete_element-----------------
Successfully deleted key: 13
delete_element-----------------
Successfully deleted key: 14
delete_element-----------------
Successfully deleted key: 15
delete_element-----------------
Successfully deleted key: 16
delete_element-----------------
Successfully deleted key: 17
delete_element-----------------
Successfully deleted key: 18
delete_element-----------------
Successfully deleted key: 19
delete_element-----------------
Successfully deleted key: 20

*****Skip List*****
Level 0: 21:solemn;
Level 1: 21:solemn;
Level 2: 21:solemn;
Level 3: 21:solemn;

*****Skip List*****
Level 0: 21:solemn;
Level 1: 21:solemn;
Level 2: 21:solemn;
Level 3: 21:solemn;
 */