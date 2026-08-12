<sub>[🧠 Root](../README.md) · [🎨 Design Patterns](../design-patterns/README.md) · [🧱 Data Structures](../data-structures/README.md) · **⚙️ Algorithms**</sub>

# ⚙️ Algorithms in C++

![Algorithms](https://img.shields.io/badge/algorithms-96-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Classic algorithms implemented in **C++17** for study and reference. Each file is
self-contained, commented in **English**, opens with a header doc-comment (idea +
complexity table + properties), a **step-by-step complexity derivation** (instruction-count
summations / recurrences + formal **O / Ω / Θ** bounds), and ends with a `main()` that runs
**`assert`-based tests** (edge cases included) plus a short demo.

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

## Dynamic Programming

| Problem | Time | File |
|---|---|---|
| Climbing stairs | O(n) | [climbing_stairs.cpp](dynamic-programming/climbing_stairs.cpp) |
| Kadane's max subarray | O(n) | [kadane_max_subarray.cpp](dynamic-programming/kadane_max_subarray.cpp) |
| House robber | O(n) | [house_robber.cpp](dynamic-programming/house_robber.cpp) |
| Coin change (min + ways) | O(n·amount) | [coin_change.cpp](dynamic-programming/coin_change.cpp) |
| 0/1 knapsack | O(n·W) | [knapsack_01.cpp](dynamic-programming/knapsack_01.cpp) |
| Unbounded knapsack | O(n·W) | [unbounded_knapsack.cpp](dynamic-programming/unbounded_knapsack.cpp) |
| Subset sum & partition | O(n·sum) | [subset_sum_partition.cpp](dynamic-programming/subset_sum_partition.cpp) |
| Rod cutting | O(n²) | [rod_cutting.cpp](dynamic-programming/rod_cutting.cpp) |
| Longest common subsequence | O(n·m) | [longest_common_subsequence.cpp](dynamic-programming/longest_common_subsequence.cpp) |
| Longest increasing subsequence | O(n log n) | [longest_increasing_subsequence.cpp](dynamic-programming/longest_increasing_subsequence.cpp) |
| Edit distance | O(n·m) | [edit_distance.cpp](dynamic-programming/edit_distance.cpp) |
| Matrix chain multiplication | O(n³) | [matrix_chain_multiplication.cpp](dynamic-programming/matrix_chain_multiplication.cpp) |
| Longest palindromic subsequence | O(n²) | [longest_palindromic_subsequence.cpp](dynamic-programming/longest_palindromic_subsequence.cpp) |
| Grid paths (unique + min sum) | O(n·m) | [grid_paths.cpp](dynamic-programming/grid_paths.cpp) |
| Word break | O(n²) | [word_break.cpp](dynamic-programming/word_break.cpp) |

## Greedy

| Problem | Time | File |
|---|---|---|
| Activity selection | O(n log n) | [activity_selection.cpp](greedy/activity_selection.cpp) |
| Fractional knapsack | O(n log n) | [fractional_knapsack.cpp](greedy/fractional_knapsack.cpp) |
| Huffman coding | O(n log n) | [huffman_coding.cpp](greedy/huffman_coding.cpp) |
| Job sequencing | O(n log n) | [job_sequencing.cpp](greedy/job_sequencing.cpp) |
| Merge intervals | O(n log n) | [merge_intervals.cpp](greedy/merge_intervals.cpp) |
| Gas station | O(n) | [gas_station.cpp](greedy/gas_station.cpp) |
| Minimum platforms | O(n log n) | [minimum_platforms.cpp](greedy/minimum_platforms.cpp) |
| Coin change (greedy + counterexample) | O(n) | [coin_change_greedy.cpp](greedy/coin_change_greedy.cpp) |

## Strings

| Algorithm | Time | File |
|---|---|---|
| Naive matching (baseline) | O(n·m) | [naive_string_match.cpp](strings/naive_string_match.cpp) |
| KMP | O(n+m) | [kmp.cpp](strings/kmp.cpp) |
| Rabin-Karp (rolling hash) | O(n+m) avg | [rabin_karp.cpp](strings/rabin_karp.cpp) |
| Z-algorithm | O(n+m) | [z_algorithm.cpp](strings/z_algorithm.cpp) |
| Manacher (longest palindrome) | O(n) | [manacher.cpp](strings/manacher.cpp) |
| Boyer-Moore | O(n/m) best | [boyer_moore.cpp](strings/boyer_moore.cpp) |
| Suffix array + LCP (Kasai) | O(n log n) | [suffix_array_lcp.cpp](strings/suffix_array_lcp.cpp) |
| Aho-Corasick (multi-pattern) | O(n+m+z) | [aho_corasick.cpp](strings/aho_corasick.cpp) |

## Math / Number Theory

| Algorithm | Time | File |
|---|---|---|
| GCD/LCM + extended Euclid | O(log n) | [gcd_lcm_extended.cpp](math/gcd_lcm_extended.cpp) |
| Sieve of Eratosthenes | O(n log log n) | [sieve_of_eratosthenes.cpp](math/sieve_of_eratosthenes.cpp) |
| Fast exponentiation (binary/modular) | O(log n) | [fast_exponentiation.cpp](math/fast_exponentiation.cpp) |
| Modular inverse (Fermat + Euclid) | O(log n) | [modular_inverse.cpp](math/modular_inverse.cpp) |
| Prime factorization | O(√n) | [prime_factorization.cpp](math/prime_factorization.cpp) |
| Miller-Rabin primality | O(k log³ n) | [miller_rabin.cpp](math/miller_rabin.cpp) |
| Combinatorics (nCr, Pascal, nCr mod p) | O(n) | [combinatorics.cpp](math/combinatorics.cpp) |
| Euler's totient | O(√n) / sieve | [euler_totient.cpp](math/euler_totient.cpp) |
| Matrix exponentiation (fast Fibonacci) | O(log n) | [matrix_exponentiation.cpp](math/matrix_exponentiation.cpp) |

## NP-Complete / NP-Hard

Classic intractable problems: an **exact solver** (exponential / pseudo-polynomial /
Held-Karp / meet-in-the-middle) for small instances, plus a well-known **approximation**
where one exists. Each header explains why the problem is hard.

| Problem | Class | Exact approach (+ approx) | File |
|---|---|---|---|
| SAT / 3-SAT | NP-complete | DPLL backtracking | [sat_3sat.cpp](np-complete/sat_3sat.cpp) |
| Traveling salesman | NP-hard | Held-Karp O(2ⁿ·n²) + nearest-neighbor | [traveling_salesman.cpp](np-complete/traveling_salesman.cpp) |
| Hamiltonian path/cycle | NP-complete | backtracking | [hamiltonian_path_cycle.cpp](np-complete/hamiltonian_path_cycle.cpp) |
| Vertex cover | NP-complete | branching + 2-approximation | [vertex_cover.cpp](np-complete/vertex_cover.cpp) |
| Maximum clique | NP-hard | Bron-Kerbosch | [max_clique.cpp](np-complete/max_clique.cpp) |
| Maximum independent set | NP-hard | branching | [max_independent_set.cpp](np-complete/max_independent_set.cpp) |
| Chromatic number | NP-hard | k-coloring search | [graph_coloring_chromatic.cpp](np-complete/graph_coloring_chromatic.cpp) |
| Set cover | NP-hard | exact + greedy ln n-approx | [set_cover.cpp](np-complete/set_cover.cpp) |
| Subset sum | NP-complete | meet-in-the-middle O(2^(n/2)) | [subset_sum_meet_in_middle.cpp](np-complete/subset_sum_meet_in_middle.cpp) |
| Bin packing | NP-hard | exact + first-fit-decreasing | [bin_packing.cpp](np-complete/bin_packing.cpp) |

---

*Part of a personal study repository for algorithms and design patterns.*
