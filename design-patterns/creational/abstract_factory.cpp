/*
 * Pattern: Abstract Factory
 * Category: Creational
 *
 * Intent:
 *   Provide an interface for creating families of related or dependent objects
 *   without specifying their concrete classes.
 *
 * Problem / When to use it:
 *   - Your system must work with multiple families of related products (e.g.
 *     a full set of matching Windows widgets vs. macOS widgets).
 *   - You need to guarantee that products from one family are used together
 *     and never mixed (a Win button next to a Mac checkbox).
 *   - You want client code to depend only on abstract interfaces, so swapping
 *     the whole family is a single configuration change.
 *
 * Real-world analogy:
 *   A furniture company offers matching sets: choosing the "Victorian" line
 *   gives you a Victorian chair AND sofa; the "Modern" line gives you modern
 *   versions of both. You pick the factory once; the set stays consistent.
 *
 * Participants:
 *   - AbstractProduct (Button,       interfaces for a kind of product.
 *     Checkbox)
 *   - ConcreteProduct (WinButton,    family-specific implementations.
 *     MacButton, WinCheckbox, ...)
 *   - AbstractFactory (GUIFactory):  interface with a creation method per
 *                                    product kind.
 *   - ConcreteFactory (WinFactory,   produces a matching family of products.
 *     MacFactory)
 *   - Client (Application):          uses only AbstractFactory / AbstractProduct.
 *
 * Trade-offs:
 *   Pros:
 *     - Guarantees compatibility among products of one family.
 *     - Isolates client code from concrete classes.
 *     - Swapping product families is a one-line change.
 *   Cons:
 *     - Adding a NEW product kind means changing the factory interface and
 *       every concrete factory.
 *     - More classes and indirection than a direct construction.
 */

#include <iostream>
#include <memory>
#include <string>

// --- Abstract Product A: Button ---
class Button {
public:
    virtual ~Button() = default;
    virtual std::string render() const = 0;
    virtual std::string onClick() const = 0;
};

// --- Abstract Product B: Checkbox ---
class Checkbox {
public:
    virtual ~Checkbox() = default;
    virtual std::string render() const = 0;
    virtual std::string toggle() const = 0;
};

// --- Concrete Products: Windows family ---
class WinButton final : public Button {
public:
    std::string render() const override { return "[ Win Button ]"; }
    std::string onClick() const override { return "Windows click: sharp beep."; }
};

class WinCheckbox final : public Checkbox {
public:
    std::string render() const override { return "[x] Win Checkbox"; }
    std::string toggle() const override { return "Windows checkbox toggled."; }
};

// --- Concrete Products: macOS family ---
class MacButton final : public Button {
public:
    std::string render() const override { return "( Mac Button )"; }
    std::string onClick() const override { return "macOS click: soft tick."; }
};

class MacCheckbox final : public Checkbox {
public:
    std::string render() const override { return "(o) Mac Checkbox"; }
    std::string toggle() const override { return "macOS checkbox toggled."; }
};

// --- Abstract Factory ---
// One creation method per product kind. Every concrete factory returns a
// self-consistent family, so widgets are guaranteed to match.
class GUIFactory {
public:
    virtual ~GUIFactory() = default;
    virtual std::unique_ptr<Button> createButton() const = 0;
    virtual std::unique_ptr<Checkbox> createCheckbox() const = 0;
};

// --- Concrete Factory: Windows ---
class WinFactory final : public GUIFactory {
public:
    std::unique_ptr<Button> createButton() const override {
        return std::make_unique<WinButton>();
    }
    std::unique_ptr<Checkbox> createCheckbox() const override {
        return std::make_unique<WinCheckbox>();
    }
};

// --- Concrete Factory: macOS ---
class MacFactory final : public GUIFactory {
public:
    std::unique_ptr<Button> createButton() const override {
        return std::make_unique<MacButton>();
    }
    std::unique_ptr<Checkbox> createCheckbox() const override {
        return std::make_unique<MacCheckbox>();
    }
};

// --- Client ---
// Application depends ONLY on the abstract interfaces. It never mentions a
// concrete widget, so the same code renders a coherent UI for any platform.
class Application {
public:
    explicit Application(const GUIFactory& factory)
        : button_(factory.createButton()),
          checkbox_(factory.createCheckbox()) {}

    void renderUI() const {
        std::cout << "  " << button_->render()
                  << "  ->  " << button_->onClick() << '\n';
        std::cout << "  " << checkbox_->render()
                  << "  ->  " << checkbox_->toggle() << '\n';
    }

private:
    std::unique_ptr<Button> button_;
    std::unique_ptr<Checkbox> checkbox_;
};

// Chooses a factory at configuration time based on the running platform.
std::unique_ptr<GUIFactory> makeFactory(const std::string& os) {
    if (os == "windows") return std::make_unique<WinFactory>();
    return std::make_unique<MacFactory>();
}

int main() {
    std::cout << "--- Abstract Factory: Cross-platform GUI ---\n";

    for (const std::string os : {"windows", "macos"}) {
        std::cout << "\nPlatform: " << os << '\n';
        std::unique_ptr<GUIFactory> factory = makeFactory(os);
        Application app(*factory);   // client wired to one consistent family
        app.renderUI();
    }

    return 0;
}
