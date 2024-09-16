#include "skiplist.h"

using namespace std;

int main() { 
    int K, V, L; // K: key, V: value, L: level
    cin >> K >> V >> L; // input key, value, level
    
    // 调用构造函数
    Node<int, int> *node = new Node<int, int>(K, V, L); // create a new node
    cout << "Key: " << node->getKey() << endl; // output key
    cout << "Value: " << node->getValue() << endl; // output value

    // 调用析构函数
    delete node; // delete node

    return 0;
}