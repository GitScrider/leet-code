<sub>[🧠 Root](../../README.md) · [⚙️ Algorithms](../README.md) · **💰 Greedy**</sub>

# 💰 Greedy

![algorithms](https://img.shields.io/badge/algorithms-8-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Problems solved by making the locally optimal choice at each step, most driven by a sort plus a single pass; includes a case where greedy provably fails.

| File | Complexity | Description |
|---|---|---|
| [activity_selection.cpp](activity_selection.cpp) | O(n log n) | Max non-overlapping activities by earliest finish time. |
| [fractional_knapsack.cpp](fractional_knapsack.cpp) | O(n log n) | Maximize value taking item fractions by value density. |
| [huffman_coding.cpp](huffman_coding.cpp) | O(n log n) | Optimal prefix codes from a min-heap of frequencies. |
| [job_sequencing.cpp](job_sequencing.cpp) | O(n log n) | Schedule deadline jobs to maximize total profit. |
| [merge_intervals.cpp](merge_intervals.cpp) | O(n log n) | Merge overlapping intervals after sorting by start. |
| [gas_station.cpp](gas_station.cpp) | O(n) | Find the start index to complete a circular route. |
| [minimum_platforms.cpp](minimum_platforms.cpp) | O(n log n) | Min platforms for overlapping train schedules. |
| [coin_change_greedy.cpp](coin_change_greedy.cpp) | O(n) | Greedy coin count plus a counterexample it fails. |

## Build
```bash
g++ -std=c++17 -Wall -Wextra activity_selection.cpp -o demo && ./demo
```
