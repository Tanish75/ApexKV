#pragma once
#include <vector>
#include <string>
using namespace std;


// Node structure
struct Node {
    string key;
    string value;
    Node* next;

    Node(string k, string v);
};


// Hash Table class
class HashTable {
    vector<Node*> buckets;
    int numBuckets;
    int size;

    int hash(string key);

public:
   HashTable(int bucketCount = 10);
    ~HashTable();

    void put(string key, string value);
    string get(string key);
    bool remove(string key);
    int getSize();
    void print();
};
