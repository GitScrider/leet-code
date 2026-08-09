/*
 * Pattern: Command (Behavioral)
 *
 * Intent:
 *   Encapsulate a request as an object, thereby letting you parameterize clients
 *   with different requests, queue or log requests, and support undoable operations.
 *
 * Problem / When to use it:
 *   - You want to decouple the object that invokes an operation from the one that performs it.
 *   - You need undo/redo, queuing, macros, or logging of operations.
 *   - You want to parameterize objects with an action to perform (like a callback, but as an object).
 *   - You want to support deferred or scheduled execution of a request.
 *
 * Real-world analogy:
 *   A restaurant order slip: the waiter (Invoker) writes a request onto a slip
 *   (Command) and hands it to the kitchen (Receiver). The slip decouples who asks
 *   from who cooks, and can be stacked, queued, or cancelled.
 *
 * Participants:
 *   - Command        : declares execute() / undo().
 *   - ConcreteCommand: binds a Receiver to an action and stores state for undo.
 *   - Receiver       : knows how to perform the actual work.
 *   - Invoker        : triggers commands and keeps a history to support undo.
 *   - Client         : creates ConcreteCommands and configures the invoker.
 *
 * Trade-offs:
 *   Pros:
 *     - Decouples invoker from receiver; commands are first-class objects.
 *     - Easy to add new commands without changing existing code (Open/Closed).
 *     - Enables undo/redo, macros (composite commands), queuing, and logging.
 *   Cons:
 *     - Can proliferate many small command classes.
 *     - Undo requires each command to capture enough state to reverse itself.
 */

#include <cstddef>
#include <iostream>
#include <memory>
#include <stack>
#include <string>

// --- Receiver: the object that actually performs the work ---
// A tiny text document; commands mutate it and can reverse their effect.
class Document {
public:
    void append(const std::string& text) { content_ += text; }

    // Remove the last `n` characters (used by AppendCommand::undo).
    void removeFromEnd(std::size_t n) {
        if (n >= content_.size()) content_.clear();
        else content_.erase(content_.size() - n);
    }

    const std::string& content() const { return content_; }

private:
    std::string content_;
};

// --- Command: abstract interface with execute() and undo() ---
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string label() const = 0;
};

// --- ConcreteCommand: append text; undo removes exactly what it added ---
class AppendCommand final : public Command {
public:
    AppendCommand(Document& doc, std::string text)
        : doc_(doc), text_(std::move(text)) {}

    void execute() override { doc_.append(text_); }
    void undo() override { doc_.removeFromEnd(text_.size()); } // reverse the append
    std::string label() const override { return "Append(\"" + text_ + "\")"; }

private:
    Document& doc_;    // reference to the receiver (not owned)
    std::string text_; // state needed to both perform and reverse the action
};

// --- Invoker: triggers commands and records history for undo ---
class Editor {
public:
    // Execute a command now and push it so it can be undone later.
    void run(std::unique_ptr<Command> cmd) {
        std::cout << "Execute: " << cmd->label() << "\n";
        cmd->execute();
        history_.push(std::move(cmd));
    }

    void undo() {
        if (history_.empty()) {
            std::cout << "Undo: nothing to undo\n";
            return;
        }
        std::unique_ptr<Command> cmd = std::move(history_.top());
        history_.pop();
        std::cout << "Undo:    " << cmd->label() << "\n";
        cmd->undo();
    }

private:
    std::stack<std::unique_ptr<Command>> history_; // LIFO enables sequential undo
};

int main() {
    // --- Client: wire up receiver, invoker, and concrete commands ---
    Document doc;
    Editor editor;

    editor.run(std::make_unique<AppendCommand>(doc, "Hello"));
    editor.run(std::make_unique<AppendCommand>(doc, ", World"));
    editor.run(std::make_unique<AppendCommand>(doc, "!"));
    std::cout << "Content: \"" << doc.content() << "\"\n";
    std::cout << "----------------------------------------\n";

    // Undo the last two commands in reverse order.
    editor.undo(); // removes "!"
    editor.undo(); // removes ", World"
    std::cout << "Content: \"" << doc.content() << "\"\n";
    std::cout << "----------------------------------------\n";

    // The invoker doesn't care what the command does; it only calls execute().
    editor.run(std::make_unique<AppendCommand>(doc, " again"));
    std::cout << "Content: \"" << doc.content() << "\"\n";

    return 0;
}
