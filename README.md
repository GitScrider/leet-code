<div align="center">

# 🧠 Algorithms, Data Structures & Design Patterns in C++

**A study & reference collection** — every file is self-contained, heavily commented in English,
and **verified to compile clean and pass its own tests**.

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Files](https://img.shields.io/badge/.cpp_files-139-1f6feb?style=flat-square)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)
![Tests](https://img.shields.io/badge/tests-passing-2ea043?style=flat-square)
![Flags](https://img.shields.io/badge/--Wall_--Wextra-clean-2ea043?style=flat-square)
![Comments](https://img.shields.io/badge/comments-English-8957e5?style=flat-square)

</div>

---

## 📖 About

A personal, growing library that reimplements the classic Computer-Science curriculum **from scratch**
in modern **C++17** — the Gang-of-Four design patterns, core data structures, and a broad sweep of
algorithms up to the classic NP-complete problems. It is built for **learning and reference**: each file
reads like a lesson, not just working code.

Every file:

- is a **single, self-contained translation unit** (compiles on its own, standard library only);
- opens with a **header doc-comment** — intent, a Big-O table, invariants, trade-offs;
- carries a **step-by-step complexity derivation** — instruction-count summations, recurrences solved by
  recursion tree / Master Theorem, and formal **O / Ω / Θ** bounds with per-case analysis;
- ends with a `main()` running **`assert`-based tests** (edge cases included) plus a short demo.

## ✨ What's inside

| Area | Count | Contents | Catalogue |
|---|:---:|---|:---:|
| 🎨 **Design Patterns** | 22 | All 22 GoF patterns — creational, structural, behavioral | [browse →](design-patterns/README.md) |
| 🧱 **Data Structures** | 21 | Linear, trees, hashing, graphs — from scratch with Rule of Five | [browse →](data-structures/README.md) |
| ⚙️ **Algorithms** | 96 | Sorting, searching, backtracking, graphs, DP, greedy, strings, math, NP-complete | [browse →](algorithms/README.md) |
| | **139** | total `.cpp` files | |

## 🗂️ Repository structure

```
leet-code/
├── design-patterns/          22  →  the 22 Gang-of-Four patterns
│   ├── creational/            5      Factory Method, Abstract Factory, Builder, Prototype, Singleton
│   ├── structural/            7      Adapter, Bridge, Composite, Decorator, Facade, Flyweight, Proxy
│   └── behavioral/           10      Chain of Resp., Command, Iterator, Mediator, Memento,
│                                     Observer, State, Strategy, Template Method, Visitor
├── data-structures/          21
│   ├── linear/                7      dynamic array, singly/doubly/circular lists, stack, queue, deque
│   ├── trees/                 8      BST, AVL, red-black, binary heap, trie, segment tree, Fenwick
│   ├── hashing/               3      separate chaining, open addressing, hash-function tour
│   └── graphs/                3      adjacency list, adjacency matrix, union-find (DSU)
└── algorithms/               96
    ├── sorting/              10      bubble, insertion, selection, shell, merge, quick, heap,
    │                                 counting, radix, bucket
    ├── searching/            10      linear, binary (+ bounds), rotated, ternary, jump,
    │                                 interpolation, exponential, peak, binary-search-on-answer
    ├── recursion-backtracking/ 13    Hanoi, permutations, subsets, N-Queens, Sudoku, maze, …
    ├── graphs/               13      BFS, DFS, topo sort, Dijkstra, Bellman-Ford, Floyd-Warshall,
    │                                 Kruskal, Prim, SCC, bridges, …
    ├── dynamic-programming/  15      knapsack, LCS, LIS, edit distance, matrix chain, …
    ├── greedy/                8      activity selection, Huffman, intervals, …
    ├── strings/               8      KMP, Rabin-Karp, Z, Manacher, Boyer-Moore, suffix array, Aho-Corasick
    ├── math/                  9      sieve, gcd/Euclid, fast/modular exponentiation, Miller-Rabin, …
    └── np-complete/          10      SAT, TSP, Hamiltonian, vertex cover, clique, set cover, bin packing, …
```

## ✅ Verified

> All **139** `.cpp` files compile **cleanly** under `g++ -std=c++17 -Wall -Wextra` (tested with
> **g++ 16.2.0**) and pass every one of their `assert`-based tests — **0 warnings, 0 failures**.

```
TOTAL=139   OK=139   WARN=0   COMPILE_FAIL=0   RUNTIME_FAIL=0
```

## ⚙️ Build & run

Each file is its own program. With any C++17 compiler:

```bash
g++ -std=c++17 -Wall -Wextra algorithms/sorting/merge_sort.cpp -o demo && ./demo
```

Build **everything** and run the test suites (bash):

```bash
find design-patterns data-structures algorithms -name '*.cpp' -print0 \
  | while IFS= read -r -d '' f; do \
      g++ -std=c++17 -Wall -Wextra "$f" -o /tmp/demo && /tmp/demo >/dev/null \
        && echo "OK   $f" || echo "FAIL $f"; \
    done
```

On Windows with MSVC: `cl /std:c++17 /EHsc <file>.cpp`  *(note: the `math/` files use `__int128`, a GCC/Clang extension).*

## 🧭 Sections

- 🎨 **[Design Patterns →](design-patterns/README.md)** — intent, real-world analogy, participants, and pros/cons for each pattern.
- 🧱 **[Data Structures →](data-structures/README.md)** — operation/complexity tables, invariants, and Rule-of-Five memory management.
- ⚙️ **[Algorithms →](algorithms/README.md)** — best/avg/worst tables, stability & in-place flags, and full O/Ω/Θ derivations.

---

<div align="center">
<sub>Built as a personal study repository — contributions are just future commits to my own understanding. 🚀</sub>
</div>
