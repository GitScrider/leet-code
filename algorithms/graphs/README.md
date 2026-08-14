<sub>[🧠 Root](../../README.md) · [⚙️ Algorithms](../README.md) · **🕸️ Graph Algorithms**</sub>

# 🕸️ Graph Algorithms

![algorithms](https://img.shields.io/badge/algorithms-13-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Traversal, shortest paths, minimum spanning trees, and connectivity on directed and undirected graphs.

| File | Complexity | Description |
|---|---|---|
| [bfs.cpp](bfs.cpp) | O(V+E) | Breadth-first traversal + unweighted shortest path |
| [dfs.cpp](dfs.cpp) | O(V+E) | Depth-first traversal, recursive and iterative |
| [topological_sort.cpp](topological_sort.cpp) | O(V+E) | Order a DAG via Kahn and DFS |
| [dijkstra.cpp](dijkstra.cpp) | O((V+E) log V) | Single-source shortest paths, non-negative weights |
| [bellman_ford.cpp](bellman_ford.cpp) | O(V·E) | Shortest paths with negative-cycle detection |
| [floyd_warshall.cpp](floyd_warshall.cpp) | O(V³) | All-pairs shortest paths |
| [kruskal_mst.cpp](kruskal_mst.cpp) | O(E log E) | Minimum spanning tree via union-find |
| [prim_mst.cpp](prim_mst.cpp) | O((V+E) log V) | Minimum spanning tree via priority queue |
| [connected_components.cpp](connected_components.cpp) | O(V+E) | Label components of an undirected graph |
| [cycle_detection.cpp](cycle_detection.cpp) | O(V+E) | Detect cycles in directed and undirected graphs |
| [strongly_connected_components.cpp](strongly_connected_components.cpp) | O(V+E) | Find SCCs of a directed graph |
| [bipartite_check.cpp](bipartite_check.cpp) | O(V+E) | Test 2-colorability of a graph |
| [articulation_points_bridges.cpp](articulation_points_bridges.cpp) | O(V+E) | Find cut vertices and bridges |

## Build
```bash
g++ -std=c++17 -Wall -Wextra dijkstra.cpp -o demo && ./demo
```
