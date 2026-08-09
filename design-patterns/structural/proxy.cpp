/*
 * Pattern: Proxy (Structural)
 *
 * Intent: Provide a surrogate or placeholder for another object to control
 * access to it. The proxy implements the same interface as the real subject,
 * so clients cannot tell the difference.
 *
 * Problem / When to use it:
 *   - Virtual proxy: defer the cost of creating an expensive object until it
 *     is actually needed (lazy initialization), as shown below.
 *   - Protection proxy: check permissions before forwarding a request.
 *   - Remote proxy: represent an object living in a different address space.
 *   - Caching / smart-reference proxy: memoize results or manage lifetime.
 *
 * Real-world analogy: A thumbnail in a photo gallery stands in for a huge
 * full-resolution image. The heavyweight file is only read from disk the first
 * time you actually open the picture.
 *
 * Participants:
 *   - Subject (Image): common interface for RealSubject and Proxy.
 *   - RealSubject (RealImage): the real, expensive-to-create object.
 *   - Proxy (ProxyImage): holds a reference to the RealSubject, controls its
 *     creation/access, and forwards requests once appropriate.
 *
 * Trade-offs:
 *   Pros:
 *     - Adds control (lazy load, access checks, caching) without touching the
 *       real subject or the client, which both see only the Subject interface.
 *   Cons:
 *     - Extra indirection can add latency and code complexity.
 *     - A lazy proxy may surprise callers by doing heavy work on first use.
 */

#include <iostream>
#include <memory>
#include <string>

// --- Subject: interface shared by the real object and its proxy ---
class Image {
public:
    virtual ~Image() = default;         // polymorphic base needs virtual dtor
    virtual void display() const = 0;
};

// --- RealSubject: costly to construct (simulated disk load) ---
class RealImage final : public Image {
public:
    explicit RealImage(std::string filename)
        : filename_(std::move(filename)) {
        loadFromDisk();  // the expensive work happens up front
    }

    void display() const override {
        std::cout << "RealImage: rendering \"" << filename_ << "\"\n";
    }

private:
    void loadFromDisk() const {
        std::cout << "RealImage: loading \"" << filename_
                  << "\" from disk (expensive)...\n";
    }

    std::string filename_;
};

// --- Proxy: same interface, but defers creating the RealImage ---
// This is a VIRTUAL proxy (lazy initialization). Variants share this shape:
//   * Protection proxy: check credentials in display() before forwarding.
//   * Caching proxy: keep 'real_' alive to reuse a computed/loaded result.
//   * Remote proxy: 'real_' would marshal calls across a network boundary.
class ProxyImage final : public Image {
public:
    explicit ProxyImage(std::string filename)
        : filename_(std::move(filename)) {}  // cheap: no disk access yet

    void display() const override {
        // Create the heavyweight object only on first real use.
        if (!real_) {
            std::cout << "ProxyImage: first request, instantiating RealImage\n";
            real_ = std::make_unique<RealImage>(filename_);
        } else {
            std::cout << "ProxyImage: reusing already-loaded RealImage\n";
        }
        real_->display();  // forward to the real subject
    }

private:
    std::string filename_;
    // 'mutable' lets a const display() perform lazy initialization.
    mutable std::unique_ptr<RealImage> real_;
};

int main() {
    // Constructing the proxy is cheap: nothing is loaded from disk here.
    std::cout << "-- Creating proxies (no load expected) --\n";
    std::unique_ptr<Image> photo = std::make_unique<ProxyImage>("vacation.png");
    std::unique_ptr<Image> unused = std::make_unique<ProxyImage>("never.png");

    std::cout << "\n-- First display(): triggers the expensive load --\n";
    photo->display();

    std::cout << "\n-- Second display(): real object is cached --\n";
    photo->display();

    // 'unused' was never displayed, so its RealImage is never created,
    // demonstrating the memory/time savings of the virtual proxy.
    std::cout << "\n\"never.png\" was never loaded because it was never shown.\n";
    return 0;
}
