/*
 * Pattern: Facade (Structural)
 *
 * Intent: Provide a single, unified, high-level interface to a set of
 * interfaces in a subsystem, making the subsystem easier to use.
 *
 * Problem / When to use it:
 *   - A subsystem has many interdependent classes and a fiddly usage protocol.
 *   - You want to decouple clients from subsystem internals (looser coupling).
 *   - You need a simple default entry point but still allow expert access.
 *   - You want to layer your system: the facade is the door into a layer.
 *
 * Real-world analogy: A universal remote's "Watch Movie" button. One press
 * dims the lights, powers the TV, switches the amplifier and starts the player,
 * hiding the exact order and details of each device.
 *
 * Participants:
 *   - Facade (HomeTheaterFacade): knows which subsystem objects handle a
 *     request and delegates to them in the right order.
 *   - Subsystem classes (Amplifier, Projector, ...): do the real work; they
 *     know nothing about the facade and can still be used directly.
 *
 * Trade-offs:
 *   Pros:
 *     - Shields clients from subsystem complexity; fewer objects to juggle.
 *     - Promotes weak coupling between clients and subsystems.
 *   Cons:
 *     - The facade can become a "god object" coupled to everything.
 *     - Adds an extra layer; may hide capabilities power users still need.
 */

#include <iostream>
#include <memory>
#include <string>

// --- Subsystem classes: each is independent and unaware of the facade ---

class Amplifier {
public:
    void on() { std::cout << "Amplifier: powering on\n"; }
    void setVolume(int level) {
        std::cout << "Amplifier: volume set to " << level << "\n";
    }
    void off() { std::cout << "Amplifier: powering off\n"; }
};

class Projector {
public:
    void on() { std::cout << "Projector: powering on\n"; }
    void wideScreenMode() {
        std::cout << "Projector: switching to 16:9 widescreen\n";
    }
    void off() { std::cout << "Projector: powering off\n"; }
};

class TheaterLights {
public:
    void dim(int percent) {
        std::cout << "Lights: dimming to " << percent << "%\n";
    }
    void on() { std::cout << "Lights: full brightness\n"; }
};

class StreamingPlayer {
public:
    void on() { std::cout << "Player: powering on\n"; }
    void play(const std::string& movie) {
        std::cout << "Player: now playing \"" << movie << "\"\n";
    }
    void stop() { std::cout << "Player: stopped\n"; }
    void off() { std::cout << "Player: powering off\n"; }
};

// --- Facade: a thin orchestrator over the subsystem ---
// It owns the subsystem parts and exposes two coarse operations that encode
// the correct sequence of low-level calls the client would otherwise repeat.
class HomeTheaterFacade {
public:
    HomeTheaterFacade()
        : amp_(std::make_unique<Amplifier>()),
          projector_(std::make_unique<Projector>()),
          lights_(std::make_unique<TheaterLights>()),
          player_(std::make_unique<StreamingPlayer>()) {}

    // High-level operation: hides the ordering and configuration details.
    void watchMovie(const std::string& movie) {
        std::cout << "-- Get ready to watch a movie --\n";
        lights_->dim(10);
        projector_->on();
        projector_->wideScreenMode();
        amp_->on();
        amp_->setVolume(7);
        player_->on();
        player_->play(movie);
        std::cout << "\n";
    }

    // Tears everything down in a sensible order.
    void endMovie() {
        std::cout << "-- Shutting the theater down --\n";
        player_->stop();
        player_->off();
        amp_->off();
        projector_->off();
        lights_->on();
        std::cout << "\n";
    }

private:
    std::unique_ptr<Amplifier> amp_;
    std::unique_ptr<Projector> projector_;
    std::unique_ptr<TheaterLights> lights_;
    std::unique_ptr<StreamingPlayer> player_;
};

int main() {
    // The client talks only to the facade: two calls instead of a dozen.
    HomeTheaterFacade theater;
    theater.watchMovie("The C++ Chronicles");
    theater.endMovie();

    // The subsystem remains accessible directly for advanced needs.
    std::cout << "-- Direct subsystem access (power user) --\n";
    Amplifier standalone;
    standalone.on();
    standalone.setVolume(3);
    standalone.off();
    return 0;
}
