#include <iostream>
#include <tuple>
#include <vector>
#include <chrono>
#include <thread>
#include "skiplist_cache.h"

/*
 * Test case 1:
8
1 absurd 3600
2 appropriate 3600
3 barren -1
4 commentary 3600
5 consistent 3600
6 conspicuous 3600
7 confidential 3600
8 decent 3600
 */

/*
 * Test result 1:
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success
Insert Success

*****Skip List*****
Level 0: 1:absurd;2:appropriate;3:barren;4:commentary;5:consistent;6:conspicuous;7:confidential;8:decent;
Level 1: 1:absurd;2:appropriate;3:barren;4:commentary;5:consistent;6:conspicuous;7:confidential;8:decent;
Level 2: 1:absurd;3:barren;8:decent;
Level 3: 1:absurd;8:decent;
Level 4: 8:decent;
Level 5: 8:decent;
Level 6: 8:decent;
Level 7: 8:decent;
Level 8: 8:decent;
dump_file-----------------
1:absurd:3584;
2:appropriate:3584;
3:barren:633405451;
4:commentary:3584;
5:consistent:3584;
6:conspicuous:3584;
7:confidential:3584;
8:decent:3584;
Loading data from file...
key: 1, value: absurd, expiration_time: 3584
key: 2, value: appropriate, expiration_time: 3584
key: 3, value: barren, expiration_time: 633405451
key: 4, value: commentary, expiration_time: 3584
key: 5, value: consistent, expiration_time: 3584
key: 6, value: conspicuous, expiration_time: 3584
key: 7, value: confidential, expiration_time: 3584
key: 8, value: decent, expiration_time: 3584

*****Skip List*****
Level 0: 1:absurd;2:appropriate;3:barren;4:commentary;5:consistent;6:conspicuous;7:confidential;8:decent;
Level 1: 1:absurd;2:appropriate;3:barren;4:commentary;5:consistent;6:conspicuous;7:confidential;8:decent;
Level 2: 1:absurd;2:appropriate;4:commentary;6:conspicuous;8:decent;
Level 3: 8:decent;
 */



using namespace std;

int main() { 
    int N = 0; // N: number of elements

    // max_level = 16, Cache capacity = 3
    SkipListWithCache<string, string> *skipList = new SkipListWithCache<string, string>(16, 3);

    SkipListWithCache<string, string> *skipList2 = new SkipListWithCache<string, string>(16, 3);

    cin >> N;
    
    vector<tuple<string, string, int>> elements(N); // elements to be inserted

    for (int i = 0; i < N; i++) { 
        string key;
        string value;
        int ttl;
        cin >> key >> value >> ttl;
        elements[i] = make_tuple(key, value, ttl);
    }

    for (int i = 0; i < N; i++) { 
        string key = get<0>(elements[i]);
        string value = get<1>(elements[i]);
        int ttl = get<2>(elements[i]);

        if (skipList->insert_element(key, value, ttl) == 0) { 
            cout << "Insert Success" << endl;
        } else { 
            cout << "Insert Failed" << endl;
        }
    }

    skipList->display_skiplist(); // display the skip list

    this_thread::sleep_for(chrono::seconds(15)); // sleep for 15 seconds

    skipList->dump_file();

    skipList2->load_file();

    skipList2->display_skiplist(); // display the skip list

    return 0;
}

