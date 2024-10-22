#include "skiplist.h"
#include <vector>

using namespace std;

int main() { 
    int N = 0; // N: number of elements
    int M = 0; // M: number of queries

    // create a new skip list with 16 levels
    SkipList<int, int> *skipList = new SkipList<int, int>(16); 

    cin >> N >> M;

    // store the elements to be inserted into the skip list
    vector<pair<int, int>> elements(N);

    // store the keys to be searched in the skip list
    vector<int> keys(M); 

    for (int i = 0; i < N; i++) { 
        int key, value;
        cin >> key >> value;
        elements[i] = make_pair(key, value);
    }

    for (int i = 0; i < M; i++) { 
        int key;
        cin >> key;
        keys[i] = key;
    }

    // insert N elements into the skip list
    for (int i = 0; i < N; i++) {
        int key = elements[i].first;
        int value = elements[i].second;

        if (skipList->insert_element(key, value) == 0) { 
            cout << "Insert Success" << endl;
        } else { 
            cout << "Insert Failed" << endl;
        }
    }

    // search M elements in the skip list
    for (int i = 0; i < M; i++) { 
        int key = keys[i];
        if (skipList->search_element(key)) { 
            cout << "Search Success" << endl;
        } else { 
            cout << "Search Failed" << endl;
        }
    }

    return 0;
}