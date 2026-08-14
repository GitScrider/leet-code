<sub>[🧠 Root](../../README.md) · [⚙️ Algorithms](../README.md) · **📊 Dynamic Programming**</sub>

# 📊 Dynamic Programming

![algorithms](https://img.shields.io/badge/algorithms-15-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Classic DP problems solved by overlapping subproblems and optimal substructure, spanning 1-D recurrences, knapsack variants, and 2-D sequence/grid tables.

| File | Complexity | Description |
|---|---|---|
| [climbing_stairs.cpp](climbing_stairs.cpp) | O(n) | Count ways to reach step n (Fibonacci recurrence). |
| [kadane_max_subarray.cpp](kadane_max_subarray.cpp) | O(n) | Maximum contiguous subarray sum. |
| [house_robber.cpp](house_robber.cpp) | O(n) | Max non-adjacent sum along a row of houses. |
| [coin_change.cpp](coin_change.cpp) | O(n·amount) | Minimum coins and number of ways to make an amount. |
| [knapsack_01.cpp](knapsack_01.cpp) | O(n·W) | 0/1 knapsack: maximize value under a weight cap. |
| [unbounded_knapsack.cpp](unbounded_knapsack.cpp) | O(n·W) | Knapsack with unlimited copies of each item. |
| [subset_sum_partition.cpp](subset_sum_partition.cpp) | O(n·sum) | Subset-sum feasibility and equal-partition test. |
| [rod_cutting.cpp](rod_cutting.cpp) | O(n²) | Maximize revenue by cutting a rod into pieces. |
| [longest_common_subsequence.cpp](longest_common_subsequence.cpp) | O(n·m) | Longest subsequence shared by two strings. |
| [longest_increasing_subsequence.cpp](longest_increasing_subsequence.cpp) | O(n log n) | Longest strictly increasing subsequence. |
| [edit_distance.cpp](edit_distance.cpp) | O(n·m) | Min insert/delete/replace edits between two strings. |
| [matrix_chain_multiplication.cpp](matrix_chain_multiplication.cpp) | O(n³) | Optimal parenthesization of a matrix product chain. |
| [longest_palindromic_subsequence.cpp](longest_palindromic_subsequence.cpp) | O(n²) | Longest subsequence that reads the same both ways. |
| [grid_paths.cpp](grid_paths.cpp) | O(n·m) | Count unique paths and find min-sum path in a grid. |
| [word_break.cpp](word_break.cpp) | O(n²) | Test if a string splits into dictionary words. |

## Build
```bash
g++ -std=c++17 -Wall -Wextra climbing_stairs.cpp -o demo && ./demo
```
