<sub>[🧠 Root](../../README.md) · [⚙️ Algorithms](../README.md) · **🔎 Searching**</sub>

# 🔎 Searching

![algorithms](https://img.shields.io/badge/algorithms-10-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Search algorithms in C++17, from a linear scan to logarithmic and sub-linear variants plus the binary-search-on-answer idiom.

| File | Complexity | Description |
|---|---|---|
| [linear_search.cpp](linear_search.cpp) | O(n) | Scans every element in sequence until the target is found. |
| [binary_search.cpp](binary_search.cpp) | O(log n) | Halves a sorted range each step to locate the target. |
| [binary_search_bounds.cpp](binary_search_bounds.cpp) | O(log n) | Lower/upper bound, first/last occurrence, and count in sorted data. |
| [search_rotated_array.cpp](search_rotated_array.cpp) | O(log n) | Binary search over a rotated sorted array. |
| [ternary_search.cpp](ternary_search.cpp) | O(log₃ n) | Finds the extremum of a unimodal function or array. |
| [jump_search.cpp](jump_search.cpp) | O(√n) | Jumps fixed-size blocks then linear-scans within one. |
| [interpolation_search.cpp](interpolation_search.cpp) | O(log log n) avg | Estimates position from value in uniformly distributed data. |
| [exponential_search.cpp](exponential_search.cpp) | O(log i) | Doubles a bound then binary-searches the found range. |
| [find_peak_element.cpp](find_peak_element.cpp) | O(log n) | Binary search toward the ascending neighbor to reach a peak. |
| [binary_search_on_answer.cpp](binary_search_on_answer.cpp) | O(n log range) | Binary search over the answer space with a monotonic feasibility check. |

## Build
```bash
g++ -std=c++17 -Wall -Wextra binary_search.cpp -o demo && ./demo
```
