#include "skiplist.h"
#include <vector>

using namespace std;

int main() { 
    int N = 0; // N: number of elements to be inserted
    int K = 0; // K: number of keys to be deleted
    int M = 0; // M: number of keys to be searched 

    // create a new skip list with 16 levels
    SkipList<int, int> *skipList = new SkipList<int, int>(16);

    cin >> N >> K >> M;

    // store the elements to be inserted into the skip list
    vector<pair<int, int>> elements(N);

    // store the keys to be deleted from the skip list
    vector<int> keys_to_delete(K);

    // store the keys to be searched in the skip list
    vector<int> keys_to_search(M);

    for (int i = 0; i < N; i++) { 
        int key, value;
        cin >> key >> value;
        elements[i] = make_pair(key, value);
    }

    for (int i = 0; i < K; i++) { 
        int key;
        cin >> key;
        keys_to_delete[i] = key;
    }

    for (int i = 0; i < M; i++) { 
        int key;
        cin >> key;
        keys_to_search[i] = key;
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

    // delete K elements from the skip list
    for (int i = 0; i < K; i++) { 
        int key = keys_to_delete[i];
        skipList->delete_element(key);
    }

    // search M elements in the skip list
    for (int i = 0; i < M; i++) { 
        int key = keys_to_search[i];
        if (skipList->search_element(key)) { 
            cout << "Search Success" << endl;
        } else { 
            cout << "Search Failed" << endl;
        }
    }

    return 0;
}