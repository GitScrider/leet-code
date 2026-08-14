<sub>[🧠 Root](../../README.md) · [⚙️ Algorithms](../README.md) · **🔀 Sorting**</sub>

# 🔀 Sorting

![algorithms](https://img.shields.io/badge/algorithms-10-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Classic sorting algorithms in C++17. Comparison sorts are generic (`template<typename T>`, ascending via `operator<`); counting/radix/bucket operate on integers.

| File | Complexity | Description |
|---|---|---|
| [bubble_sort.cpp](bubble_sort.cpp) | O(n²) | Repeatedly swaps adjacent out-of-order pairs; stable, in-place. |
| [insertion_sort.cpp](insertion_sort.cpp) | O(n²) | Builds a sorted prefix by inserting each element; stable, fast on nearly-sorted data. |
| [selection_sort.cpp](selection_sort.cpp) | O(n²) | Selects the minimum each pass and swaps it into place; in-place, not stable. |
| [shell_sort.cpp](shell_sort.cpp) | ~O(n^1.25) | Gapped insertion sort over diminishing increments; in-place. |
| [merge_sort.cpp](merge_sort.cpp) | O(n log n) | Divide-and-conquer merge of sorted halves; stable, O(n) space. |
| [quick_sort.cpp](quick_sort.cpp) | O(n log n) avg | Partitions around a pivot then recurses; in-place, O(n²) worst case. |
| [heap_sort.cpp](heap_sort.cpp) | O(n log n) | Builds a max-heap then repeatedly extracts the maximum; in-place. |
| [counting_sort.cpp](counting_sort.cpp) | O(n+k) | Counts key frequencies to place integers directly; stable, k = key range. |
| [radix_sort.cpp](radix_sort.cpp) | O(d·(n+b)) | LSD digit-by-digit stable counting-sort passes; d = digits, b = base. |
| [bucket_sort.cpp](bucket_sort.cpp) | O(n+k) | Distributes into buckets, sorts each, then concatenates; O(n²) worst case. |

## Build
```bash
g++ -std=c++17 -Wall -Wextra merge_sort.cpp -o demo && ./demo
```
