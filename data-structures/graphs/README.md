<sub>[🧠 Root](../../README.md) · [🧱 Data Structures](../README.md) · **🕸️ Graph Structures**</sub>

# 🕸️ Graph Structures

![structures](https://img.shields.io/badge/structures-3-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Ways to represent and connect graphs — the two classic adjacency representations plus the disjoint-set structure behind connectivity queries.

| File | Key ops (Big-O) | Description |
|---|---|---|
| [graph_adjacency_list.cpp](graph_adjacency_list.cpp) | addEdge O(1), BFS/DFS O(V+E), space O(V+E) | Graph stored as adjacency lists; edge-sparse friendly. |
| [graph_adjacency_matrix.cpp](graph_adjacency_matrix.cpp) | hasEdge O(1), neighbors O(V), space O(V²) | Graph stored as an adjacency matrix; O(1) edge lookups. |
| [union_find.cpp](union_find.cpp) | find/unite ~O(α(n)) amortized | Union-Find (DSU) with path compression and union by size. |

## Build
```bash
g++ -std=c++17 -Wall -Wextra graph_adjacency_list.cpp -o demo && ./demo
```
