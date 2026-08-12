<sub>[🧠 Root](../README.md) · [🎨 Design Patterns](../design-patterns/README.md) · **🧱 Data Structures** · [⚙️ Algorithms](../algorithms/README.md)</sub>

# 🧱 Data Structures in C++

![Structures](https://img.shields.io/badge/structures-21-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

From-scratch, generic (`template<typename T>`) implementations of core data structures in **C++17**,
built for study and reference. Each structure is implemented *from the ground up* — managing its own
memory (no wrapping `std::vector`/`std::list`/`std::deque`) — so the mechanics are visible.

Each file:

- opens with a header doc-comment: summary, an **operations & Big-O complexity** table, the internal
  invariants, and when-to-use trade-offs,
- implements the full **Rule of Five** (destructor + copy/move constructors and assignments) where it
  owns memory,
- ends with a `main()` that runs **`assert`-based tests** (including edge cases: empty, resize,
  wrap-around, copy/move independence) plus a short `std::cout` demo.

All comments are in **English**.

## Build & run

```bash
g++ -std=c++17 -Wall -Wextra linear/dynamic_array.cpp -o demo && ./demo
```

## Catalogue

### Linear

| Structure | Key operations (Big-O) | File |
|---|---|---|
| Dynamic Array | index O(1), push_back amortized O(1), insert/erase O(n) | [dynamic_array.cpp](linear/dynamic_array.cpp) |
| Singly Linked List | push_front/back O(1), pop_front O(1), search O(n) | [singly_linked_list.cpp](linear/singly_linked_list.cpp) |
| Doubly Linked List | push/pop at both ends O(1), erase node O(1) | [doubly_linked_list.cpp](linear/doubly_linked_list.cpp) |
| Stack (LIFO) | push/pop/top amortized O(1) | [stack.cpp](linear/stack.cpp) |
| Queue (FIFO, ring buffer) | enqueue/dequeue O(1) | [queue.cpp](linear/queue.cpp) |
| Deque (ring buffer) | push/pop at both ends amortized O(1) | [deque.cpp](linear/deque.cpp) |
| Circular Linked List | push/pop at both ends O(1), rotate O(k) | [circular_linked_list.cpp](linear/circular_linked_list.cpp) |

### Trees

| Structure | Key operations (Big-O, avg) | File |
|---|---|---|
| Binary Tree (traversals) | inorder/preorder/postorder/level-order O(n) | [binary_tree.cpp](trees/binary_tree.cpp) |
| Binary Search Tree | search/insert/remove O(log n) avg, O(n) worst | [binary_search_tree.cpp](trees/binary_search_tree.cpp) |
| Binary Heap (min-heap) | push/pop O(log n), peek O(1), build O(n) | [binary_heap.cpp](trees/binary_heap.cpp) |
| AVL Tree | search/insert/remove O(log n) guaranteed | [avl_tree.cpp](trees/avl_tree.cpp) |
| Red-Black Tree | search/insert/remove O(log n) guaranteed | [red_black_tree.cpp](trees/red_black_tree.cpp) |
| Trie (prefix tree) | insert/search/startsWith O(L), L = key length | [trie.cpp](trees/trie.cpp) |
| Segment Tree | range query + point update O(log n) | [segment_tree.cpp](trees/segment_tree.cpp) |
| Fenwick Tree (BIT) | prefix sum + point update O(log n) | [fenwick_tree.cpp](trees/fenwick_tree.cpp) |

### Hashing

| Structure | Key operations (Big-O) | File |
|---|---|---|
| Hash Table — separate chaining | put/get/erase O(1) avg, O(n) worst | [hash_table_chaining.cpp](hashing/hash_table_chaining.cpp) |
| Hash Table — open addressing (linear probing) | put/get/erase O(1) avg, O(n) worst | [hash_table_open_addressing.cpp](hashing/hash_table_open_addressing.cpp) |
| Hash functions tour (djb2, FNV-1a, rolling, int mix) | hash O(L) | [hash_functions.cpp](hashing/hash_functions.cpp) |

### Graphs

| Structure | Key operations (Big-O) | File |
|---|---|---|
| Graph — adjacency list | addEdge O(1), BFS/DFS O(V+E), space O(V+E) | [graph_adjacency_list.cpp](graphs/graph_adjacency_list.cpp) |
| Graph — adjacency matrix | hasEdge O(1), neighbors O(V), space O(V²) | [graph_adjacency_matrix.cpp](graphs/graph_adjacency_matrix.cpp) |
| Union-Find (DSU) | find/unite ~O(α(n)) amortized | [union_find.cpp](graphs/union_find.cpp) |

### Coming next

- **Algorithms** — see the top-level `algorithms/` folder (sorting, searching, graph algorithms, DP, …)

---

*Part of a personal study repository for algorithms and design patterns.*
