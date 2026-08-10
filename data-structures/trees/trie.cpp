/*
 * Trie (Prefix Tree)  -- Data Structure - Tree
 *
 * Summary:
 *   A trie stores a set of strings as a tree of characters: each edge is labelled
 *   by a letter and each root-to-node path spells a prefix. Words that share a
 *   prefix share the same path, so the structure answers prefix queries directly.
 *   This trie is specialised for keys over the lowercase alphabet 'a'..'z'.
 *
 * Operations & complexity  (L = length of the word/prefix):
 *   Operation      | Time  | Notes
 *   ---------------+-------+------------------------------------------------
 *   insert(word)   | O(L)  | walk/create one node per character
 *   search(word)   | O(L)  | exact match: path must exist AND end be marked
 *   startsWith(pre)| O(L)  | path must exist; end need not be a word
 *   remove(word)   | O(L)  | unmark, then prune now-useless nodes
 *
 *   Time depends only on the key length, NOT on how many words are stored --
 *   that prefix-sharing is the trie's signature advantage over a hash set for
 *   prefix work. Space trade-off: each node reserves a fixed 26-way child array,
 *   so sparse tries can waste memory (an unordered_map<char,Node*> trades that
 *   memory for pointer overhead and hashing; the array is chosen here for speed
 *   and simplicity).
 *
 * Invariants:
 *   1. The root represents the empty prefix and is never a word by itself unless
 *      the empty string was explicitly inserted.
 *   2. A node's isEndOfWord flag is true iff the path from the root to that node
 *      is a complete inserted word.
 *   3. Pruning invariant: after remove(), no node is left that is neither the end
 *      of a word nor an ancestor of some word (i.e. no dead branches linger).
 *
 * When to use / trade-offs:
 *   - Autocomplete, prefix search, dictionary/spell-check, IP routing tables.
 *   - Longest-prefix and "all words with prefix" queries are natural.
 *   - Uses more memory than a plain hash set; not ideal for a handful of long,
 *     non-overlapping keys.
 */

#include <array>
#include <string>
#include <vector>
#include <cstddef>   // std::size_t
#include <cassert>
#include <iostream>

class Trie {
public:
    Trie() : root_(new Node()) {}

    // The trie owns a tree of raw Node pointers. A shallow copy would leave two
    // Tries pointing at the same nodes, and both destructors would delete them
    // -> double-free. We forbid copying and provide move instead; moving just
    // transfers the single root pointer and leaves the source empty-but-valid.
    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;

    Trie(Trie&& other) noexcept : root_(other.root_) {
        other.root_ = new Node();   // leave the moved-from trie usable & safe to destroy
    }
    Trie& operator=(Trie&& other) noexcept {
        if (this != &other) {
            destroy(root_);
            root_ = other.root_;
            other.root_ = new Node();
        }
        return *this;
    }

    // Recursive free of the whole tree. Depth is bounded by the longest key, so
    // for realistic dictionaries the recursion stays shallow; a pathological
    // multi-thousand-character key could be freed iteratively instead.
    ~Trie() { destroy(root_); }

    // Insert a word, creating child nodes as needed. Marks the final node as a
    // word end so that search() can distinguish a stored word from a mere prefix.
    void insert(const std::string& word) {
        Node* cur = root_;
        for (char ch : word) {
            std::size_t idx = index(ch);
            if (cur->children[idx] == nullptr) {
                cur->children[idx] = new Node();
            }
            cur = cur->children[idx];
        }
        cur->isEndOfWord = true;
    }

    // Exact match: the full path must exist AND its last node must be flagged as
    // a word end. ("app" being stored does not make "ap" a stored word.)
    bool search(const std::string& word) const {
        const Node* node = findNode(word);
        return node != nullptr && node->isEndOfWord;
    }

    // Prefix match: only the path needs to exist; the end need not be a word.
    bool startsWith(const std::string& prefix) const {
        return findNode(prefix) != nullptr;
    }

