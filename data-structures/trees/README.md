<sub>[🧠 Root](../../README.md) · [🧱 Data Structures](../README.md) · **🌳 Trees**</sub>

# 🌳 Trees

![structures](https://img.shields.io/badge/structures-8-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Hierarchical structures from scratch — traversals, ordered and self-balancing search trees, heaps, and the range-query trees used in competitive programming.

| File | Key ops (Big-O) | Description |
|---|---|---|
| [binary_tree.cpp](binary_tree.cpp) | inorder/preorder/postorder/level-order O(n) | Plain binary tree with the four core traversals |
| [binary_search_tree.cpp](binary_search_tree.cpp) | search/insert/remove O(log n) avg, O(n) worst | Ordered BST with left < node < right invariant |
| [binary_heap.cpp](binary_heap.cpp) | push/pop O(log n), peek O(1), build O(n) | Array-backed min-heap for priority queues |
| [avl_tree.cpp](avl_tree.cpp) | search/insert/remove O(log n) guaranteed | Height-balanced BST kept balanced by rotations |
| [red_black_tree.cpp](red_black_tree.cpp) | search/insert/remove O(log n) guaranteed | Color-balanced BST with recoloring and rotations |
| [trie.cpp](trie.cpp) | insert/search/startsWith O(L), L = key length | Prefix tree for string keys over an alphabet |
| [segment_tree.cpp](segment_tree.cpp) | range query + point update O(log n) | Tree over an array for associative range queries |
| [fenwick_tree.cpp](fenwick_tree.cpp) | prefix sum + point update O(log n) | Binary Indexed Tree for cumulative sums |

## Build
```bash
g++ -std=c++17 -Wall -Wextra binary_tree.cpp -o demo && ./demo
```
