/*
 * Template Method (Behavioral)
 *
 * Intent: Define the skeleton of an algorithm in a base-class method, deferring
 * some steps to subclasses. Subclasses redefine certain steps without changing
 * the algorithm's overall structure.
 *
 * Problem / When to use it:
 *  - Several algorithms share the same overall structure but differ in a few
 *    steps.
 *  - You want to lock the invariant sequence of steps in one place.
 *  - You want to let subclasses customize behavior only at well-defined points.
 *
 * Real-world analogy: Making tea and coffee follow the same recipe skeleton -
 * boil water, brew, pour, add condiments - but "brew" and "condiments" differ.
 *
 * Participants:
 *  - AbstractClass (Beverage): defines the template method and declares the
 *    primitive (abstract) operations and optional hooks.
 *  - ConcreteClass (Tea, Coffee): implement the primitive operations and may
 *    override hooks.
 *
 * Trade-offs:
 *  Pros:
 *   - Removes duplication by hoisting the common skeleton into the base class.
 *   - Enforces the invariant step order (inversion of control / "don't call
 *     us, we'll call you").
 *  Cons:
 *   - Relies on inheritance, which is more rigid than composition (Strategy).
 *   - The skeleton can grow hard to follow as the number of hooks increases.
 */

#include <iostream>
#include <string>

// --- Abstract Class ---
class Beverage {
public:
    virtual ~Beverage() = default;

    // The Template Method: a fixed skeleton. Because it is non-virtual,
    // subclasses cannot override it and therefore cannot alter the step
    // ordering; they customize only the virtual primitives and hooks it calls.
    void prepare() const {
        boilWater();
        brew();                       // primitive operation (must override)
        pourInCup();
        if (customerWantsCondiments()) // hook: subclass may opt out
            addCondiments();          // primitive operation (must override)
        std::cout << "  " << name() << " is ready.\n";
    }

protected:
    // Primitive operations: the variable steps each beverage must supply.
    virtual void brew() const = 0;
    virtual void addCondiments() const = 0;
    virtual std::string name() const = 0;

    // Hook: a virtual with a default that subclasses may override to influence
    // the skeleton. Defaults to "yes".
    virtual bool customerWantsCondiments() const { return true; }

private:
    // Invariant steps shared by every beverage; not overridable.
    void boilWater() const { std::cout << "  Boiling water\n"; }
    void pourInCup() const { std::cout << "  Pouring into cup\n"; }
};

// --- Concrete Class: Tea ---
class Tea final : public Beverage {
protected:
    void brew() const override { std::cout << "  Steeping the tea\n"; }
    void addCondiments() const override { std::cout << "  Adding lemon\n"; }
    std::string name() const override { return "Tea"; }
};

// --- Concrete Class: Coffee ---
// Overrides the hook so its condiment step can be skipped on demand.
class Coffee final : public Beverage {
public:
    explicit Coffee(bool wantsCondiments) : wantsCondiments_(wantsCondiments) {}

protected:
    void brew() const override { std::cout << "  Dripping coffee through filter\n"; }
    void addCondiments() const override {
        std::cout << "  Adding sugar and milk\n";
    }
    std::string name() const override { return "Coffee"; }
    bool customerWantsCondiments() const override { return wantsCondiments_; }

private:
    bool wantsCondiments_;
};

// --- Demonstration ---
int main() {
    std::cout << "Making tea:\n";
    Tea tea;
    tea.prepare();

    std::cout << "\nMaking coffee (with condiments):\n";
    Coffee coffee(true);
    coffee.prepare();

    std::cout << "\nMaking black coffee (hook opts out of condiments):\n";
    Coffee blackCoffee(false);
    blackCoffee.prepare();

    // Same skeleton runs for every beverage; only the deferred steps change.
    return 0;
}
