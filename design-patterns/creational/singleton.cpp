/*
 * Pattern: Singleton (Creational)
 *
 * Intent: Ensure a class has exactly one instance and provide a single global
 *         point of access to it.
 *
 * Problem / When to use it:
 *   - Exactly one object must coordinate a shared resource (a log sink, an
 *     app-wide configuration, a connection pool).
 *   - A plain global variable cannot guarantee controlled, lazy creation.
 *   - Many parts of the code need the same instance without passing it around.
 *
 * Real-world analogy: A building's single physical thermostat -- every room's
 *   controls talk to the one device that owns the true state.
 *
 * Participants:
 *   - Singleton: Logger (owns the sole instance and the access point)
 *   - Client   : main() (retrieves the instance via Logger::instance())
 *
 * Trade-offs:
 *   Pros:
 *     - Guarantees a single instance with controlled, lazy initialization.
 *     - Global access point without an unmanaged global variable.
 *   Cons:
 *     - Hidden global state hurts testability and hides dependencies.
 *     - Easy to overuse; can become a bottleneck or mask tight coupling.
 *
 * Thread-safety note (Meyers singleton):
 *   Since C++11, initialization of a function-local 'static' is guaranteed to
 *   run exactly once, and concurrent callers block until it completes. So the
 *   instance() below is thread-safe for construction without extra locking.
 *   (Ongoing concurrent *use* of the object still needs its own sync -- here a
 *   mutex guards log().)
 */

#include <iostream>
#include <mutex>
#include <string>

// --- Singleton ---
class Logger {
public:
    // Global access point. The static local is initialized once, lazily,
    // in a thread-safe way (C++11 "magic statics").
    static Logger& instance() {
        static Logger instance;  // constructed on first call only
        return instance;
    }

    // Delete copy/move so no second instance can ever be created.
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    void setLevel(const std::string& level) {
        std::lock_guard<std::mutex> lock(mutex_);
        level_ = level;
    }

    void log(const std::string& message) {
        // Guard concurrent writes; construction is already safe, but shared
        // *use* of the single instance still needs synchronization.
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "[" << level_ << "] " << message
                  << " (msg #" << ++count_ << ")\n";
    }

private:
    // Private ctor/dtor: only instance() may create/destroy the object.
    Logger() { std::cout << "(Logger constructed once)\n"; }
    ~Logger() = default;

    std::string level_{"INFO"};
    int count_{0};
    std::mutex mutex_;
};

// A helper that also reaches for the logger, proving both call sites share
// the very same object.
void doWork() {
    Logger::instance().log("work done in helper");
}

int main() {
    // First access triggers construction.
    Logger::instance().log("application starting");

    Logger::instance().setLevel("DEBUG");
    doWork();

    // Prove it is one and the same instance: same address everywhere.
    Logger& a = Logger::instance();
    Logger& b = Logger::instance();
    std::cout << "Same instance? " << std::boolalpha
              << (&a == &b) << "\n";

    a.log("final message");
    return 0;
}
