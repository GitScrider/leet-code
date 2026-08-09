/*
 * Pattern: Decorator (Structural)
 *
 * Intent: Attach additional responsibilities to an object dynamically.
 * Decorators provide a flexible alternative to subclassing for extending
 * behavior at run time.
 *
 * Problem / When to use it:
 *   - You want to add responsibilities to individual objects, not whole classes.
 *   - The extensions can be combined in many orders/quantities, so a static
 *     class-per-combination explosion is undesirable.
 *   - You want to add or remove behavior transparently, without the client
 *     knowing whether it holds a plain object or a decorated one.
 *
 * Real-world analogy: Ordering coffee. You start with a base beverage and keep
 * wrapping it with add-ons (milk, sugar, whipped cream). Each add-on adjusts
 * both the description and the price, and you can stack them freely.
 *
 * Participants:
 *   - Component (Beverage): common interface for objects that can be decorated.
 *   - ConcreteComponent (Espresso, HouseBlend): the base objects being wrapped.
 *   - Decorator (CondimentDecorator): holds a Component and conforms to the
 *     same interface, forwarding requests to the wrapped object.
 *   - ConcreteDecorator (Milk, Sugar, WhipCream): augment cost()/description().
 *
 * Trade-offs:
 *   Pros:
 *     - Add/compose behavior at run time without touching existing classes.
 *     - Avoids a combinatorial explosion of subclasses.
 *     - Each decorator has a single, focused responsibility.
 *   Cons:
 *     - Many small wrapper objects can make a system harder to learn/debug.
 *     - A decorated object is not identical to its component (identity checks
 *       and downcasts become awkward).
 *     - Order of wrapping can matter and is not always obvious.
 */

#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <utility>

// --- Component: the interface shared by base beverages and decorators ---
class Beverage {
public:
    virtual ~Beverage() = default;
    virtual std::string description() const = 0;
    virtual double cost() const = 0; // in currency units
};

// --- ConcreteComponent: a plain espresso, no add-ons ---
class Espresso final : public Beverage {
public:
    std::string description() const override { return "Espresso"; }
    double cost() const override { return 1.90; }
};

// --- ConcreteComponent: a house blend coffee ---
class HouseBlend final : public Beverage {
public:
    std::string description() const override { return "House Blend"; }
    double cost() const override { return 1.20; }
};

// --- Decorator: base class that wraps and forwards to a Beverage ---
// It IS-A Beverage (so it can stand in for one) and HAS-A Beverage (the one
// it decorates), which is exactly what lets decorators stack recursively.
class CondimentDecorator : public Beverage {
public:
    explicit CondimentDecorator(std::unique_ptr<Beverage> inner)
        : beverage_(std::move(inner)) {}

protected:
    std::unique_ptr<Beverage> beverage_; // the wrapped component (owned)
};

// --- ConcreteDecorator: adds milk ---
class Milk final : public CondimentDecorator {
public:
    using CondimentDecorator::CondimentDecorator;

    std::string description() const override {
        return beverage_->description() + " + Milk";
    }
    double cost() const override {
        return beverage_->cost() + 0.40; // augment the wrapped cost
    }
};

// --- ConcreteDecorator: adds sugar ---
class Sugar final : public CondimentDecorator {
public:
    using CondimentDecorator::CondimentDecorator;

    std::string description() const override {
        return beverage_->description() + " + Sugar";
    }
    double cost() const override {
        return beverage_->cost() + 0.15;
    }
};

// --- ConcreteDecorator: adds whipped cream ---
class WhipCream final : public CondimentDecorator {
public:
    using CondimentDecorator::CondimentDecorator;

    std::string description() const override {
        return beverage_->description() + " + Whip Cream";
    }
    double cost() const override {
        return beverage_->cost() + 0.60;
    }
};

// Small client-side helper to present an order uniformly.
static void printOrder(const Beverage& b) {
    std::cout << std::left << std::setw(40) << b.description()
              << " $" << std::fixed << std::setprecision(2) << b.cost() << '\n';
}

// --- Client: builds beverages by wrapping, then treats them as Beverage ---
int main() {
    std::cout << std::fixed << std::setprecision(2);

    // A bare espresso.
    std::unique_ptr<Beverage> order1 = std::make_unique<Espresso>();
    printOrder(*order1);

    // House blend + milk + sugar, built by stacking decorators.
    // Each wrap returns something that is still a Beverage, so we keep going.
    std::unique_ptr<Beverage> order2 = std::make_unique<HouseBlend>();
    order2 = std::make_unique<Milk>(std::move(order2));
    order2 = std::make_unique<Sugar>(std::move(order2));
    printOrder(*order2);

    // Espresso + double sugar + whip cream, composed in one expression.
    std::unique_ptr<Beverage> order3 =
        std::make_unique<WhipCream>(
            std::make_unique<Sugar>(
                std::make_unique<Sugar>(
                    std::make_unique<Espresso>())));
    printOrder(*order3);

    return 0;
}
