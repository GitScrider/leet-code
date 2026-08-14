<sub>[🧠 Root](../../README.md) · [⚙️ Algorithms](../README.md) · **🔤 Strings**</sub>

# 🔤 Strings

![algorithms](https://img.shields.io/badge/algorithms-8-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Pattern matching and string-analysis algorithms, from the brute-force baseline to linear-time automata, rolling hashes, palindrome scanning, and multi-pattern search.

| File | Complexity | Description |
|---|---|---|
| [naive_string_match.cpp](naive_string_match.cpp) | O(n·m) | Brute-force substring search baseline |
| [kmp.cpp](kmp.cpp) | O(n+m) | Knuth-Morris-Pratt with failure/prefix function |
| [rabin_karp.cpp](rabin_karp.cpp) | O(n+m) avg | Rolling-hash matching |
| [z_algorithm.cpp](z_algorithm.cpp) | O(n+m) | Z-array for prefix matches and pattern search |
| [manacher.cpp](manacher.cpp) | O(n) | Longest palindromic substring |
| [boyer_moore.cpp](boyer_moore.cpp) | O(n/m) best | Bad-character / good-suffix skipping |
| [suffix_array_lcp.cpp](suffix_array_lcp.cpp) | O(n log n) | Suffix array + LCP via Kasai's algorithm |
| [aho_corasick.cpp](aho_corasick.cpp) | O(n+m+z) | Multi-pattern matching automaton |

## Build
```bash
g++ -std=c++17 -Wall -Wextra kmp.cpp -o demo && ./demo
```
