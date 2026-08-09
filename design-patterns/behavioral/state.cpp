/*
 * State (Behavioral)
 *
 * Intent: Allow an object to alter its behavior when its internal state
 * changes. The object will appear to change its class.
 *
 * Problem / When to use it:
 *  - An object's behavior depends on its state and must change at run time.
 *  - Code is littered with large conditionals that branch on a state field.
 *  - Each state has distinct behavior AND distinct transition rules.
 *
 * Real-world analogy: A vending machine reacts differently to the same
 * actions ("insert coin", "press button") depending on whether it is empty,
 * has taken a coin, or is sold out.
 *
 * Participants:
 *  - State: interface declaring the state-dependent operations.
 *  - ConcreteState (NoCoin, HasCoin, SoldOut): implement behavior for one
 *    state and decide which state the Context moves to next.
 *  - Context (VendingMachine): owns the current State and delegates to it.
 *
 * Trade-offs:
 *  Pros:
 *   - Replaces sprawling switch/if chains with small, named classes.
 *   - Transitions are explicit; each state is closed for modification.
 *  Cons:
 *   - More classes/objects than a single enum + switch for tiny machines.
 *   - Transition knowledge is distributed across the concrete states.
 */

#include <iostream>
#include <memory>
#include <string>

class VendingMachine; // Context, forward-declared for the State interface.

// --- State interface ---
// Declares one handler per event plus a human-readable name for logging.
class State {
public:
    virtual ~State() = default;
    virtual void insertCoin(VendingMachine& machine) = 0;
    virtual void pressButton(VendingMachine& machine) = 0;
    virtual std::string name() const = 0;
};

// --- Context ---
// Holds the active State and forwards requests to it. Client code never
// touches State subclasses directly; it only manipulates the Context.
class VendingMachine {
public:
    explicit VendingMachine(int stock);

    void insertCoin() { state_->insertCoin(*this); }
    void pressButton() { state_->pressButton(*this); }

    // Called by the concrete states to perform a transition. Kept as the last
    // action inside a handler so the outgoing state is not used after deletion.
    void setState(std::unique_ptr<State> next) {
        std::cout << "  [transition] " << state_->name() << " -> "
                  << next->name() << '\n';
        state_ = std::move(next);
    }

    int stock() const { return stock_; }
    void dispenseOne() { --stock_; }

private:
    std::unique_ptr<State> state_;
    int stock_;
};

// --- Concrete States (declarations) ---
// Declared up front because their transitions reference one another; the
// bodies are defined out of line once every type is a complete type.
class NoCoinState final : public State {
public:
    void insertCoin(VendingMachine& machine) override;
    void pressButton(VendingMachine& machine) override;
    std::string name() const override { return "NoCoin"; }
};

class HasCoinState final : public State {
public:
    void insertCoin(VendingMachine& machine) override;
    void pressButton(VendingMachine& machine) override;
    std::string name() const override { return "HasCoin"; }
};

class SoldOutState final : public State {
public:
    void insertCoin(VendingMachine& machine) override;
    void pressButton(VendingMachine& machine) override;
    std::string name() const override { return "SoldOut"; }
};

// --- Concrete State behavior ---
void NoCoinState::insertCoin(VendingMachine& machine) {
    std::cout << "Coin accepted.\n";
    machine.setState(std::make_unique<HasCoinState>());
}
void NoCoinState::pressButton(VendingMachine& machine) {
    (void)machine; // No transition: illustrates a request ignored in this state.
    std::cout << "Please insert a coin first.\n";
}

void HasCoinState::insertCoin(VendingMachine& machine) {
    (void)machine;
    std::cout << "A coin is already inserted.\n";
}
void HasCoinState::pressButton(VendingMachine& machine) {
    std::cout << "Dispensing a drink...\n";
    machine.dispenseOne();
    if (machine.stock() > 0)
        machine.setState(std::make_unique<NoCoinState>());
    else
        machine.setState(std::make_unique<SoldOutState>());
}

void SoldOutState::insertCoin(VendingMachine& machine) {
    (void)machine;
    std::cout << "Machine sold out. Returning your coin.\n";
}
void SoldOutState::pressButton(VendingMachine& machine) {
    (void)machine;
    std::cout << "Machine sold out. No drinks left.\n";
}

// The Context starts in the state that matches its initial data.
VendingMachine::VendingMachine(int stock) : stock_(stock) {
    if (stock_ > 0)
        state_ = std::make_unique<NoCoinState>();
    else
        state_ = std::make_unique<SoldOutState>();
}

// --- Demonstration ---
int main() {
    VendingMachine machine(2); // Two drinks in stock.

    std::cout << "-- Press button with no coin --\n";
    machine.pressButton();

    std::cout << "\n-- Buy first drink --\n";
    machine.insertCoin();
    machine.pressButton();

    std::cout << "\n-- Buy second (last) drink --\n";
    machine.insertCoin();
    machine.insertCoin(); // Extra coin is rejected by HasCoinState.
    machine.pressButton();

    std::cout << "\n-- Try again after sold out --\n";
    machine.insertCoin();
    machine.pressButton();

    return 0;
}
