/*
 * Pattern: Factory Method
 * Category: Creational
 *
 * Intent:
 *   Define an interface for creating an object, but let subclasses decide
 *   which concrete class to instantiate. Factory Method defers instantiation
 *   to subclasses.
 *
 * Problem / When to use it:
 *   - A class cannot anticipate the concrete type of objects it must create.
 *   - You want to localize the "which class to instantiate" decision in one
 *     overridable method instead of scattering `new` calls through client code.
 *   - You want to add new product variants without touching the code that
 *     uses them (Open/Closed Principle).
 *
 * Real-world analogy:
 *   A logistics company plans deliveries the same way regardless of vehicle,
 *   but a road branch dispatches trucks while a sea branch dispatches ships.
 *   The planning workflow is shared; only the vehicle-creation step differs.
 *
 * Participants:
 *   - Product (Transport):        interface of objects the factory creates.
 *   - ConcreteProduct (Truck,     concrete implementations of Product.
 *     Ship)
 *   - Creator (Logistics):        declares the factory method createTransport()
 *                                 and contains shared logic that uses Product.
 *   - ConcreteCreator            override the factory method to return a
 *     (RoadLogistics,             specific ConcreteProduct.
 *     SeaLogistics)
 *
 * Trade-offs:
 *   Pros:
 *     - Decouples client code from concrete product classes.
 *     - Single place per subclass controls object creation.
 *     - New products require only a new Creator subclass (Open/Closed).
 *   Cons:
 *     - Can introduce many small subclasses just to vary creation.
 *     - The parallel Creator/Product hierarchies add structural overhead.
 */

#include <iostream>
#include <memory>
#include <string>

// --- Product interface ---
// Declares the operation all concrete transports must support. A virtual
// destructor is essential because we own products polymorphically.
class Transport {
public:
    virtual ~Transport() = default;
    virtual std::string deliver(const std::string& cargo) const = 0;
};

// --- Concrete Product: Truck ---
class Truck final : public Transport {
public:
    std::string deliver(const std::string& cargo) const override {
        return "Truck delivering '" + cargo + "' by road in a box.";
    }
};

// --- Concrete Product: Ship ---
class Ship final : public Transport {
public:
    std::string deliver(const std::string& cargo) const override {
        return "Ship delivering '" + cargo + "' by sea in a container.";
    }
};

// --- Creator ---
// Contains the business logic that operates on Product objects. It does NOT
// know which concrete Transport it works with; that decision is deferred to
// subclasses via the factory method createTransport().
class Logistics {
public:
    virtual ~Logistics() = default;

    // The factory method. Subclasses override it to choose the product.
    // Returning unique_ptr makes ownership explicit and leak-free.
    virtual std::unique_ptr<Transport> createTransport() const = 0;

    // Template-method-style workflow shared by all creators: the creator
    // relies only on the abstract Transport interface, never on Truck/Ship.
    std::string planDelivery(const std::string& cargo) const {
        std::unique_ptr<Transport> transport = createTransport();
        return "[Plan] " + transport->deliver(cargo);
    }
};

// --- Concrete Creator: RoadLogistics ---
// Decides that road delivery means Trucks.
class RoadLogistics final : public Logistics {
public:
    std::unique_ptr<Transport> createTransport() const override {
        return std::make_unique<Truck>();
    }
};

// --- Concrete Creator: SeaLogistics ---
// Decides that sea delivery means Ships.
class SeaLogistics final : public Logistics {
public:
    std::unique_ptr<Transport> createTransport() const override {
        return std::make_unique<Ship>();
    }
};

// --- Client code ---
// Works purely against the Logistics abstraction, so it stays unchanged when
// new transport types (e.g. AirLogistics/Plane) are added later.
void runDelivery(const Logistics& logistics, const std::string& cargo) {
    std::cout << logistics.planDelivery(cargo) << '\n';
}

int main() {
    std::cout << "--- Factory Method: Logistics ---\n";

    // Choose the concrete creator at runtime; client logic is identical.
    std::unique_ptr<Logistics> road = std::make_unique<RoadLogistics>();
    std::unique_ptr<Logistics> sea  = std::make_unique<SeaLogistics>();

    runDelivery(*road, "Furniture");
    runDelivery(*sea, "Bananas");

    // The same abstract reference can point to any creator.
    std::cout << "\n--- Selecting a creator dynamically ---\n";
    for (const auto& mode : {std::string("road"), std::string("sea")}) {
        std::unique_ptr<Logistics> logistics =
            (mode == "sea") ? std::unique_ptr<Logistics>(std::make_unique<SeaLogistics>())
                            : std::unique_ptr<Logistics>(std::make_unique<RoadLogistics>());
        std::cout << mode << ": " << logistics->planDelivery("Parcel") << '\n';
    }

    return 0;
}
