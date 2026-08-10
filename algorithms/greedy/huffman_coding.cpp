/*
 * Huffman Coding  (Algorithm - Greedy)
 * ------------------------------------------------------------------
 * Problem:
 *   Given an alphabet of symbols with frequencies, build an optimal
 *   binary PREFIX code (no codeword is a prefix of another) that
 *   minimizes the total encoded length  sum(freq[s] * codelen[s]).
 *
 * Idea (the greedy choice and WHY it is safe):
 *   Greedy choice: repeatedly MERGE the two LEAST-FREQUENT nodes into a
 *   new internal node whose frequency is their sum, using a min-heap.
 *   The two rarest symbols become siblings at the deepest level.
 *
 *   Exchange argument (why it is safe):
 *     Let x, y be the two lowest-frequency symbols. In an optimal prefix
 *     tree the two deepest leaves are siblings (else we could shorten one
 *     for free). Swapping those deepest leaves with x and y cannot increase
 *     the cost, because x and y have the smallest frequencies and moving a
 *     smaller frequency to a deeper spot never costs more. So some optimal
 *     tree makes x and y sibling leaves. Replacing {x,y} by a merged symbol
 *     of frequency f[x]+f[y] gives a strictly smaller instance with the same
 *     optimal structure; induction yields overall optimality.
 *     (Greedy-choice property + optimal substructure.)
 *
 * Complexity:
 *   +----------------------------+------------------+
 *   | Step                       | Cost             |
 *   +----------------------------+------------------+
 *   | Build heap of n leaves     | O(n)             |
 *   | n-1 merges (2 pop, 1 push) | O(n log n)       |
 *   | Emit codes via DFS         | O(n)             |
 *   +----------------------------+------------------+
 *   Total: O(n log n), dominated by the heap operations.
 *
 * Complexity derivation (heap build + n-1 merges + DFS):
 *   Let n be the number of distinct symbols and C(n) the total work.
 *     Phase 1 -- populate the min-heap. The code inserts the n leaves one at a
 *     time; insertion i sifts up through a heap of size i, costing O(log i), so
 *     building by incremental pushes is
 *         B(n) = SUM_{i=1}^{n} c*log2 i  <=  c*n*log2 n  =  O(n log n).
 *     (A bottom-up heapify would be O(n) -- the table's figure -- but even the
 *     slower incremental build cannot raise the total, since Phase 2 is already
 *     O(n log n).)
 *     Phase 2 -- the greedy merges. Each merge removes 2 nodes and adds 1, so
 *     the loop runs exactly n-1 times. Merge j does two pops and one push on a
 *     heap of size <= n, each a sift costing O(log n):
 *         M(n) = SUM_{j=1}^{n-1} 3*c*log2 n = 3c*(n-1)*log2 n = O(n log n).
 *     Phase 3 -- emit codes by DFS. The finished tree has n leaves and n-1
 *     internal nodes = 2n-1 nodes; the walk visits each once with O(1) routing:
 *         D(n) = SUM_{node} c1  =  c1*(2n-1)  =  O(n).
 *   Adding the phases:
 *         C(n) = O(n log n) + O(n log n) + O(n)  =  O(n log n),
 *   dominated by the n-1 heap-driven merges, as the table states.
 *   (Note: the separate totalBits tally re-looks-up each symbol's frequency by
 *   a linear scan of `freqs`, an O(n^2) reporting convenience OUTSIDE the code
 *   construction; a hash-map lookup restores the O(n log n) intrinsic to
 *   Huffman. DFS string concatenations are likewise ignored, as is customary.)
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The merge phase performs 3(n-1) = Theta(n) heap operations; for at least
 *   the first n/2 merges the heap still holds Theta(n) nodes, so each min-
 *   extraction sift costs up to Theta(log n) -> the merge phase is Omega(n log
 *   n) as well as O(n log n) -> Theta(n log n); the O(n) build and DFS cannot
 *   lower this. The frequency VALUES never remove the heap work, so with
 *   g(n) = n log n:
 *     upper  O:     C(n) <= c2 * (n log2 n)  for n >= 2  => O(n log n)
 *     lower  Omega: C(n) >= c1 * (n log2 n)  for n >= 2  => Omega(n log n)
 *     tight  Theta: both hold                            => Theta(n log n)
 *   Best = average = worst = Theta(n log n): the cost is fixed by n alone, not
 *   by the frequencies, so no per-case split is needed. This is a HEAP /
 *   priority-queue bound, NOT a comparison-sort one -- the Omega(n log n)
 *   sorting lower bound does not apply (the symbols are never fully sorted),
 *   yet the per-operation log n heap cost yields the same order anyway.
 *
 * Key points:
 *   - Min-heap ordered by frequency; a stable tie-break (insertion order)
 *     makes the tree deterministic. Ties never change the OPTIMAL total.
 *   - Symbols live only at LEAVES => the code is automatically prefix-free;
 *     the assert re-verifies this invariant.
 *   - Greedy is NOT optimal if a fixed-length code is required, or if
 *     codeword-length limits are imposed (that is length-limited Huffman,
 *     solved by the package-merge algorithm instead).
 *   - A single distinct symbol still needs 1 bit by convention.
 */

#include <vector>
#include <queue>
#include <string>
#include <memory>
#include <utility>
#include <cstddef>
#include <iostream>
#include <cassert>

struct Node {
    std::size_t freq;
    char        symbol;   // meaningful only for leaves
    bool        leaf;
    Node*       left;
    Node*       right;
    std::size_t order;    // insertion index: deterministic tie-break
};

