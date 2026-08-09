/*
 * Pattern: Composite (Structural)
 *
 * Intent: Compose objects into tree structures to represent part-whole
 * hierarchies, and let clients treat individual objects and compositions
 * of objects uniformly.
 *
 * Problem / When to use it:
 *   - You need to represent a part-whole hierarchy (trees of objects).
 *   - Clients should ignore the difference between a single object (leaf)
 *     and a group of objects (composite).
 *   - Operations should recurse naturally over the whole structure.
 *   - You want to add new node kinds without changing client code.
 *
 * Real-world analogy: A file system. A directory can contain files and other
 * directories; asking for the "size" of a directory recurses into everything
 * it holds, while a file just reports its own size.
 *
 * Participants:
 *   - Component (FileSystemNode): common interface for leaves and composites.
 *   - Leaf (File): a node with no children; implements the operations directly.
 *   - Composite (Directory): a node that stores children and forwards
 *     operations to them recursively.
 *   - Client (main): manipulates the tree only through the Component interface.
 *
 * Trade-offs:
 *   Pros:
 *     - Uniform treatment of leaves and composites simplifies client code.
 *     - Adding new component types is easy (Open/Closed Principle).
 *     - Recursive structure mirrors the recursive nature of the problem.
 *   Cons:
 *     - The shared interface can become overly general (e.g. child-management
 *       methods that make no sense on a leaf).
 *     - Type safety is weaker: it is harder to restrict what a composite holds.
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// --- Component: the common interface for both leaves and composites ---
class FileSystemNode {
public:
    virtual ~FileSystemNode() = default;

    // The core operation shared by every node in the tree.
    virtual std::size_t getSize() const = 0;

    // Pretty-print the node; 'indent' expresses depth for readability.
    virtual void print(int indent = 0) const = 0;

protected:
    // Small helper so subclasses share the indentation logic.
    static void printIndent(int indent) {
        for (int i = 0; i < indent; ++i) {
            std::cout << "  ";
        }
    }
};

// --- Leaf: a File has no children and answers operations directly ---
class File final : public FileSystemNode {
public:
    File(std::string name, std::size_t bytes)
        : name_(std::move(name)), bytes_(bytes) {}

    std::size_t getSize() const override { return bytes_; }

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "- " << name_ << " (" << bytes_ << " bytes)\n";
    }

private:
    std::string name_;
    std::size_t bytes_;
};

// --- Composite: a Directory owns children and forwards operations recursively ---
class Directory final : public FileSystemNode {
public:
    explicit Directory(std::string name) : name_(std::move(name)) {}

    // The composite owns its children via unique_ptr (no raw new/delete).
    void add(std::unique_ptr<FileSystemNode> child) {
        children_.push_back(std::move(child));
    }

    // Size of a directory is the recursive sum of all contained nodes.
    std::size_t getSize() const override {
        std::size_t total = 0;
        for (const auto& child : children_) {
            total += child->getSize();
        }
        return total;
    }

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "+ " << name_ << "/ (" << getSize() << " bytes total)\n";
        for (const auto& child : children_) {
            child->print(indent + 1); // forward the operation to each child
        }
    }

private:
    std::string name_;
    std::vector<std::unique_ptr<FileSystemNode>> children_;
};

// --- Client: builds and queries the tree only through FileSystemNode ---
int main() {
    // Build a small file-system tree:
    //   root/
    //     readme.txt
    //     src/
    //       main.cpp
    //       util.cpp
    //     assets/
    //       logo.png
    auto root = std::make_unique<Directory>("root");
    root->add(std::make_unique<File>("readme.txt", 1200));

    auto src = std::make_unique<Directory>("src");
    src->add(std::make_unique<File>("main.cpp", 4096));
    src->add(std::make_unique<File>("util.cpp", 2048));

    auto assets = std::make_unique<Directory>("assets");
    assets->add(std::make_unique<File>("logo.png", 8192));

    root->add(std::move(src));
    root->add(std::move(assets));

    // The client treats the whole tree uniformly through the base interface.
    std::cout << "File system layout:\n";
    root->print();

    std::cout << "\nTotal size of 'root': " << root->getSize() << " bytes\n";

    return 0;
}
