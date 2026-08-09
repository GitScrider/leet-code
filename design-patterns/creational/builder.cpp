/*
 * Pattern: Builder (Creational)
 *
 * Intent: Separate the construction of a complex object from its
 *         representation, so the same construction process can create
 *         different representations.
 *
 * Problem / When to use it:
 *   - An object needs many optional/step-wise parts and a huge constructor
 *     (the "telescoping constructor" smell) would be unwieldy.
 *   - The assembly sequence should be reusable while the produced parts vary.
 *   - You want to build immutable products piece by piece.
 *
 * Real-world analogy: Ordering a custom PC. The build recipe ("gaming rig")
 *   is fixed, but the shop can assemble it from different concrete parts.
 *
 * Participants:
 *   - Product        : Computer (the complex object being built)
 *   - Builder        : ComputerBuilder (abstract step interface)
 *   - ConcreteBuilder: GamingPcBuilder, OfficePcBuilder (assemble parts)
 *   - Director       : ComputerShop (encapsulates a construction recipe)
 *
 * Trade-offs:
 *   Pros:
 *     - Isolates construction code from representation.
 *     - Same Director recipe yields different products via different builders.
 *     - Allows step-by-step construction of complex/immutable objects.
 *   Cons:
 *     - More moving parts (extra classes) for simple objects.
 *     - Builder must stay in sync with the Product's structure.
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// --- Product ---
// The complex object we assemble. It is unaware of how it was built.
class Computer {
public:
    void setCpu(const std::string& cpu) { cpu_ = cpu; }
    void setGpu(const std::string& gpu) { gpu_ = gpu; }
    void setRam(int gigabytes) { ramGb_ = gigabytes; }
    void addPeripheral(const std::string& p) { peripherals_.push_back(p); }

    void describe() const {
        std::cout << "Computer [CPU=" << cpu_
                  << ", GPU=" << gpu_
                  << ", RAM=" << ramGb_ << "GB]\n";
        std::cout << "  Peripherals:";
        if (peripherals_.empty()) {
            std::cout << " (none)";
        } else {
            for (const auto& p : peripherals_) std::cout << ' ' << p;
        }
        std::cout << "\n";
    }

private:
    std::string cpu_{"unset"};
    std::string gpu_{"integrated"};
    int ramGb_{0};
    std::vector<std::string> peripherals_;
};

// --- Builder (abstract step interface) ---
// Declares the construction steps common to all concrete builders.
class ComputerBuilder {
public:
    virtual ~ComputerBuilder() = default;

    // Start a fresh product so a builder instance can be reused.
    void reset() { product_ = std::make_unique<Computer>(); }

    virtual void buildCpu() = 0;
    virtual void buildGpu() = 0;
    virtual void buildRam() = 0;
    virtual void buildPeripherals() = 0;

    // Hand ownership of the finished product to the caller.
    std::unique_ptr<Computer> getResult() { return std::move(product_); }

protected:
    std::unique_ptr<Computer> product_{std::make_unique<Computer>()};
};

// --- Concrete Builder: high-end gaming machine ---
class GamingPcBuilder final : public ComputerBuilder {
public:
    void buildCpu() override { product_->setCpu("Ryzen 9 7950X"); }
    void buildGpu() override { product_->setGpu("GeForce RTX 4090"); }
    void buildRam() override { product_->setRam(64); }
    void buildPeripherals() override {
        product_->addPeripheral("Mechanical-Keyboard");
        product_->addPeripheral("144Hz-Monitor");
    }
};

// --- Concrete Builder: modest office machine ---
class OfficePcBuilder final : public ComputerBuilder {
public:
    void buildCpu() override { product_->setCpu("Core i3-13100"); }
    void buildGpu() override { product_->setGpu("integrated"); }
    void buildRam() override { product_->setRam(8); }
    void buildPeripherals() override {
        product_->addPeripheral("Office-Keyboard");
    }
};

// --- Director ---
// Encapsulates the *order* of construction (the recipe). It does not know
// which concrete parts result -- that depends on the injected builder.
class ComputerShop {
public:
    // A full build using every step.
    std::unique_ptr<Computer> assembleFull(ComputerBuilder& b) const {
        b.reset();
        b.buildCpu();
        b.buildGpu();
        b.buildRam();
        b.buildPeripherals();
        return b.getResult();
    }

    // A minimal build reusing only some steps -- same builder, fewer steps.
    std::unique_ptr<Computer> assembleBarebones(ComputerBuilder& b) const {
        b.reset();
        b.buildCpu();
        b.buildRam();
        return b.getResult();
    }
};

int main() {
    ComputerShop shop;

    // Same Director recipe + different builders -> different representations.
    GamingPcBuilder gaming;
    OfficePcBuilder office;

    std::unique_ptr<Computer> gamingPc = shop.assembleFull(gaming);
    std::unique_ptr<Computer> officePc = shop.assembleFull(office);

    std::cout << "Full gaming build:\n";
    gamingPc->describe();
    std::cout << "\nFull office build:\n";
    officePc->describe();

    // Same builder, a different recipe -> a barebones representation.
    std::unique_ptr<Computer> barebones = shop.assembleBarebones(gaming);
    std::cout << "\nBarebones build (gaming parts, fewer steps):\n";
    barebones->describe();

    return 0;
}
