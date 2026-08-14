<sub>[🧠 Root](../../README.md) · [🎨 Design Patterns](../README.md) · **🏗️ Creational Patterns**</sub>

# 🏗️ Creational Patterns

![patterns](https://img.shields.io/badge/patterns-5-1f6feb?style=flat-square)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-2ea043?style=flat-square)

Patterns that abstract the object-creation process, letting a system stay independent of how its objects are made, composed, and represented.

| File | Pattern | Intent |
|---|---|---|
| [factory_method.cpp](factory_method.cpp) | Factory Method | Defer instantiation to subclasses via a virtual creator method. |
| [abstract_factory.cpp](abstract_factory.cpp) | Abstract Factory | Create families of related objects without naming concrete classes. |
| [builder.cpp](builder.cpp) | Builder | Construct a complex object step by step. |
| [prototype.cpp](prototype.cpp) | Prototype | Create new objects by cloning an existing instance. |
| [singleton.cpp](singleton.cpp) | Singleton | Ensure a class has one instance with a global access point. |

## Build
```bash
g++ -std=c++17 -Wall -Wextra singleton.cpp -o demo && ./demo
```
