/*
 * Visitor (Behavioral)
 *
 * Intent: Represent an operation to be performed on the elements of an object
 * structure. Visitor lets you define a new operation without changing the
 * classes of the elements on which it operates.
 *
 * Problem / When to use it:
 *  - You need many unrelated operations over a stable set of element types.
 *  - You want to add operations without editing the element classes.
 *  - Related behavior should be gathered in one class rather than scattered.
 *
 * Real-world analogy: A tax auditor visits different businesses (restaurant,
 * factory) and applies the right rules to each without the businesses knowing
 * the audit logic. A different auditor can visit the same businesses.
 *
 * Participants:
 *  - Visitor (ShapeVisitor): declares a visit() overload per ConcreteElement.
 *  - ConcreteVisitor (AreaVisitor, XmlExportVisitor): one new operation each.
 *  - Element (Shape): declares accept(Visitor&).
 *  - ConcreteElement (Circle, Rectangle): implement accept() via double
 *    dispatch, calling back the matching visit() overload.
 *
 * Trade-offs:
 *  Pros:
 *   - Add new operations easily (just write a new visitor).
 *   - Groups a single operation's logic in one cohesive place.
 *  Cons:
 *   - Adding a new element type forces every visitor to change.
 *   - Visitors may need access to element internals, weakening encapsulation.
 */

#include <iostream>
#include <memory>
#include <vector>

// Own PI constant: M_PI is not standard C++ and is hidden under -std=c++17.
constexpr double kPi = 3.14159265358979323846;

// Forward declarations of the concrete elements the visitor knows about.
class Circle;
class Rectangle;

// --- Visitor interface ---
// One overload per concrete element type. Adding an element type here is the
// cost of the pattern; adding an operation is cheap (a new subclass).
class ShapeVisitor {
public:
    virtual ~ShapeVisitor() = default;
    virtual void visit(const Circle& circle) = 0;
    virtual void visit(const Rectangle& rectangle) = 0;
};

// --- Element interface ---
class Shape {
public:
    virtual ~Shape() = default;
    // First dispatch: virtual on the element's dynamic type.
    virtual void accept(ShapeVisitor& visitor) const = 0;
};

// --- Concrete Element: Circle ---
class Circle final : public Shape {
public:
    explicit Circle(double radius) : radius_(radius) {}
    double radius() const { return radius_; }

    // Second dispatch: the static type of *this (Circle) selects the overload.
    void accept(ShapeVisitor& visitor) const override { visitor.visit(*this); }

private:
    double radius_;
};

// --- Concrete Element: Rectangle ---
class Rectangle final : public Shape {
public:
    Rectangle(double width, double height) : width_(width), height_(height) {}
    double width() const { return width_; }
    double height() const { return height_; }

    void accept(ShapeVisitor& visitor) const override { visitor.visit(*this); }

private:
    double width_;
    double height_;
};

// --- Concrete Visitor: compute area ---
// Accumulates the total area across every shape it visits.
class AreaVisitor final : public ShapeVisitor {
public:
    void visit(const Circle& circle) override {
        const double area = kPi * circle.radius() * circle.radius();
        std::cout << "Circle area = " << area << '\n';
        total_ += area;
    }
    void visit(const Rectangle& rectangle) override {
        const double area = rectangle.width() * rectangle.height();
        std::cout << "Rectangle area = " << area << '\n';
        total_ += area;
    }
    double total() const { return total_; }

private:
    double total_ = 0.0;
};

// --- Concrete Visitor: export to XML ---
// A completely different operation over the same shapes, added without
// touching Circle or Rectangle.
class XmlExportVisitor final : public ShapeVisitor {
public:
    void visit(const Circle& circle) override {
        std::cout << "<circle radius=\"" << circle.radius() << "\"/>\n";
    }
    void visit(const Rectangle& rectangle) override {
        std::cout << "<rectangle width=\"" << rectangle.width()
                  << "\" height=\"" << rectangle.height() << "\"/>\n";
    }
};

// --- Demonstration ---
int main() {
    // A heterogeneous object structure held via the base interface.
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(2.0));
    shapes.push_back(std::make_unique<Rectangle>(3.0, 4.0));
    shapes.push_back(std::make_unique<Circle>(1.5));

    std::cout << "-- AreaVisitor --\n";
    AreaVisitor areas;
    for (const auto& shape : shapes)
        shape->accept(areas); // Double dispatch picks the right visit().
    std::cout << "Total area = " << areas.total() << "\n\n";

    std::cout << "-- XmlExportVisitor --\n";
    XmlExportVisitor xml;
    for (const auto& shape : shapes)
        shape->accept(xml);

    return 0;
}
