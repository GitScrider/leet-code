/*
 * Pattern: Chain of Responsibility (Behavioral)
 *
 * Intent:
 *   Decouple the sender of a request from its receivers by giving multiple
 *   objects a chance to handle the request. The receiving objects are chained,
 *   and the request is passed along the chain until one of them handles it.
 *
 * Problem / When to use it:
 *   - More than one object may handle a request and the handler is not known a priori.
 *   - You want to issue a request without specifying the receiver explicitly.
 *   - The set of handlers (and their order) should be configurable at runtime.
 *   - You want to avoid a monolithic if/else or switch that couples all cases together.
 *
 * Real-world analogy:
 *   A support desk: a ticket first hits the front-line agent; if they cannot
 *   resolve it, it escalates to a supervisor, then to a manager, and so on.
 *
 * Participants:
 *   - Handler        : declares the interface for handling and holds a successor link.
 *   - ConcreteHandler: handles requests it is responsible for, or forwards to its successor.
 *   - Client         : builds the chain and submits requests to its head.
 *
 * Trade-offs:
 *   Pros:
 *     - Reduces coupling: sender needn't know which handler will service the request.
 *     - Adds flexibility in assigning responsibilities; reorder/insert handlers freely.
 *   Cons:
 *     - Reception is not guaranteed: a request may fall off the end unhandled.
 *     - Can be harder to observe/debug the runtime path a request took.
 */

#include <iostream>
#include <memory>
#include <string>
#include <utility>

// --- Request: a small value object carrying the data to be handled ---
enum class Severity { Info, Warning, Error, Critical };

struct SupportTicket {
    Severity severity;
    std::string description;
};

static std::string toString(Severity s) {
    switch (s) {
        case Severity::Info:     return "INFO";
        case Severity::Warning:  return "WARNING";
        case Severity::Error:    return "ERROR";
        case Severity::Critical: return "CRITICAL";
    }
    return "UNKNOWN";
}

// --- Handler: abstract base defining the handling interface + successor link ---
class SupportHandler {
public:
    virtual ~SupportHandler() = default;

    // Fluent setter so the client can build the chain in one expression.
    // Returns the *next* handler to allow chaining: a->setNext(b)->setNext(c).
    SupportHandler* setNext(std::unique_ptr<SupportHandler> next) {
        SupportHandler* raw = next.get();
        next_ = std::move(next);
        return raw;
    }

    // Template method: try to handle here; otherwise forward to the successor.
    void handle(const SupportTicket& ticket) {
        if (canHandle(ticket)) {
            process(ticket);
        } else if (next_) {
            std::cout << "  [" << name() << "] escalating...\n";
            next_->handle(ticket);
        } else {
            std::cout << "  [end of chain] Ticket unhandled: \""
                      << ticket.description << "\"\n";
        }
    }

protected:
    virtual bool canHandle(const SupportTicket& ticket) const = 0;
    virtual void process(const SupportTicket& ticket) const = 0;
    virtual std::string name() const = 0;

private:
    std::unique_ptr<SupportHandler> next_; // owns the rest of the chain
};

// --- ConcreteHandler: front-line agent, resolves low-severity tickets ---
class FrontLineAgent final : public SupportHandler {
protected:
    bool canHandle(const SupportTicket& t) const override {
        return t.severity == Severity::Info;
    }
    void process(const SupportTicket& t) const override {
        std::cout << "  [FrontLineAgent] resolved " << toString(t.severity)
                  << ": " << t.description << "\n";
    }
    std::string name() const override { return "FrontLineAgent"; }
};

// --- ConcreteHandler: supervisor handles warnings/errors ---
class Supervisor final : public SupportHandler {
protected:
    bool canHandle(const SupportTicket& t) const override {
        return t.severity == Severity::Warning || t.severity == Severity::Error;
    }
    void process(const SupportTicket& t) const override {
        std::cout << "  [Supervisor] handled " << toString(t.severity)
                  << ": " << t.description << "\n";
    }
    std::string name() const override { return "Supervisor"; }
};

// --- ConcreteHandler: manager is the last resort for critical incidents ---
class Manager final : public SupportHandler {
protected:
    bool canHandle(const SupportTicket& t) const override {
        return t.severity == Severity::Critical;
    }
    void process(const SupportTicket& t) const override {
        std::cout << "  [Manager] escalated to on-call, " << toString(t.severity)
                  << ": " << t.description << "\n";
    }
    std::string name() const override { return "Manager"; }
};

int main() {
    // --- Client: assemble the chain FrontLine -> Supervisor -> Manager ---
    auto head = std::make_unique<FrontLineAgent>();
    head->setNext(std::make_unique<Supervisor>())
        ->setNext(std::make_unique<Manager>());

    // A batch of requests is fed to the head; each traverses until handled.
    const SupportTicket tickets[] = {
        {Severity::Info,     "How do I reset my password?"},
        {Severity::Error,    "Checkout page throws 500"},
        {Severity::Critical, "Production database is down"},
        {Severity::Warning,  "Disk usage at 85%"},
    };

    for (const auto& ticket : tickets) {
        std::cout << "Submitting " << toString(ticket.severity)
                  << " ticket: \"" << ticket.description << "\"\n";
        head->handle(ticket);
        std::cout << "----------------------------------------\n";
    }

    return 0;
}
