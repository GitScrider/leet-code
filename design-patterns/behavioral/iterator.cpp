/*
 * Pattern: Iterator (Behavioral)
 *
 * Intent:
 *   Provide a way to access the elements of an aggregate object sequentially
 *   without exposing its underlying representation.
 *
 * Problem / When to use it:
 *   - You need to traverse a collection without leaking its internal structure.
 *   - You want a uniform traversal interface across different aggregate types.
 *   - You want multiple, independent traversals in progress at the same time.
 *   - You want to decouple traversal algorithms from the collection itself.
 *
 * Real-world analogy:
 *   A TV remote's channel-up button: it steps you through channels one at a time
 *   without you needing to know how the channels are stored inside the TV.
 *
 * Participants:
 *   - Iterator        : defines the traversal interface (first/next/isDone/current).
 *   - ConcreteIterator: tracks the current position within a specific aggregate.
 *   - Aggregate       : defines an interface for creating an Iterator.
 *   - ConcreteAggregate: implements storage and returns a matching iterator.
 *
 * Trade-offs:
 *   Pros:
 *     - Hides representation; the same client code traverses different collections.
 *     - Supports multiple simultaneous traversals, each with its own state.
 *     - Simplifies the aggregate: traversal logic lives in the iterator.
 *   Cons:
 *     - Extra objects/classes for simple cases.
 *     - External iterators can be trickier for complex or lazily-generated data.
 */

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

// --- ConcreteAggregate: a custom playlist that hides its storage ---
class Playlist; // forward declaration for the classic (GoF-style) iterator

// --- Iterator: classic GoF-style external iterator interface ---
// first() resets, next() advances, isDone() tests completion, current() reads.
class SongIterator {
public:
    explicit SongIterator(const Playlist& list) : list_(list), pos_(0) {}

    void first();
    void next();
    bool isDone() const;
    const std::string& current() const;

private:
    const Playlist& list_; // the aggregate being traversed (not owned)
    std::size_t pos_;       // position state lives in the iterator, not the aggregate
};

// --- ConcreteAggregate ---
class Playlist {
public:
    void add(const std::string& song) { songs_.push_back(song); }

    // Factory method: hand out an iterator matched to this aggregate.
    SongIterator createIterator() const { return SongIterator(*this); }

    // --- STL-style support so range-based for works too ---
    // Exposing begin()/end() lets clients use idiomatic C++ traversal without
    // knowing that a std::vector backs the collection.
    std::vector<std::string>::const_iterator begin() const { return songs_.begin(); }
    std::vector<std::string>::const_iterator end() const { return songs_.end(); }

private:
    friend class SongIterator; // iterator needs read access to storage details
    std::vector<std::string> songs_;
};

// --- ConcreteIterator method definitions (need the full Playlist type) ---
void SongIterator::first() { pos_ = 0; }
void SongIterator::next() { ++pos_; }
bool SongIterator::isDone() const { return pos_ >= list_.songs_.size(); }
const std::string& SongIterator::current() const { return list_.songs_[pos_]; }

int main() {
    // --- Client: build the aggregate ---
    Playlist playlist;
    playlist.add("Bohemian Rhapsody");
    playlist.add("Stairway to Heaven");
    playlist.add("Hotel California");
    playlist.add("Comfortably Numb");

    // 1) Traverse using the explicit GoF-style iterator interface.
    std::cout << "Classic iterator traversal:\n";
    SongIterator it = playlist.createIterator();
    for (it.first(); !it.isDone(); it.next()) {
        std::cout << "  - " << it.current() << "\n";
    }
    std::cout << "----------------------------------------\n";

    // 2) Two independent iterators can traverse concurrently, each with its
    //    own position; this is why traversal state must live in the iterator.
    std::cout << "Two concurrent iterators (pairing songs):\n";
    SongIterator a = playlist.createIterator();
    SongIterator b = playlist.createIterator();
    b.next(); // offset the second iterator by one
    for (a.first(); !a.isDone() && !b.isDone(); a.next(), b.next()) {
        std::cout << "  " << a.current() << "  ->  " << b.current() << "\n";
    }
    std::cout << "----------------------------------------\n";

    // 3) The same collection also works with a range-based for loop.
    std::cout << "Range-based for traversal:\n";
    for (const std::string& song : playlist) {
        std::cout << "  * " << song << "\n";
    }

    return 0;
}
