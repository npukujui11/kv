#include "skiplist.h"

using namespace std;

int main() { 
    SkipList<string, string> *skipList = new SkipList<string, string>(16);

    skipList->load_file(); // load the skip list from the file

    skipList->display_list_prettily(); // display the skip list

/*
*****Skip List*****
Level 7: _header -------------------------------------------------------------------------------------------------------------> 16:intricate
Level 6: _header -------------------------------------------------------------------------------------------------------------> 16:intricate
Level 5: _header -------------------------------------------------------------------------------------------------------------> 16:intricate
Level 4: _header -------------------------------------------------------------------------------------------------------------> 16:intricate --------------------------------------------------------------------------------------------------------------------------------------------------------------------> 7:confidential
Level 3: _header --> 1:absurd ------------------------------------------------------------------------------------------------> 16:intricate ----------------------------------------------------------------------------------------------------------------> 4:commentary -------------------------------------> 7:confidential --> 8:decent --> 9:delicate
Level 2: _header --> 1:absurd -------------------> 11:exclusive --------------------------------------------------------------> 16:intricate --> 17:notorious --> 18:obscure ------------------> 2:appropriate -------------------> 21:solemn ---------------> 4:commentary --> 5:consistent --> 6:conspicuous --> 7:confidential --> 8:decent --> 9:delicate
Level 1: _header --> 1:absurd --> 10:energetic --> 11:exclusive --> 12:feeble --> 13:greasy --> 14:inadequate --> 15:intent --> 16:intricate --> 17:notorious --> 18:obscure --> 19:pathetic --> 2:appropriate --> 20:reluctant --> 21:solemn --> 3:barren --> 4:commentary --> 5:consistent --> 6:conspicuous --> 7:confidential --> 8:decent --> 9:delicate 
Level 0: _header --> 1:absurd --> 10:energetic --> 11:exclusive --> 12:feeble --> 13:greasy --> 14:inadequate --> 15:intent --> 16:intricate --> 17:notorious --> 18:obscure --> 19:pathetic --> 2:appropriate --> 20:reluctant --> 21:solemn --> 3:barren --> 4:commentary --> 5:consistent --> 6:conspicuous --> 7:confidential --> 8:decent --> 9:delicate
 */

    return 0;
}