    // Remove a word if present. Returns true if a word was actually removed.
    // Two-part job: (1) clear the isEndOfWord flag on the terminal node, then
    // (2) prune any nodes that are now useless -- i.e. have no children and are
    // not themselves the end of some other word. We recurse to the terminal node
    // and let the unwinding stack delete dead nodes bottom-up.
    bool remove(const std::string& word) {
        return removeHelper(root_, word, 0);
    }

    // Collect every stored word that begins with `prefix`, sorted alphabetically
    // (the 26-way array is visited in 'a'..'z' order). Handy for autocomplete.
    std::vector<std::string> wordsWithPrefix(const std::string& prefix) const {
        std::vector<std::string> out;
        const Node* start = findNode(prefix);
        if (start != nullptr) {
            std::string buffer = prefix;
            collect(start, buffer, out);
        }
        return out;
    }

private:
    struct Node {
        std::array<Node*, 26> children;  // one slot per lowercase letter
        bool isEndOfWord = false;
        Node() { children.fill(nullptr); }
    };

    Node* root_;

    // Map 'a'..'z' to 0..25. Keys are assumed lowercase per this trie's contract.
    static std::size_t index(char ch) {
        assert(ch >= 'a' && ch <= 'z' && "Trie keys must be lowercase a-z");
        return static_cast<std::size_t>(ch - 'a');
    }

    // Walk the path spelled by `key`; return the terminal node or nullptr if the
    // path breaks. Shared by search()/startsWith()/wordsWithPrefix().
    const Node* findNode(const std::string& key) const {
        const Node* cur = root_;
        for (char ch : key) {
            std::size_t idx = index(ch);
            if (cur->children[idx] == nullptr) return nullptr;
            cur = cur->children[idx];
        }
        return cur;
    }

    // Post-order recursion that unmarks the word and prunes dead nodes on the way
    // back up. Returns true if the word existed and was removed.
    bool removeHelper(Node* node, const std::string& word, std::size_t depth) {
        if (node == nullptr) return false;

        if (depth == word.size()) {
            if (!node->isEndOfWord) return false;  // path exists but not a stored word
            node->isEndOfWord = false;             // unmark it
            return true;
        }

        std::size_t idx = index(word[depth]);
        Node* child = node->children[idx];
        bool removed = removeHelper(child, word, depth + 1);

        // Prune the child if the recursion actually removed a word AND the child
        // has become useless: no descendants and not the end of another word.
        // This keeps a longer word sharing the prefix intact -- if the child is
        // still part of another word it has children (or its own flag) and stays.
        if (removed && child != nullptr && !child->isEndOfWord && !hasChildren(child)) {
            delete child;
            node->children[idx] = nullptr;
        }
        return removed;
    }

    static bool hasChildren(const Node* node) {
        for (const Node* c : node->children) {
            if (c != nullptr) return true;
        }
        return false;
    }

    // Depth-first accumulation of complete words under `node`. `buffer` holds the
    // prefix spelled so far and is mutated in place (push char, recurse, pop).
    void collect(const Node* node, std::string& buffer,
                 std::vector<std::string>& out) const {
        if (node->isEndOfWord) out.push_back(buffer);
        for (std::size_t i = 0; i < node->children.size(); ++i) {
            if (node->children[i] != nullptr) {
                buffer.push_back(static_cast<char>('a' + i));
                collect(node->children[i], buffer, out);
                buffer.pop_back();
            }
        }
    }

    // Recursively delete a subtree. Called by the destructor and move-assign.
    void destroy(Node* node) {
        if (node == nullptr) return;
        for (Node* c : node->children) destroy(c);
        delete node;
    }
};

