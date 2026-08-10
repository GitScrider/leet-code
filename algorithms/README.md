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

## Coming next

- **Searching** — linear, binary search (+ lower/upper bound variants)
- **Recursion & backtracking** — permutations, subsets, N-Queens
- **Graph algorithms** — BFS/DFS, Dijkstra, Bellman-Ford, topological sort, MST (Kruskal/Prim)
- **Dynamic programming** — knapsack, LIS, LCS, edit distance, coin change
- **Greedy**, **Strings** (KMP, Rabin-Karp, Z), **Math** (sieve, gcd, fast exponentiation)

---

*Part of a personal study repository for algorithms and design patterns.*