// Min-heap comparator: smaller freq first; earlier insertion wins ties.
struct Compare {
    bool operator()(const Node* a, const Node* b) const {
        if (a->freq != b->freq) return a->freq > b->freq;
        return a->order > b->order;
    }
};

struct HuffmanResult {
    std::vector<std::pair<char, std::string>> codes;  // symbol -> bit string
    std::size_t totalBits;                            // weighted code length
};

// Depth-first walk that emits a bit string per leaf ('0' = left, '1' = right).
static void collectCodes(const Node* node, const std::string& prefix,
                         std::vector<std::pair<char, std::string>>& out) {
    if (node->leaf) {
        // Convention: a lone symbol (empty prefix) still gets a 1-bit code.
        out.emplace_back(node->symbol, prefix.empty() ? std::string("0") : prefix);
        return;
    }
    collectCodes(node->left, prefix + '0', out);
    collectCodes(node->right, prefix + '1', out);
}

// Build the optimal prefix code. All nodes are owned by a local arena of
// unique_ptr, so the whole tree is freed automatically on return.
HuffmanResult buildHuffman(const std::vector<std::pair<char, std::size_t>>& freqs) {
    std::vector<std::unique_ptr<Node>> arena;
    std::size_t order = 0;
    auto makeNode = [&](std::size_t f, char c, bool isLeaf,
                        Node* l, Node* r) -> Node* {
        arena.push_back(std::unique_ptr<Node>(new Node{f, c, isLeaf, l, r, order++}));
        return arena.back().get();
    };

    HuffmanResult result{{}, 0};
    if (freqs.empty()) return result;

    std::priority_queue<Node*, std::vector<Node*>, Compare> pq;
    for (const auto& kv : freqs)
        pq.push(makeNode(kv.second, kv.first, true, nullptr, nullptr));

    // Greedy merges: combine the two least-frequent subtrees until one root.
    while (pq.size() > 1) {
        Node* a = pq.top(); pq.pop();
        Node* b = pq.top(); pq.pop();
        pq.push(makeNode(a->freq + b->freq, '\0', false, a, b));
    }

    const Node* root = pq.top();
    collectCodes(root, std::string(), result.codes);

    // Total encoded length = sum over symbols of freq * codeword length.
    for (const auto& code : result.codes) {
        std::size_t f = 0;
        for (const auto& kv : freqs)
            if (kv.first == code.first) { f = kv.second; break; }
        result.totalBits += f * code.second.size();
    }
    return result;
}

// Verify no codeword is a prefix of another (the defining prefix-code property).
bool isPrefixFree(const std::vector<std::pair<char, std::string>>& codes) {
    for (std::size_t i = 0; i < codes.size(); ++i) {
        for (std::size_t j = 0; j < codes.size(); ++j) {
            if (i == j) continue;
            const std::string& a = codes[i].second;
            const std::string& b = codes[j].second;
            if (a.size() <= b.size() && b.compare(0, a.size(), a) == 0)
                return false;  // a is a prefix of b
        }
    }
    return true;
}

int main() {
    // Classic CLRS instance. Known optimum total = 224 bits.
    //   a:0 (1), b:101 (3), c:100 (3), d:111 (3), e:1101 (4), f:1100 (4)
    //   45*1 + 13*3 + 12*3 + 16*3 + 9*4 + 5*4 = 224.
    std::vector<std::pair<char, std::size_t>> freqs = {
        {'a', 45}, {'b', 13}, {'c', 12}, {'d', 16}, {'e', 9}, {'f', 5}
    };
    HuffmanResult r = buildHuffman(freqs);

    assert(r.totalBits == 224);          // greedy result equals known optimum
    assert(isPrefixFree(r.codes));       // code is a valid prefix code
    assert(r.codes.size() == freqs.size());

    // Edge / small cases with hand-computed optima.
    {
        // Two equal symbols: each gets 1 bit -> total 2.
        std::vector<std::pair<char, std::size_t>> two = {{'x', 1}, {'y', 1}};
        HuffmanResult t = buildHuffman(two);
        assert(t.totalBits == 2);
        assert(isPrefixFree(t.codes));

        // a:1,b:1,c:2 -> merge a+b=2, then +c=4; lengths c=1,a=2,b=2.
        // weighted length = a:1*2 + b:1*2 + c:2*1 = 6 (equals sum of internal
        // node weights 2+4). Do not confuse this with the root frequency (4).
        std::vector<std::pair<char, std::size_t>> three = {{'a',1},{'b',1},{'c',2}};
        HuffmanResult u = buildHuffman(three);
        assert(u.totalBits == 6);
        assert(isPrefixFree(u.codes));

        // Single symbol: 1-bit code by convention -> total = freq.
        std::vector<std::pair<char, std::size_t>> one = {{'z', 5}};
        HuffmanResult v = buildHuffman(one);
        assert(v.totalBits == 5);
        assert(v.codes.size() == 1 && v.codes[0].second.size() == 1);

        // Empty alphabet -> no codes, zero bits.
        std::vector<std::pair<char, std::size_t>> none;
        HuffmanResult w = buildHuffman(none);
        assert(w.codes.empty() && w.totalBits == 0);
    }

    // Short demo.
    std::cout << "Huffman Coding (greedy min-heap merges)\n";
    std::cout << "CLRS instance optimal total = " << r.totalBits
              << " bits (expected 224)\n";
    for (const auto& code : r.codes)
        std::cout << "  " << code.first << " : " << code.second << "\n";
    std::cout << "All assertions passed.\n";
    return 0;
}
