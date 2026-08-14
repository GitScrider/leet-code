<sub>[🧠 Root](../../README.md) · [⚙️ Algorithms](../README.md) · **♾️ NP-Complete / NP-Hard**</sub>

# ♾️ NP-Complete / NP-Hard

![problems](https://img.shields.io/badge/problems-10-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Classic intractable problems: an exact solver (exponential / pseudo-polynomial / Held-Karp / meet-in-the-middle) for small instances, plus a well-known approximation where one exists.

| File | Complexity | Description |
|---|---|---|
| [sat_3sat.cpp](sat_3sat.cpp) | O(2ⁿ) | Boolean satisfiability via DPLL backtracking |
| [traveling_salesman.cpp](traveling_salesman.cpp) | O(2ⁿ·n²) | Held-Karp exact + nearest-neighbor approximation |
| [hamiltonian_path_cycle.cpp](hamiltonian_path_cycle.cpp) | O(2ⁿ) | Path/cycle search via backtracking |
| [vertex_cover.cpp](vertex_cover.cpp) | O(2ⁿ) | Branching exact + 2-approximation |
| [max_clique.cpp](max_clique.cpp) | O(3^(n/3)) | Maximum clique via Bron-Kerbosch |
| [max_independent_set.cpp](max_independent_set.cpp) | O(2ⁿ) | Maximum independent set via branching |
| [graph_coloring_chromatic.cpp](graph_coloring_chromatic.cpp) | O(kⁿ) | Chromatic number via k-coloring search |
| [set_cover.cpp](set_cover.cpp) | O(2ⁿ) | Exact cover + greedy ln n-approximation |
| [subset_sum_meet_in_middle.cpp](subset_sum_meet_in_middle.cpp) | O(2^(n/2)) | Subset sum via meet-in-the-middle |
| [bin_packing.cpp](bin_packing.cpp) | O(2ⁿ) | Exact packing + first-fit-decreasing |

## Build
```bash
g++ -std=c++17 -Wall -Wextra traveling_salesman.cpp -o demo && ./demo
```
