/*
 * Pattern: Prototype (Creational)
 *
 * Intent: Create new objects by cloning an existing instance (the prototype)
 *         instead of instantiating a concrete class, copying its internal
 *         state.
 *
 * Problem / When to use it:
 *   - Object creation is expensive or its concrete type is decided at runtime.
 *   - You want to produce copies without coupling to their concrete classes.
 *   - You need many objects that differ only in pre-configured state.
 *
 * Real-world analogy: A cell dividing by mitosis -- the copy carries the same
 *   internal contents as the original rather than being built from scratch.
 *
 * Participants:
 *   - Prototype        : Shape (declares clone())
 *   - ConcretePrototype: Circle, Rectangle (implement clone(), copy state)
 *   - Registry         : PrototypeRegistry (stores prototypes, vends copies)
 *   - Client           : main() (asks the registry / prototypes for copies)
 *
 * Trade-offs:
 *   Pros:
 *     - Clone objects without depending on their concrete classes.
 *     - Cheaper than re-running costly construction/configuration.
 *     - Registry lets you add/remove product variants at runtime.
 *   Cons:
 *     - Deep-copying objects with cyclic references or shared resources
 *       is tricky and easy to get wrong (shallow vs deep copy bugs).
 *     - Every concrete prototype must implement clone() correctly.
 */

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// --- Prototype interface ---
// Any shape can copy itself; virtual destructor makes deletion via
// base pointer safe (polymorphic base class).
class Shape {
public:
    virtual ~Shape() = default;
    virtual std::unique_ptr<Shape> clone() const = 0;
    virtual void draw() const = 0;
};

// --- Concrete Prototype: Circle ---
class Circle final : public Shape {
public:
    Circle(std::string color, int radius)
        : color_(std::move(color)), radius_(radius) {}

    // Copy this instance's state into a brand-new object.
    std::unique_ptr<Shape> clone() const override {
        return std::make_unique<Circle>(*this);
    }

    void draw() const override {
        std::cout << "Circle(color=" << color_
                  << ", radius=" << radius_ << ")\n";
    }

    void setColor(const std::string& c) { color_ = c; }

private:
    std::string color_;
    int radius_;
};

// --- Concrete Prototype: Rectangle (holds an owned/deep member) ---
// The 'tags' vector demonstrates DEEP copy: the copied Rectangle owns its
// own vector, so mutating a clone must not affect the original.
class Rectangle final : public Shape {
public:
    Rectangle(std::string color, int w, int h)
        : color_(std::move(color)), width_(w), height_(h) {}

    std::unique_ptr<Shape> clone() const override {
        // The default copy constructor deep-copies std::string and
        // std::vector members, which is exactly what we want here.
        return std::make_unique<Rectangle>(*this);
    }

    void draw() const override {
        std::cout << "Rectangle(color=" << color_
                  << ", " << width_ << "x" << height_ << ", tags=[";
        bool first = true;
        for (const auto& t : tags_) {
            std::cout << (first ? "" : ",") << t;
            first = false;
        }
        std::cout << "])\n";
    }

    void addTag(const std::string& t) { tags_.push_back(t); }
    void setColor(const std::string& c) { color_ = c; }

private:
    std::string color_;
    int width_;
    int height_;
    std::vector<std::string> tags_;
};

// --- Registry ---
// Stores configured prototypes under string keys and hands back independent
// copies. Clients never mention concrete classes.
class PrototypeRegistry {
public:
    void registerPrototype(const std::string& key, std::unique_ptr<Shape> proto) {
        prototypes_[key] = std::move(proto);
    }

    std::unique_ptr<Shape> create(const std::string& key) const {
        auto it = prototypes_.find(key);
        return it == prototypes_.end() ? nullptr : it->second->clone();
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Shape>> prototypes_;
};

int main() {
    PrototypeRegistry registry;

    // Pre-configure prototypes once; later we just clone them.
    auto redCircle = std::make_unique<Circle>("red", 10);
    auto blueRect = std::make_unique<Rectangle>("blue", 4, 3);
    blueRect->addTag("wall");
    blueRect->addTag("brick");

    registry.registerPrototype("red-circle", std::move(redCircle));
    registry.registerPrototype("blue-rect", std::move(blueRect));

    // Vend copies through the base interface.
    std::cout << "Cloned from registry:\n";
    std::unique_ptr<Shape> c1 = registry.create("red-circle");
    std::unique_ptr<Shape> r1 = registry.create("blue-rect");
    c1->draw();
    r1->draw();

    // Deep-copy correctness: mutating a clone must not touch the original.
    std::cout << "\nDeep-copy check (mutate the clone only):\n";
    std::unique_ptr<Shape> r2 = registry.create("blue-rect");
    if (auto* rect = dynamic_cast<Rectangle*>(r2.get())) {
        rect->setColor("green");
        rect->addTag("MUTATED");
    }
    std::cout << "  clone   : ";
    r2->draw();
    std::cout << "  original: ";
    registry.create("blue-rect")->draw(); // still blue, no MUTATED tag

    // Cloning a live object directly (not via the registry) also works.
    std::cout << "\nDirect clone of a live object:\n";
    Circle base("yellow", 7);
    std::unique_ptr<Shape> copy = base.clone();
    copy->draw();

    return 0;
}
