/*
 * Pattern: Memento (Behavioral)
 *
 * Intent: Without violating encapsulation, capture and externalize an object's
 * internal state so that the object can be restored to this state later.
 *
 * Problem / When to use it:
 *   - You need snapshots to implement undo/redo or rollback on failure.
 *   - The object's internals must not be exposed to the code that stores state.
 *   - You want the originator to remain the sole authority over its own state.
 *
 * Real-world analogy: A text editor's undo stack, or a video game "save point":
 * you record a snapshot now and reload it later, without knowing the engine's
 * internal representation.
 *
 * Participants:
 *   - Originator: creates a memento of its state and restores from one (Editor).
 *   - Memento: opaque snapshot of the originator's state (EditorMemento).
 *   - Caretaker: keeps mementos but never inspects their contents (History).
 *
 * Trade-offs:
 *   Pros:
 *     - Preserves encapsulation: only the originator reads the memento's guts.
 *     - Simplifies the originator by offloading snapshot storage to a caretaker.
 *   Cons:
 *     - Mementos can be memory-heavy if state is large or snapshots are frequent.
 */

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// --- Memento ---
// Stores a snapshot of the Originator's state. Its accessors are private so
// only the Originator (declared a friend) can read the state back out; the
// Caretaker can hold the object but not peek inside it.
class EditorMemento {
public:
    // Public metadata is safe to expose (e.g. for a UI history label).
    const std::string& label() const { return label_; }

private:
    friend class Editor;  // Only the Originator may build and read the state.

    EditorMemento(std::string content, std::size_t cursor, std::string label)
        : content_(std::move(content)), cursor_(cursor), label_(std::move(label)) {}

    const std::string& content() const { return content_; }
    std::size_t cursor() const { return cursor_; }

    std::string content_;
    std::size_t cursor_;
    std::string label_;  // Human-readable tag describing the snapshot.
};

// --- Originator ---
// The object whose state we snapshot and restore. It alone decides what goes
// into a memento and how to apply one.
class Editor {
public:
    void type(const std::string& text) {
        content_ += text;
        cursor_ = content_.size();
    }

    void setCursor(std::size_t pos) {
        cursor_ = (pos <= content_.size()) ? pos : content_.size();
    }

    // Package the current state into an opaque memento.
    std::unique_ptr<EditorMemento> save(const std::string& label) const {
        return std::unique_ptr<EditorMemento>(
            new EditorMemento(content_, cursor_, label));
    }

    // Restore state previously captured; the memento's internals are readable
    // here because Editor is its friend.
    void restore(const EditorMemento& memento) {
        content_ = memento.content();
        cursor_ = memento.cursor();
    }

    void print() const {
        std::cout << "content=\"" << content_ << "\" cursor=" << cursor_ << '\n';
    }

private:
    std::string content_;
    std::size_t cursor_ = 0;
};

// --- Caretaker ---
// Owns the history of mementos and drives undo. It treats each memento as an
// opaque token, never touching the encapsulated state.
class History {
public:
    void push(std::unique_ptr<EditorMemento> memento) {
        std::cout << "  [history] saved: " << memento->label() << '\n';
        snapshots_.push_back(std::move(memento));
    }

    // Undo means: hand the previous snapshot back to the originator to apply.
    bool undo(Editor& editor) {
        if (snapshots_.empty()) {
            std::cout << "  [history] nothing to undo\n";
            return false;
        }
        std::unique_ptr<EditorMemento> last = std::move(snapshots_.back());
        snapshots_.pop_back();
        std::cout << "  [history] restoring: " << last->label() << '\n';
        editor.restore(*last);
        return true;
    }

private:
    std::vector<std::unique_ptr<EditorMemento>> snapshots_;
};

int main() {
    Editor editor;
    History history;

    editor.type("Hello");
    history.push(editor.save("after 'Hello'"));
    editor.print();

    editor.type(", world");
    history.push(editor.save("after ', world'"));
    editor.print();

    editor.type("!!! (oops)");
    std::cout << "current (unsaved) state: ";
    editor.print();

    std::cout << "\nUndoing twice:\n";
    history.undo(editor);  // Back to "Hello, world".
    editor.print();
    history.undo(editor);  // Back to "Hello".
    editor.print();

    return 0;
}
