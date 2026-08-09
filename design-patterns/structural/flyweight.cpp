/*
 * Pattern: Flyweight (Structural)
 *
 * Intent: Use sharing to support large numbers of fine-grained objects
 * efficiently by separating shared (intrinsic) state from context-specific
 * (extrinsic) state.
 *
 * Problem / When to use it:
 *   - An application must create a huge number of similar objects.
 *   - Storage cost is high because much of each object's state is duplicated.
 *   - That duplicated state is intrinsic (context-independent) and can be
 *     shared, while the rest is extrinsic and can be passed in per operation.
 *
 * Real-world analogy: A forest with millions of trees. Every oak shares the
 * same name, mesh and texture (intrinsic); only each trunk's map coordinate
 * (extrinsic) differs. We store one shared TreeType, not one per tree.
 *
 * Participants:
 *   - Flyweight (TreeType): stores intrinsic state; operates on extrinsic
 *     state supplied by the client.
 *   - FlyweightFactory (TreeFactory): creates and caches flyweights so shared
 *     instances are reused rather than duplicated.
 *   - Context (Tree): holds extrinsic state plus a reference to a flyweight.
 *   - Client (Forest): maintains contexts and asks the factory for flyweights.
 *
 * Trade-offs:
 *   Pros:
 *     - Drastically lowers memory when many objects share intrinsic state.
 *   Cons:
 *     - Trades RAM for CPU (extrinsic state recomputed/passed each call).
 *     - Code is more complex; flyweights should be immutable to stay shareable.
 */

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// --- Flyweight: immutable intrinsic state shared across many trees ---
class TreeType {
public:
    TreeType(std::string name, std::string color, std::string texture)
        : name_(std::move(name)), color_(std::move(color)),
          texture_(std::move(texture)) {}

    // Operation receives the extrinsic state (position) from the caller.
    void draw(int x, int y) const {
        std::cout << "Drawing " << color_ << " " << name_ << " (texture: "
                  << texture_ << ") at (" << x << ", " << y << ")\n";
    }

    const std::string& name() const { return name_; }

private:
    const std::string name_;     // intrinsic: shared
    const std::string color_;    // intrinsic: shared
    const std::string texture_;  // intrinsic: shared (imagine a large bitmap)
};

// --- FlyweightFactory: hands out cached, shared TreeType instances ---
class TreeFactory {
public:
    // Returns an existing flyweight when the key matches, else creates one.
    std::shared_ptr<TreeType> getTreeType(const std::string& name,
                                          const std::string& color,
                                          const std::string& texture) {
        const std::string key = name + "|" + color + "|" + texture;
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            return it->second;  // reuse the shared instance
        }
        auto type = std::make_shared<TreeType>(name, color, texture);
        cache_.emplace(key, type);
        std::cout << "[factory] created new TreeType: " << key << "\n";
        return type;
    }

    std::size_t distinctTypes() const { return cache_.size(); }

private:
    std::unordered_map<std::string, std::shared_ptr<TreeType>> cache_;
};

// --- Context: extrinsic state (position) + a handle to a shared flyweight ---
class Tree {
public:
    Tree(int x, int y, std::shared_ptr<TreeType> type)
        : x_(x), y_(y), type_(std::move(type)) {}

    void draw() const { type_->draw(x_, y_); }

private:
    int x_;  // extrinsic: unique per tree
    int y_;  // extrinsic: unique per tree
    std::shared_ptr<TreeType> type_;  // shared intrinsic state
};

// --- Client: plants many trees but reuses a handful of flyweights ---
class Forest {
public:
    void plantTree(int x, int y, const std::string& name,
                   const std::string& color, const std::string& texture) {
        auto type = factory_.getTreeType(name, color, texture);
        trees_.emplace_back(x, y, std::move(type));
    }

    void render() const {
        for (const auto& tree : trees_) {
            tree.draw();
        }
    }

    std::size_t treeCount() const { return trees_.size(); }
    std::size_t typeCount() const { return factory_.distinctTypes(); }

private:
    TreeFactory factory_;
    std::vector<Tree> trees_;
};

int main() {
    Forest forest;

    // Plant many trees, but only three intrinsic descriptions exist.
    forest.plantTree(1, 1, "Oak", "green", "oak.png");
    forest.plantTree(4, 2, "Oak", "green", "oak.png");
    forest.plantTree(7, 5, "Pine", "dark-green", "pine.png");
    forest.plantTree(9, 8, "Oak", "green", "oak.png");
    forest.plantTree(3, 6, "Birch", "yellow", "birch.png");
    forest.plantTree(2, 9, "Pine", "dark-green", "pine.png");

    std::cout << "\n-- Rendering forest --\n";
    forest.render();

    std::cout << "\nPlanted " << forest.treeCount() << " trees using only "
              << forest.typeCount() << " shared TreeType flyweights.\n";
    return 0;
}
