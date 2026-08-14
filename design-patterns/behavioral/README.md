<sub>[🧠 Root](../../README.md) · [🎨 Design Patterns](../README.md) · **🔁 Behavioral Patterns**</sub>

# 🔁 Behavioral Patterns

![patterns](https://img.shields.io/badge/patterns-10-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Patterns that describe how objects communicate, distribute responsibility, and coordinate control flow at runtime.

| File | Pattern | Intent |
|---|---|---|
| [chain_of_responsibility.cpp](chain_of_responsibility.cpp) | Chain of Responsibility | Pass a request along a chain of handlers. |
| [command.cpp](command.cpp) | Command | Encapsulate a request as an object (supports undo). |
| [iterator.cpp](iterator.cpp) | Iterator | Traverse a collection without exposing its representation. |
| [mediator.cpp](mediator.cpp) | Mediator | Centralize complex communication between objects. |
| [memento.cpp](memento.cpp) | Memento | Capture and restore an object's state (undo). |
| [observer.cpp](observer.cpp) | Observer | Notify dependents automatically when a subject changes. |
| [state.cpp](state.cpp) | State | Alter an object's behavior when its internal state changes. |
| [strategy.cpp](strategy.cpp) | Strategy | Make a family of algorithms interchangeable at runtime. |
| [template_method.cpp](template_method.cpp) | Template Method | Define an algorithm skeleton, deferring steps to subclasses. |
| [visitor.cpp](visitor.cpp) | Visitor | Add operations to an object structure without changing it. |

## Build
```bash
g++ -std=c++17 -Wall -Wextra observer.cpp -o demo && ./demo
```
