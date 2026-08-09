/*
 * Pattern: Mediator (Behavioral)
 *
 * Intent: Define an object that encapsulates how a set of objects interact.
 * Mediator promotes loose coupling by keeping objects from referring to each
 * other explicitly, letting you vary their interaction independently.
 *
 * Problem / When to use it:
 *   - Many objects communicate in a tangled, many-to-many web (n^2 references).
 *   - You want to reuse a component without dragging its peers along with it.
 *   - Interaction logic keeps changing and is scattered across the colleagues.
 *   - You want to centralize control of who-talks-to-whom in one place.
 *
 * Real-world analogy: An air-traffic control tower. Pilots never negotiate
 * runways directly with one another; each talks only to the tower, which
 * coordinates everyone.
 *
 * Participants:
 *   - Mediator: interface declaring how colleagues notify it (ChatRoomMediator).
 *   - ConcreteMediator: coordinates colleagues and knows them all (ChatRoom).
 *   - Colleague: base type that holds a Mediator reference (User).
 *   - ConcreteColleague: talks only through the mediator (ChatUser).
 *
 * Trade-offs:
 *   Pros:
 *     - Removes tight coupling among colleagues; they depend only on the mediator.
 *     - Centralizes interaction logic, easing maintenance and reuse.
 *   Cons:
 *     - The mediator can grow into a "god object" concentrating too much logic.
 */

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Forward declaration so the Colleague can reference the Mediator interface.
class User;

// --- Mediator interface ---
// Declares the single channel colleagues use to communicate indirectly.
class ChatRoomMediator {
public:
    virtual ~ChatRoomMediator() = default;
    // A colleague asks the mediator to route a message on its behalf.
    virtual void broadcast(const std::string& from, const std::string& message) = 0;
    virtual void registerUser(User* user) = 0;
};

// --- Colleague (abstract) ---
// Knows only the mediator, never its peers directly.
class User {
public:
    explicit User(std::string name, ChatRoomMediator* room)
        : name_(std::move(name)), room_(room) {}
    virtual ~User() = default;

    const std::string& name() const { return name_; }

    // Sending never targets a peer directly; it delegates to the mediator.
    void send(const std::string& message) {
        std::cout << "[" << name_ << " sends]  " << message << '\n';
        room_->broadcast(name_, message);
    }

    // The mediator calls this to deliver a routed message.
    virtual void receive(const std::string& from, const std::string& message) = 0;

protected:
    std::string name_;
    ChatRoomMediator* room_;  // Non-owning: the mediator outlives the colleagues.
};

// --- Concrete Colleague ---
class ChatUser final : public User {
public:
    using User::User;

    void receive(const std::string& from, const std::string& message) override {
        std::cout << "    -> " << name_ << " got from " << from << ": " << message << '\n';
    }
};

// --- Concrete Mediator ---
// Holds all colleagues and implements the interaction policy (broadcast to all
// except the sender). Colleagues stay ignorant of this policy.
class ChatRoom final : public ChatRoomMediator {
public:
    void registerUser(User* user) override {
        users_.push_back(user);
    }

    void broadcast(const std::string& from, const std::string& message) override {
        for (User* user : users_) {
            if (user->name() != from) {  // Do not echo back to the sender.
                user->receive(from, message);
            }
        }
    }

private:
    std::vector<User*> users_;  // Non-owning references to registered colleagues.
};

int main() {
    ChatRoom room;

    // Colleagues are owned here; the mediator only keeps non-owning pointers.
    std::vector<std::unique_ptr<User>> members;
    members.push_back(std::make_unique<ChatUser>("Alice", &room));
    members.push_back(std::make_unique<ChatUser>("Bob", &room));
    members.push_back(std::make_unique<ChatUser>("Carol", &room));

    for (const auto& member : members) {
        room.registerUser(member.get());
    }

    // Users interact without ever naming each other: the mediator wires it up.
    members[0]->send("Hi everyone!");
    std::cout << '\n';
    members[1]->send("Hey Alice, glad you're here.");

    return 0;
}
