# ApexKV

Fast key-value store using a hash table with separate chaining.

## Features

- Insert, search, and remove operations
- Dynamic resizing when load factor exceeds threshold
- O(1) average time complexity for insert/search/remove

## Build & Run

```bash
g++ -std=c++17 -I include src/hash_table.cpp main.cpp -o apexkv.exe
./apexkv.exe
```

## Progress

- Day 1: basic chaining hash table banaya (insert, search).
- Day 2: load factor check aur resize add kiya (rehash).

## Next

- remove aur tests improve karne hain.
Save karne ke baad:

bash
git add .
git commit -m "Day 2: update README"
git push
Agar chaho to isse pehle wale Day 2 commit ke saath hi merge kar sakte the, par ab alag commit bhi theek hai.

Aage har day ke end mein main tumhe README ki exact lines tumhare style mein de dunga, bas copy-paste karna hoga.

english me hi
Got it. English mein hi, but simple and handwritten style — short sentences, no heavy English.

Update README.md with this:

text
# ApexKV

Simple hash table based key-value store.

## Features

- insert, search, remove
- dynamic resizing when load factor gets high
- average O(1) time for insert/search/remove

## Build & Run

```bash
g++ -std=c++17 -I include src/hash_table.cpp main.cpp -o apexkv.exe
./apexkv.exe
```

## Progress

- Day 1: basic chaining hash table (insert, search).
- Day 2: added load factor check and resize (rehash).

## Next

- improve remove and add more tests.