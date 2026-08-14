<sub>[🧠 Root](../../README.md) · [🎨 Design Patterns](../README.md) · **🧩 Structural Patterns**</sub>

# 🧩 Structural Patterns

![patterns](https://img.shields.io/badge/patterns-7-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Patterns that compose classes and objects into larger structures while keeping those structures flexible and efficient.

| File | Pattern | Intent |
|---|---|---|
| [adapter.cpp](adapter.cpp) | Adapter | Make an incompatible interface usable by a client. |
| [bridge.cpp](bridge.cpp) | Bridge | Decouple an abstraction from its implementation. |
| [composite.cpp](composite.cpp) | Composite | Treat individual objects and compositions uniformly (trees). |
| [decorator.cpp](decorator.cpp) | Decorator | Attach responsibilities to an object dynamically. |
| [facade.cpp](facade.cpp) | Facade | Provide a simple interface over a complex subsystem. |
| [flyweight.cpp](flyweight.cpp) | Flyweight | Share common state to support huge numbers of objects. |
| [proxy.cpp](proxy.cpp) | Proxy | Provide a surrogate that controls access to another object. |

## Build
```bash
g++ -std=c++17 -Wall -Wextra decorator.cpp -o demo && ./demo
```
