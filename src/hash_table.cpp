#include "../include/hash_table.hpp"
#include <iostream>
using namespace std;


// Node constructor
Node::Node(string k, string v) {
    key = k;
    value = v;
    next = NULL;
}


// HashTable constructor
HashTable::HashTable(int bucketCount) {
    numBuckets = bucketCount;
    size = 0;
    
    buckets.resize(numBuckets, NULL);
}


// HashTable destructor
HashTable::~HashTable() {
    for(int i = 0; i < numBuckets; i++) {
        Node* current = buckets[i];
        while(current != NULL) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }
}


// Hash function
int HashTable::hash(string key) {
    int sum = 0;
    for(char c : key) {
        sum += c;
    }
    return sum % numBuckets;
}


// PUT operation
void HashTable::put(string key, string value) {
    int index = hash(key);

    Node* current = buckets[index];
    while(current != NULL) {
        if(current->key == key) {
            current->value = value;
            return;
        }
        current = current->next;
    }

    Node* newNode = new Node(key, value);
    newNode->next = buckets[index];
    buckets[index] = newNode;
    size++;
}


// GET operation
string HashTable::get(string key) {
    int index = hash(key);

    Node* current = buckets[index];
    while(current != NULL) {
        if(current->key == key) {
            return current->value;
        }
        current = current->next;
    }

    return "";
}


// REMOVE operation
bool HashTable::remove(string key) {
    int index = hash(key);

    Node* current = buckets[index];
    Node* prev = NULL;

    while(current != NULL) {
        if(current->key == key) {
            if(prev == NULL) {
                buckets[index] = current->next;
            } else {
                prev->next = current->next;
            }
            delete current;
            size--;
            return true;
        }
        prev = current;
        current = current->next;
    }

    return false;
}


// GET SIZE
int HashTable::getSize() {
    return size;
}


// PRINT
void HashTable::print() {
    for(int i = 0; i < numBuckets; i++) {
        cout << "Bucket " << i << ": ";
        Node* current = buckets[i];
        while(current != NULL) {
            cout << "(" << current->key << "," << current->value << ") -> ";
            current = current->next;
        }
        cout << "NULL" << endl;
    }
}
