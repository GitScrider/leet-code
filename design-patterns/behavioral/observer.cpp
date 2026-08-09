/*
 * Pattern: Observer (Behavioral)
 *
 * Intent: Define a one-to-many dependency between objects so that when one
 * object (the subject) changes state, all its dependents are notified and
 * updated automatically.
 *
 * Problem / When to use it:
 *   - A change to one object requires changing an open-ended set of others.
 *   - You want to broadcast updates without the subject knowing concrete types.
 *   - Observers should subscribe/unsubscribe at runtime.
 *
 * Real-world analogy: A weather station's public feed. Displays, apps, and
 * dashboards subscribe; when a new measurement arrives, every subscriber is
 * pushed the update, and any can unsubscribe at any time.
 *
 * Participants:
 *   - Subject: registers/removes observers and notifies them (WeatherStation).
 *   - Observer: interface with the update hook the subject calls (Observer).
 *   - ConcreteObserver: reacts to updates (CurrentConditionsDisplay, ...).
 *
 * Trade-offs:
 *   Pros:
 *     - Loose coupling: subject knows only the Observer interface.
 *     - Observers can be added/removed dynamically at runtime.
 *   Cons:
 *     - Notification order is unspecified; cascading updates can be hard to trace.
 *     - Dangling observers cause bugs if lifetimes are not managed carefully.
 */

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// --- Observer interface ---
// The single hook the subject invokes to push new state.
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(float temperatureC, float humidity) = 0;
    virtual std::string name() const = 0;
};

// --- Subject ---
// Maintains a list of observers and broadcasts state changes. It depends only
// on the Observer interface, never on concrete observer classes.
class WeatherStation {
public:
    // Non-owning: observers are owned by the caller; we only hold references.
    void attach(const std::shared_ptr<Observer>& observer) {
        observers_.push_back(observer);
        std::cout << "[station] attached " << observer->name() << '\n';
    }

    void detach(const std::shared_ptr<Observer>& observer) {
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                           [&](const std::weak_ptr<Observer>& w) {
                               auto s = w.lock();
                               return !s || s == observer;
                           }),
            observers_.end());
        std::cout << "[station] detached " << observer->name() << '\n';
    }

    // State change is the trigger: update internal data, then notify everyone.
    void setMeasurements(float temperatureC, float humidity) {
        temperatureC_ = temperatureC;
        humidity_ = humidity;
        notify();
    }

private:
    void notify() {
        std::cout << "[station] new reading: " << temperatureC_ << "C, "
                  << humidity_ << "% -> notifying observers\n";
        // Using weak_ptr lets expired observers be skipped and pruned safely.
        for (auto it = observers_.begin(); it != observers_.end();) {
            if (auto observer = it->lock()) {
                observer->update(temperatureC_, humidity_);
                ++it;
            } else {
                it = observers_.erase(it);  // Prune observers that are gone.
            }
        }
    }

    std::vector<std::weak_ptr<Observer>> observers_;
    float temperatureC_ = 0.0f;
    float humidity_ = 0.0f;
};

// --- Concrete Observer: live conditions readout ---
class CurrentConditionsDisplay final : public Observer {
public:
    void update(float temperatureC, float humidity) override {
        std::cout << "    [Current] " << temperatureC << "C and " << humidity
                  << "% humidity\n";
    }
    std::string name() const override { return "CurrentConditionsDisplay"; }
};

// --- Concrete Observer: tracks min/max over time ---
class StatisticsDisplay final : public Observer {
public:
    void update(float temperatureC, float /*humidity*/) override {
        min_ = std::min(min_, temperatureC);
        max_ = std::max(max_, temperatureC);
        std::cout << "    [Stats]   min=" << min_ << "C max=" << max_ << "C\n";
    }
    std::string name() const override { return "StatisticsDisplay"; }

private:
    float min_ = 1e9f;
    float max_ = -1e9f;
};

int main() {
    WeatherStation station;

    auto current = std::make_shared<CurrentConditionsDisplay>();
    auto stats = std::make_shared<StatisticsDisplay>();

    station.attach(current);
    station.attach(stats);

    std::cout << "\n--- First reading ---\n";
    station.setMeasurements(22.4f, 55.0f);

    std::cout << "\n--- Second reading ---\n";
    station.setMeasurements(19.1f, 70.0f);

    std::cout << "\n--- Detaching the current-conditions display ---\n";
    station.detach(current);

    std::cout << "\n--- Third reading (only stats remains) ---\n";
    station.setMeasurements(25.8f, 40.0f);

    return 0;
}
