/*
 * Pattern: Bridge (Structural)
 *
 * Intent:
 *   Decouple an abstraction from its implementation so that the two can vary
 *   independently, replacing a combinatorial class explosion with two
 *   separate, composable hierarchies linked by a reference ("the bridge").
 *
 * Problem / When to use it:
 *   - You have two orthogonal dimensions that both need to grow (e.g. kinds of
 *     remote control X kinds of device) and don't want a subclass per pair.
 *   - You want to switch an implementation at runtime, not just at compile time.
 *   - You want to hide implementation details from clients of the abstraction.
 *
 * Real-world analogy:
 *   A remote control (abstraction) works any TV or radio (implementation).
 *   Buying a fancier remote doesn't require a new kind of TV, and vice versa;
 *   the remote and the device evolve on their own.
 *
 * Participants:
 *   - Abstraction (RemoteControl): high-level control logic, holds an Implementor.
 *   - RefinedAbstraction (AdvancedRemote): extends the abstraction's interface.
 *   - Implementor (Device): interface for the low-level implementation.
 *   - ConcreteImplementor (TV, Radio): concrete device implementations.
 *
 * Trade-offs:
 *   Pros:
 *     - Abstraction and implementation vary independently (no NxM subclasses).
 *     - Implementation can be swapped at runtime.
 *     - Follows Open/Closed: add remotes or devices without touching the other.
 *   Cons:
 *     - Extra indirection and more moving parts than a single hierarchy.
 *     - Can be overkill when only one implementation will ever exist.
 */

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

// --- Implementor: the low-level interface the abstraction delegates to ---
class Device {
public:
    virtual ~Device() = default;
    virtual void setEnabled(bool enabled) = 0;
    virtual bool isEnabled() const = 0;
    virtual void setVolume(int percent) = 0;
    virtual int getVolume() const = 0;
    virtual std::string name() const = 0;
};

// --- ConcreteImplementor: a television ---
class TV final : public Device {
public:
    void setEnabled(bool enabled) override { on_ = enabled; }
    bool isEnabled() const override { return on_; }
    void setVolume(int percent) override {
        volume_ = std::clamp(percent, 0, 100);
    }
    int getVolume() const override { return volume_; }
    std::string name() const override { return "TV"; }

private:
    bool on_ = false;
    int volume_ = 30;
};

// --- ConcreteImplementor: a radio ---
class Radio final : public Device {
public:
    void setEnabled(bool enabled) override { on_ = enabled; }
    bool isEnabled() const override { return on_; }
    void setVolume(int percent) override {
        // Radios in this example cap out earlier, showing implementations differ.
        volume_ = std::clamp(percent, 0, 80);
    }
    int getVolume() const override { return volume_; }
    std::string name() const override { return "Radio"; }

private:
    bool on_ = false;
    int volume_ = 20;
};

// --- Abstraction: control logic expressed in terms of the Implementor ---
// The "bridge" is the Device pointer: the remote talks to *some* device without
// knowing which concrete type it is.
class RemoteControl {
public:
    explicit RemoteControl(std::shared_ptr<Device> device)
        : device_(std::move(device)) {}

    // Virtual so refined abstractions can extend, but the base is usable alone.
    virtual ~RemoteControl() = default;

    void togglePower() {
        device_->setEnabled(!device_->isEnabled());
        std::cout << device_->name() << " power -> "
                  << (device_->isEnabled() ? "ON" : "OFF") << "\n";
    }

    void volumeUp() {
        device_->setVolume(device_->getVolume() + 10);
        report("volume up");
    }

    void volumeDown() {
        device_->setVolume(device_->getVolume() - 10);
        report("volume down");
    }

protected:
    // Refined abstractions reuse the same bridged device.
    void report(const std::string& action) const {
        std::cout << device_->name() << " " << action
                  << " -> volume=" << device_->getVolume() << "\n";
    }

    std::shared_ptr<Device> device_;
};

// --- RefinedAbstraction: adds behavior without touching the Device hierarchy ---
class AdvancedRemote final : public RemoteControl {
public:
    using RemoteControl::RemoteControl;

    // Extra feature that only the advanced remote offers.
    void mute() {
        device_->setVolume(0);
        report("muted");
    }
};

int main() {
    // Same remote type drives different devices: abstraction independent of impl.
    auto tv = std::make_shared<TV>();
    RemoteControl basicRemote(tv);
    std::cout << "-- Basic remote on a TV --\n";
    basicRemote.togglePower();
    basicRemote.volumeUp();
    basicRemote.volumeDown();

    // Same device type driven by a richer remote: impl independent of abstraction.
    auto radio = std::make_shared<Radio>();
    AdvancedRemote advancedRemote(radio);
    std::cout << "\n-- Advanced remote on a Radio --\n";
    advancedRemote.togglePower();
    advancedRemote.volumeUp();
    advancedRemote.volumeUp(); // Radio caps at 80, showing implementation-specific rules
    advancedRemote.mute();

    return 0;
}
