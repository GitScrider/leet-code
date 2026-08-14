<sub>[🧠 Root](../../README.md) · [🧱 Data Structures](../README.md) · **➡️ Linear Structures**</sub>

# ➡️ Linear Structures

![structures](https://img.shields.io/badge/structures-7-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Sequential containers built from scratch — contiguous arrays, linked nodes, and the LIFO/FIFO adapters layered on top — each managing its own memory so the mechanics stay visible.

| File | Key ops (Big-O) | Description |
|---|---|---|
| [dynamic_array.cpp](dynamic_array.cpp) | index O(1), push_back amortized O(1), insert/erase O(n) | Growable contiguous array with capacity doubling |
| [singly_linked_list.cpp](singly_linked_list.cpp) | push_front/back O(1), pop_front O(1), search O(n) | Forward-linked nodes with head/tail pointers |
| [doubly_linked_list.cpp](doubly_linked_list.cpp) | push/pop at both ends O(1), erase node O(1) | Bidirectional nodes with prev/next links |
| [circular_linked_list.cpp](circular_linked_list.cpp) | push/pop at both ends O(1), rotate O(k) | Ring of nodes where tail links back to head |
| [stack.cpp](stack.cpp) | push/pop/top amortized O(1) | LIFO adapter over a growable buffer |
| [queue.cpp](queue.cpp) | enqueue/dequeue O(1) | FIFO queue backed by a ring buffer |
| [deque.cpp](deque.cpp) | push/pop at both ends amortized O(1) | Double-ended queue backed by a ring buffer |

## Build
```bash
g++ -std=c++17 -Wall -Wextra dynamic_array.cpp -o demo && ./demo
```
