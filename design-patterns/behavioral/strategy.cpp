/*
 * Strategy (Behavioral)
 *
 * Intent: Define a family of interchangeable algorithms, encapsulate each one,
 * and make them selectable at run time without changing the client that uses
 * them.
 *
 * Problem / When to use it:
 *  - You need different variants of an algorithm and want to swap them freely.
 *  - A class has many conditional branches choosing between related behaviors.
 *  - You want to isolate algorithm details from the code that invokes them.
 *
 * Real-world analogy: At checkout you pick how to pay - credit card or PayPal.
 * The checkout process is identical; only the payment algorithm differs.
 *
 * Participants:
 *  - Strategy (PaymentStrategy): common interface for all algorithms.
 *  - ConcreteStrategy (CreditCardPayment, PayPalPayment): the variants.
 *  - Context (ShoppingCart): holds a Strategy and delegates work to it.
 *
 * Trade-offs:
 *  Pros:
 *   - Swap algorithms at run time; open for new strategies, closed for edits.
 *   - Removes conditional logic; each algorithm is isolated and testable.
 *  Cons:
 *   - More objects/types; clients must know how the strategies differ.
 *   - For a single simple algorithm the extra indirection is overkill.
 *
 * Note: In modern C++ a stateless strategy can be expressed far more cheaply
 * as a std::function<...> instead of an interface hierarchy. Both approaches
 * are shown in main().
 */

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

// --- Strategy interface ---
class PaymentStrategy {
public:
    virtual ~PaymentStrategy() = default;
    virtual void pay(double amount) const = 0;
};

// --- Concrete Strategy: credit card ---
class CreditCardPayment final : public PaymentStrategy {
public:
    CreditCardPayment(std::string holder, std::string number)
        : holder_(std::move(holder)), number_(std::move(number)) {}

    void pay(double amount) const override {
        // Show only the last 4 digits - never log full card numbers.
        const std::string tail =
            number_.size() >= 4 ? number_.substr(number_.size() - 4) : number_;
        std::cout << "Paid $" << amount << " with credit card ****" << tail
                  << " (" << holder_ << ")\n";
    }

private:
    std::string holder_;
    std::string number_;
};

// --- Concrete Strategy: PayPal ---
class PayPalPayment final : public PaymentStrategy {
public:
    explicit PayPalPayment(std::string email) : email_(std::move(email)) {}

    void pay(double amount) const override {
        std::cout << "Paid $" << amount << " via PayPal account " << email_
                  << "\n";
    }

private:
    std::string email_;
};

// --- Context ---
// Owns the current strategy but is agnostic about which one it is. Clients can
// replace the strategy at any time via setStrategy().
class ShoppingCart {
public:
    void addItem(double price) { total_ += price; }

    void setStrategy(std::unique_ptr<PaymentStrategy> strategy) {
        strategy_ = std::move(strategy);
    }

    void checkout() const {
        if (!strategy_) {
            std::cout << "No payment method selected.\n";
            return;
        }
        std::cout << "Total due: $" << total_ << " -> ";
        strategy_->pay(total_); // Delegate the "how" to the chosen strategy.
    }

private:
    double total_ = 0.0;
    std::unique_ptr<PaymentStrategy> strategy_;
};

// --- Lightweight alternative Context using std::function ---
// When a strategy carries no state and needs no class, a callable is enough.
class FunctionalCart {
public:
    using PayFn = std::function<void(double)>;

    void addItem(double price) { total_ += price; }
    void setStrategy(PayFn fn) { pay_ = std::move(fn); }
    void checkout() const {
        std::cout << "Total due: $" << total_ << " -> ";
        pay_(total_);
    }

private:
    double total_ = 0.0;
    PayFn pay_ = [](double amt) {
        std::cout << "Paid $" << amt << " in cash\n";
    };
};

// --- Demonstration ---
int main() {
    ShoppingCart cart;
    cart.addItem(29.99);
    cart.addItem(15.50);

    std::cout << "-- Pay with a credit card --\n";
    cart.setStrategy(std::make_unique<CreditCardPayment>("Ada Lovelace",
                                                         "4111111111111234"));
    cart.checkout();

    std::cout << "\n-- Same cart, switch strategy at run time --\n";
    cart.setStrategy(std::make_unique<PayPalPayment>("ada@example.com"));
    cart.checkout();

    std::cout << "\n-- std::function strategy (no class hierarchy) --\n";
    FunctionalCart quickCart;
    quickCart.addItem(7.25);
    quickCart.checkout(); // Uses the default cash lambda.
    quickCart.setStrategy([](double amt) {
        std::cout << "Paid $" << amt << " with gift card\n";
    });
    quickCart.checkout();

    return 0;
}
