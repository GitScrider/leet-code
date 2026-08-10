# Algorithms in C++

Classic algorithms implemented in **C++17** for study and reference. Each file is
self-contained, commented in **English**, opens with a header doc-comment (idea +
complexity table + properties), and ends with a `main()` that runs **`assert`-based
tests** (edge cases included) plus a short demo.

## Build & run

```bash
g++ -std=c++17 -Wall -Wextra sorting/merge_sort.cpp -o demo && ./demo
```

## Sorting

Comparison sorts are generic (`template<typename T>`, ascending via `operator<`);
counting/radix/bucket operate on integers.

| Algorithm | Best | Average | Worst | Space | Stable | In-place |
|---|---|---|---|---|---|---|
| [Bubble](sorting/bubble_sort.cpp) | O(n) | O(n²) | O(n²) | O(1) | ✅ | ✅ |
| [Insertion](sorting/insertion_sort.cpp) | O(n) | O(n²) | O(n²) | O(1) | ✅ | ✅ |
| [Selection](sorting/selection_sort.cpp) | O(n²) | O(n²) | O(n²) | O(1) | ❌ | ✅ |
| [Shell](sorting/shell_sort.cpp) | O(n log n) | ~O(n^1.25) | O(n²) | O(1) | ❌ | ✅ |
| [Merge](sorting/merge_sort.cpp) | O(n log n) | O(n log n) | O(n log n) | O(n) | ✅ | ❌ |
| [Quick](sorting/quick_sort.cpp) | O(n log n) | O(n log n) | O(n²) | O(log n) | ❌ | ✅ |
| [Heap](sorting/heap_sort.cpp) | O(n log n) | O(n log n) | O(n log n) | O(1) | ❌ | ✅ |
| [Counting](sorting/counting_sort.cpp) | O(n+k) | O(n+k) | O(n+k) | O(n+k) | ✅ | ❌ |
| [Radix (LSD)](sorting/radix_sort.cpp) | O(d·(n+b)) | O(d·(n+b)) | O(d·(n+b)) | O(n+b) | ✅ | ❌ |
| [Bucket](sorting/bucket_sort.cpp) | O(n+k) | O(n+k) | O(n²) | O(n+k) | ✅* | ❌ |

<sub>*Bucket sort stability depends on the per-bucket sort; see the file's header note. `k` = key range, `d` = digit count, `b` = radix base.</sub>

## Searching

| Algorithm | Time | File |
|---|---|---|
| Linear search | O(n) | [linear_search.cpp](searching/linear_search.cpp) |
| Binary search | O(log n) | [binary_search.cpp](searching/binary_search.cpp) |
| Binary search bounds (lower/upper/first/last/count) | O(log n) | [binary_search_bounds.cpp](searching/binary_search_bounds.cpp) |
| Search in rotated array | O(log n) | [search_rotated_array.cpp](searching/search_rotated_array.cpp) |
| Ternary search (unimodal) | O(log₃ n) | [ternary_search.cpp](searching/ternary_search.cpp) |
| Jump search | O(√n) | [jump_search.cpp](searching/jump_search.cpp) |
| Interpolation search | O(log log n) avg | [interpolation_search.cpp](searching/interpolation_search.cpp) |
| Exponential search | O(log i) | [exponential_search.cpp](searching/exponential_search.cpp) |
| Find peak element | O(log n) | [find_peak_element.cpp](searching/find_peak_element.cpp) |
| Binary search on the answer | O(n log range) | [binary_search_on_answer.cpp](searching/binary_search_on_answer.cpp) |

## Recursion & Backtracking

| Problem | Time | File |
|---|---|---|
| Recursion basics (factorial, fib, gcd, power) | varies | [recursion_basics.cpp](recursion-backtracking/recursion_basics.cpp) |
| Tower of Hanoi | O(2ⁿ) | [tower_of_hanoi.cpp](recursion-backtracking/tower_of_hanoi.cpp) |
| Permutations | O(n·n!) | [permutations.cpp](recursion-backtracking/permutations.cpp) |
| Subsets (power set) | O(n·2ⁿ) | [subsets.cpp](recursion-backtracking/subsets.cpp) |
| Combinations (n choose k) | O(k·C(n,k)) | [combinations.cpp](recursion-backtracking/combinations.cpp) |
| Combination sum | exponential | [combination_sum.cpp](recursion-backtracking/combination_sum.cpp) |
| N-Queens | O(n!) | [n_queens.cpp](recursion-backtracking/n_queens.cpp) |
| Sudoku solver | exponential | [sudoku_solver.cpp](recursion-backtracking/sudoku_solver.cpp) |
| Rat in a maze | exponential | [rat_in_maze.cpp](recursion-backtracking/rat_in_maze.cpp) |
| Word search (grid) | exponential | [word_search.cpp](recursion-backtracking/word_search.cpp) |
| Generate parentheses | Catalan(n) | [generate_parentheses.cpp](recursion-backtracking/generate_parentheses.cpp) |
| Palindrome partitioning | exponential | [palindrome_partitioning.cpp](recursion-backtracking/palindrome_partitioning.cpp) |
| Graph coloring (m-coloring) | O(mⁿ) | [graph_coloring.cpp](recursion-backtracking/graph_coloring.cpp) |

## Graph Algorithms

| Algorithm | Time | File |
|---|---|---|
| BFS (traversal + shortest path) | O(V+E) | [bfs.cpp](graphs/bfs.cpp) |
| DFS (recursive + iterative) | O(V+E) | [dfs.cpp](graphs/dfs.cpp) |
| Topological sort (Kahn + DFS) | O(V+E) | [topological_sort.cpp](graphs/topological_sort.cpp) |
| Dijkstra | O((V+E) log V) | [dijkstra.cpp](graphs/dijkstra.cpp) |
| Bellman-Ford (+ neg. cycle) | O(V·E) | [bellman_ford.cpp](graphs/bellman_ford.cpp) |
| Floyd-Warshall (all-pairs) | O(V³) | [floyd_warshall.cpp](graphs/floyd_warshall.cpp) |
| Kruskal MST | O(E log E) | [kruskal_mst.cpp](graphs/kruskal_mst.cpp) |
| Prim MST | O((V+E) log V) | [prim_mst.cpp](graphs/prim_mst.cpp) |
| Connected components | O(V+E) | [connected_components.cpp](graphs/connected_components.cpp) |
| Cycle detection (directed + undirected) | O(V+E) | [cycle_detection.cpp](graphs/cycle_detection.cpp) |
| Strongly connected components | O(V+E) | [strongly_connected_components.cpp](graphs/strongly_connected_components.cpp) |
| Bipartite check | O(V+E) | [bipartite_check.cpp](graphs/bipartite_check.cpp) |
| Articulation points & bridges | O(V+E) | [articulation_points_bridges.cpp](graphs/articulation_points_bridges.cpp) |

## Coming next

- **Dynamic programming** — knapsack, LIS, LCS, edit distance, coin change
- **Greedy** — activity selection, Huffman, interval scheduling
- **Strings** — KMP, Rabin-Karp, Z-algorithm
- **Math** — sieve of Eratosthenes, gcd/lcm, fast exponentiation, modular arithmetic

---

*Part of a personal study repository for algorithms and design patterns.*
