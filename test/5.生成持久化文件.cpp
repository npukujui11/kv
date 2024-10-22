#include "skiplist.h"
#include <vector>

/**
 * Test case 1 for skip list
21 
1 absurd
2 appropriate
3 barren
4 commentary
5 consistent
6 conspicuous
7 confidential
8 decent
9 delicate
10 energetic
11 exclusive
12 feeble
13 greasy
14 inadequate
15 intent
16 intricate
17 notorious
18 obscure
19 pathetic
20 reluctant
21 solemn
*/

/**
 * Test case 2 for skip list1
10
1 absurd
2 appropriate
3 barren
4 commentary
5 consistent
6 conspicuous
7 confidential
8 decent
9 delicate
10 energetic
*/

using namespace std;

int main() {
    SkipList<string, string> *skipList = new SkipList<string, string>(16);

    int N = 0;
    cin >> N;

    vector<pair<string, string>> elements(N);

    for (int i = 0; i < N; i++) { 
        string key;
        string value;
        cin >> key >> value;
        elements[i] = make_pair(key, value);
    }

    for (int i = 0; i < N; i++) { 
        string key = elements[i].first;
        string value = elements[i].second;

        if (skipList->insert_element(key, value) == 0) { 
            cout << "Insert Success" << endl;
        } else {
            cout << "Insert Failed" << endl;
        }
    }

    skipList->display_list_prettily(); // display the skip list

/* Test case 1 display_list_prettily() will output:
*****Skip List*****
Level 7: _header -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------> 8:decent
Level 6: _header -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------> 8:decent
Level 5: _header -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------> 8:decent
Level 4: _header --------------------------------------------------------------------------------------------------------------------------------------------------------------> 19:pathetic ---------------------------------------------------------------------------------------------------------------------------------------> 8:decent
Level 3: _header --> 1:absurd ------------------------------------------------------------------------------------------------> 16:intricate ----------------------------------> 19:pathetic --------------------> 20:reluctant --> 21:solemn --------------------------------------------------------------------------------------> 8:decent
Level 2: _header --> 1:absurd --> 10:energetic -------------------> 12:feeble ----------------> 14:inadequate ----------------> 16:intricate --> 17:notorious --> 18:obscure --> 19:pathetic --------------------> 20:reluctant --> 21:solemn --> 3:barren -------------------------------------------------------------------------> 8:decent --> 9:delicate
Level 1: _header --> 1:absurd --> 10:energetic --> 11:exclusive --> 12:feeble --> 13:greasy --> 14:inadequate --> 15:intent --> 16:intricate --> 17:notorious --> 18:obscure --> 19:pathetic --> 2:appropriate --> 20:reluctant --> 21:solemn --> 3:barren --> 4:commentary --> 5:consistent --> 6:conspicuous --> 7:confidential --> 8:decent --> 9:delicate
Level 0: _header --> 1:absurd --> 10:energetic --> 11:exclusive --> 12:feeble --> 13:greasy --> 14:inadequate --> 15:intent --> 16:intricate --> 17:notorious --> 18:obscure --> 19:pathetic --> 2:appropriate --> 20:reluctant --> 21:solemn --> 3:barren --> 4:commentary --> 5:consistent --> 6:conspicuous --> 7:confidential --> 8:decent --> 9:delicate
 */


/* Test case 2 display_list_prettily() will output:
*****Skip List*****
Level 7: _header --------------------------------------------------------------------------------------------------------------------------------------> 8:decent
Level 6: _header --------------------------------------------------------------------------------------------------------------------------------------> 8:decent
Level 5: _header --------------------------------------------------------------------------------------------------------------------------------------> 8:decent
Level 4: _header --------------------------------------------------------------------------------------------------------------------------------------> 8:decent
Level 3: _header --> 1:absurd -------------------------------------------------------------------------------------------------------------------------> 8:decent
Level 2: _header --> 1:absurd --> 10:energetic --------------------> 3:barren -------------------------------------------------------------------------> 8:decent --> 9:delicate
Level 1: _header --> 1:absurd --> 10:energetic --> 2:appropriate --> 3:barren --> 4:commentary --> 5:consistent --> 6:conspicuous --> 7:confidential --> 8:decent --> 9:delicate
Level 0: _header --> 1:absurd --> 10:energetic --> 2:appropriate --> 3:barren --> 4:commentary --> 5:consistent --> 6:conspicuous --> 7:confidential --> 8:decent --> 9:delicate
*/

    // Save the skip list to a file
    skipList->dump_file(); // save the skip list to a file

    return 0;
}