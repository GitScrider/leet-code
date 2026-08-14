<sub>[🧠 Root](../../README.md) · [⚙️ Algorithms](../README.md) · **🧭 Recursion & Backtracking**</sub>

# 🧭 Recursion & Backtracking

![algorithms](https://img.shields.io/badge/algorithms-13-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Recursive problem-solving and systematic search: from basic recurrences to backtracking that builds candidates incrementally and prunes dead ends.

| File | Complexity | Description |
|---|---|---|
| [recursion_basics.cpp](recursion_basics.cpp) | varies | Recursion fundamentals: factorial, Fibonacci, GCD, power |
| [tower_of_hanoi.cpp](tower_of_hanoi.cpp) | O(2ⁿ) | Move a stack of disks across three pegs |
| [permutations.cpp](permutations.cpp) | O(n·n!) | Generate all orderings of the input |
| [subsets.cpp](subsets.cpp) | O(n·2ⁿ) | Enumerate the power set |
| [combinations.cpp](combinations.cpp) | O(k·C(n,k)) | Choose k elements from n |
| [combination_sum.cpp](combination_sum.cpp) | exponential | Find combinations summing to a target |
| [n_queens.cpp](n_queens.cpp) | O(n!) | Place n non-attacking queens on a board |
| [sudoku_solver.cpp](sudoku_solver.cpp) | exponential | Fill a 9×9 grid by constraint backtracking |
| [rat_in_maze.cpp](rat_in_maze.cpp) | exponential | Find paths from corner to corner of a grid |
| [word_search.cpp](word_search.cpp) | exponential | Search a word along adjacent grid cells |
| [generate_parentheses.cpp](generate_parentheses.cpp) | Catalan(n) | Produce all valid balanced parenthesis strings |
| [palindrome_partitioning.cpp](palindrome_partitioning.cpp) | exponential | Split a string into palindromic substrings |
| [graph_coloring.cpp](graph_coloring.cpp) | O(mⁿ) | Color vertices with m colors (m-coloring) |

## Build
```bash
g++ -std=c++17 -Wall -Wextra n_queens.cpp -o demo && ./demo
```
