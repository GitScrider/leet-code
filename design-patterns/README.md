# Design Patterns in C++

A study & reference collection of the **23 Gang of Four (GoF) design patterns** (the 22 catalogued
by [refactoring.guru](https://refactoring.guru/design-patterns)), implemented in modern **C++17**.

Each pattern lives in a single, self-contained `.cpp` file that:

- opens with a header doc-comment (intent, when to use, real-world analogy, participants, pros/cons),
- implements the pattern with modern C++ idioms (smart pointers, `override`, `enum class`, RAII, const-correctness),
- ends with a small `main()` that demonstrates the pattern and prints example output.

All code comments are written in **English**.

## Layout

```
design-patterns/
├── creational/    # How objects get created
├── structural/    # How objects/classes are composed
└── behavioral/    # How objects communicate & share responsibility
```

## Build & run

Each file is an independent program. With any C++17 compiler:

```bash
g++ -std=c++17 -Wall -Wextra creational/singleton.cpp -o demo && ./demo
```

```bash
clang++ -std=c++17 -Wall -Wextra structural/decorator.cpp -o demo && ./demo
```

On Windows with MSVC:

```bash
cl /std:c++17 /EHsc behavioral\observer.cpp
```

## Catalogue

### Creational — object creation mechanisms

| Pattern | Intent | File |
|---|---|---|
| Factory Method | Defer instantiation to subclasses via a virtual creator method. | [factory_method.cpp](creational/factory_method.cpp) |
| Abstract Factory | Create families of related objects without naming concrete classes. | [abstract_factory.cpp](creational/abstract_factory.cpp) |
| Builder | Construct a complex object step by step. | [builder.cpp](creational/builder.cpp) |
| Prototype | Create new objects by cloning an existing instance. | [prototype.cpp](creational/prototype.cpp) |
| Singleton | Ensure a class has one instance with a global access point. | [singleton.cpp](creational/singleton.cpp) |

### Structural — composing classes & objects

| Pattern | Intent | File |
|---|---|---|
| Adapter | Make an incompatible interface usable by a client. | [adapter.cpp](structural/adapter.cpp) |
| Bridge | Decouple an abstraction from its implementation. | [bridge.cpp](structural/bridge.cpp) |
| Composite | Treat individual objects and compositions uniformly (trees). | [composite.cpp](structural/composite.cpp) |
| Decorator | Attach responsibilities to an object dynamically. | [decorator.cpp](structural/decorator.cpp) |
| Facade | Provide a simple interface over a complex subsystem. | [facade.cpp](structural/facade.cpp) |
| Flyweight | Share common state to support huge numbers of objects. | [flyweight.cpp](structural/flyweight.cpp) |
| Proxy | Provide a surrogate that controls access to another object. | [proxy.cpp](structural/proxy.cpp) |

### Behavioral — communication & responsibility

| Pattern | Intent | File |
|---|---|---|
| Chain of Responsibility | Pass a request along a chain of handlers. | [chain_of_responsibility.cpp](behavioral/chain_of_responsibility.cpp) |
| Command | Encapsulate a request as an object (supports undo). | [command.cpp](behavioral/command.cpp) |
| Iterator | Traverse a collection without exposing its representation. | [iterator.cpp](behavioral/iterator.cpp) |
| Mediator | Centralize complex communication between objects. | [mediator.cpp](behavioral/mediator.cpp) |
| Memento | Capture and restore an object's state (undo). | [memento.cpp](behavioral/memento.cpp) |
| Observer | Notify dependents automatically when a subject changes. | [observer.cpp](behavioral/observer.cpp) |
| State | Alter an object's behavior when its internal state changes. | [state.cpp](behavioral/state.cpp) |
| Strategy | Make a family of algorithms interchangeable at runtime. | [strategy.cpp](behavioral/strategy.cpp) |
| Template Method | Define an algorithm skeleton, deferring steps to subclasses. | [template_method.cpp](behavioral/template_method.cpp) |
| Visitor | Add operations to an object structure without changing it. | [visitor.cpp](behavioral/visitor.cpp) |

## Quick guide: which pattern?

- **Need to create objects flexibly?** → Creational (Factory Method, Abstract Factory, Builder, Prototype, Singleton)
- **Need to fit/compose objects together?** → Structural (Adapter, Bridge, Composite, Decorator, Facade, Flyweight, Proxy)
- **Need to coordinate behavior & responsibilities?** → Behavioral (the rest)

---

*Part of a personal study repository for algorithms and design patterns.*
