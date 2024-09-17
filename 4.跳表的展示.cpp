#include "skiplist.h"
#include <vector>

/**
 * Test case for skip list
13
1 1
2 2
3 3
4 4
5 5
6 6
7 7
8 8
9 9
10 10
11 11
12 12
13 13
*/

using namespace std;

int main() {
    SkipList<int, int> *skipList = new SkipList<int, int>(16);

    int N = 0;
    cin >> N;

    vector<pair<int, int>> elements(N);

    for (int i = 0; i < N; i++) { 
        int key, value;
        cin >> key >> value;
        elements[i] = make_pair(key, value);
    }

    for (int i = 0; i < N; i++) { 
        int key = elements[i].first;
        int value = elements[i].second;

        if (skipList->insert_element(key, value) == 0) { 
            cout << "Insert Success" << endl;
        } else { 
            cout << "Insert Failed" << endl;
        }
    }

    skipList->display_list(); // display the skip list
}

/*
*****Skip List*****
Level 7: _header ----------------------------------------------------------> 8:8
Level 6: _header ----------------------------------------------------------> 8:8
Level 5: _header ----------------------------------------------------------> 8:8
Level 4: _header ----------------------------------------------------------> 8:8
Level 3: _header --> 1:1 --------------------------------------------------> 8:8
Level 2: _header --> 1:1 ----------> 3:3 ----------------------------------> 8:8 --> 9:9 --> 10:10 ------------> 12:12
Level 1: _header --> 1:1 --> 2:2 --> 3:3 --> 4:4 --> 5:5 --> 6:6 --> 7:7 --> 8:8 --> 9:9 --> 10:10 --> 11:11 --> 12:12 --> 13:13
Level 0: _header --> 1:1 --> 2:2 --> 3:3 --> 4:4 --> 5:5 --> 6:6 --> 7:7 --> 8:8 --> 9:9 --> 10:10 --> 11:11 --> 12:12 --> 13:13
*/