// ---------------------------------------------------------------------------
// Tests + demo
// ---------------------------------------------------------------------------
int main() {
    // --- Edge case: empty trie ---
    {
        Trie t;
        assert(!t.search("anything"));
        assert(!t.startsWith("a"));
        assert(!t.remove("ghost"));   // removing from empty trie is a no-op
    }

    // --- Single word insert / search / prefix ---
    {
        Trie t;
        t.insert("cat");
        assert(t.search("cat"));
        assert(!t.search("ca"));      // "ca" is a prefix, not a stored word
        assert(t.startsWith("ca"));   // ...but it IS a valid prefix
        assert(t.startsWith("cat"));
        assert(!t.startsWith("cats")); // longer than anything stored
        assert(!t.search("dog"));
    }

    // --- Prefix sharing + autocomplete-style queries ---
    {
        Trie t;
        for (const std::string& w :
             {"app", "apple", "apply", "apt", "bat", "batch"}) {
            t.insert(w);
        }

        assert(t.search("app"));
        assert(t.search("apple"));
        assert(t.startsWith("ap"));
        assert(!t.search("ap"));      // prefix only

        // Autocomplete: all words starting with "app", alphabetical.
        std::vector<std::string> apps = t.wordsWithPrefix("app");
        assert((apps == std::vector<std::string>{"app", "apple", "apply"}));

        std::vector<std::string> bats = t.wordsWithPrefix("bat");
        assert((bats == std::vector<std::string>{"bat", "batch"}));

        std::vector<std::string> none = t.wordsWithPrefix("z");
        assert(none.empty());
    }

    // --- remove(): unmark + prune, without breaking a longer shared word ---
    {
        Trie t;
        t.insert("app");
        t.insert("apple");

        // Remove the SHORTER word. "apple" shares the "app" prefix and must survive.
        assert(t.remove("app"));
        assert(!t.search("app"));     // "app" is gone as a word...
        assert(t.startsWith("app"));  // ...but its path still leads to "apple"
        assert(t.search("apple"));    // the longer word is intact

        // Removing a non-existent / prefix-only word returns false, changes nothing.
        assert(!t.remove("ap"));
        assert(!t.remove("applesauce"));
        assert(t.search("apple"));

        // Remove the LONGER word; the now-dead tail nodes should be pruned, but
        // the trie must remain valid and other words unaffected.
        assert(t.remove("apple"));
        assert(!t.search("apple"));
        assert(!t.startsWith("app")); // whole "app..." branch is now dead & pruned
    }

    // --- Duplicate insert is idempotent; one remove clears it ---
    {
        Trie t;
        t.insert("hello");
        t.insert("hello");           // inserting twice does not stack
        assert(t.search("hello"));
        assert(t.remove("hello"));
        assert(!t.search("hello"));
        assert(!t.remove("hello"));  // already gone
    }

    // --- Move semantics: moved-from trie stays valid, moved-to owns the data ---
    {
        Trie a;
        a.insert("move");
        Trie b = std::move(a);
        assert(b.search("move"));
        assert(!a.search("move"));   // a is empty-but-usable after the move
        a.insert("fresh");           // still safe to use
        assert(a.search("fresh"));
    }

    // --- Human-readable demo ---
    std::cout << "Trie (prefix tree) demo\n";
    std::cout << "-----------------------\n";
    Trie dict;
    for (const std::string& w :
         {"code", "coder", "coding", "cola", "cost", "dog"}) {
        dict.insert(w);
    }
    std::cout << "Dictionary loaded with 6 words.\n";
    std::cout << "search(\"coder\")   -> " << std::boolalpha << dict.search("coder") << '\n';
    std::cout << "search(\"co\")      -> " << dict.search("co") << " (prefix, not a word)\n";
    std::cout << "startsWith(\"co\")  -> " << dict.startsWith("co") << '\n';

    std::cout << "Autocomplete \"co\": ";
    for (const std::string& w : dict.wordsWithPrefix("co")) std::cout << w << ' ';
    std::cout << '\n';

    dict.remove("coder");
    std::cout << "After remove(\"coder\"), autocomplete \"cod\": ";
    for (const std::string& w : dict.wordsWithPrefix("cod")) std::cout << w << ' ';
    std::cout << "  (\"code\" and \"coding\" remain)\n";

    std::cout << "\nAll assertions passed.\n";
    return 0;
}
