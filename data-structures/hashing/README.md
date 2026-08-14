<sub>[🧠 Root](../../README.md) · [🧱 Data Structures](../README.md) · **#️⃣ Hashing**</sub>

# #️⃣ Hashing

![structures](https://img.shields.io/badge/structures-3-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Hash-based structures built from scratch — two collision-resolution strategies for hash tables plus a tour of the hash functions that power them.

| File | Key ops (Big-O) | Description |
|---|---|---|
| [hash_table_chaining.cpp](hash_table_chaining.cpp) | put/get/erase O(1) avg, O(n) worst | Hash table using separate chaining (buckets of linked entries). |
| [hash_table_open_addressing.cpp](hash_table_open_addressing.cpp) | put/get/erase O(1) avg, O(n) worst | Hash table using open addressing with linear probing. |
| [hash_functions.cpp](hash_functions.cpp) | hash O(L) | Tour of hash functions: djb2, FNV-1a, rolling hash, integer mix. |

## Build
```bash
g++ -std=c++17 -Wall -Wextra hash_table_chaining.cpp -o demo && ./demo
```
