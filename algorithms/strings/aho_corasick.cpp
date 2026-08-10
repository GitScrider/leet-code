/*
 * Aho-Corasick Multi-Pattern Search  (Algorithm - Strings)
 * -----------------------------------------------------------------------------
 * Problem : Given a SET of patterns, find every occurrence of every pattern in
 *           a text with ONE left-to-right scan of the text.
 *
 * Idea    : Build a trie of all patterns, then turn it into a finite automaton.
 *
 *           FAILURE LINK  fail(v): the node for the LONGEST proper suffix of the
 *           string spelled by v that is also a prefix of some pattern (i.e. a
 *           trie node). It is the string analogue of the KMP failure function.
 *           When the current character has no edge out of v we follow fail(v)
 *           repeatedly instead of restarting -- so the text is never re-scanned.
 *           Failure links are computed by BFS, because fail(child) only depends
 *           on fail(parent), which is shallower and already known.
 *
 *           OUTPUT (dictionary-suffix) LINK: the nearest node reachable via
 *           failure links that is itself the end of a pattern. Walking this
 *           chain at each text position emits nested/overlapping matches such
 *           as "he" inside "she", or "a","aa","aaa" all ending together.
 *
 *   Complexity (n = |text|, m = sum of pattern lengths, z = # matches reported)
 *   +---------------------+---------------------+----------------------------+
 *   | Step                | Aho-Corasick        | Naive (search each pattern)|
 *   +---------------------+---------------------+----------------------------+
 *   | Build automaton     | O(m)                | --                         |
 *   | Search text         | O(n + z)            | O(n * m)                   |
 *   | Space               | O(m)                | O(1)                       |
 *   +---------------------+---------------------+----------------------------+
 *
 * Complexity derivation (BFS build O(m); amortized KMP-style scan O(n + z)):
 *   Let V be the number of trie nodes. Inserting the patterns creates at most one
 *   node per pattern character, so V <= m + 1 = O(m), and the number of trie edges
 *   is V - 1 = O(m).
 *
 *   BUILD (BFS). The BFS dequeues each node once and iterates its outgoing edges,
 *   so the edge-iteration total over all nodes is (V - 1) = O(m). Computing
 *   fail(v) walks the parent's failure chain; each while step moves to a STRICTLY
 *   shallower node (fail always decreases trie depth), while a node's fail-depth
 *   exceeds its parent's fail-depth by at most 1. This is the KMP potential
 *   argument: summed over every node the while-step count telescopes to
 *
 *       SUM_{v} (steps to find fail(v)) <= SUM_{v} 1 = O(V) = O(m)
 *
 *   so BUILD = O(m) edge scans + O(m) fail-walk = O(m). (unordered_map find is
 *   O(1) average, so this is O(m) EXPECTED; a collision-heavy alphabet degrades
 *   each lookup toward O(degree).)
 *
 *   SEARCH. The outer loop runs n = |text| times, in two amortized parts:
 *     (a) transitions: the inner while follows failure links. Each position adds
 *         at most 1 to the current depth (a single forward edge step), so depth
 *         rises by <= n in total; each while iteration drops depth by >= 1, hence
 *             SUM_{i=0}^{n-1} (fail steps at i) <= n  = O(n).
 *     (b) reporting: the output-link chain visits ONLY pattern-ending nodes (after
 *         cur's own single visit), and every such node emits >= 1 match, so
 *             SUM_{i=0}^{n-1} (chain work at i) = O(n + z),   z = # matches.
 *   Total SEARCH = O(n) + O(n + z) = O(n + z).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f = O(g)      iff  EXISTS c2, n0 :       f <= c2*g(n)  for input size >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f        for input size >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   BUILD: work is proportional to the trie size, c1*m <= f(m) <= c2*m, so it is
 *     Theta(m) -- input-independent given the pattern set (best = worst).
 *   SEARCH is OUTPUT-SENSITIVE, so cost splits into scan and reporting:
 *     BEST case  (no pattern occurs, z = 0): exactly n transitions => Theta(n).
 *     WORST case (dense overlaps, e.g. patterns a, aa, ..., a^k on text a^n):
 *       z reaches Theta(n*k) and dominates => Theta(n + z).
 *     Hence SEARCH = O(n + z) (upper) and Omega(n) (lower, the one mandatory scan);
 *     it is Theta(n + z) only once z is counted as an input parameter, NOT a single
 *     Theta(n), because z is unbounded in n alone.
 *   This is pattern matching, NOT sorting, so the comparison-sort Omega(n log n)
 *   lower bound does NOT apply: the automaton reads each text symbol O(1) times
 *   amortized, beating the O(n*m) naive per-pattern re-scan.
 *
 * Key points:
 *   - One text scan finds all patterns at once; failure links replace restarts.
 *   - Output links make reporting O(1) per actual match, so total is O(n + z).
 *   - Nodes live in a std::vector (indices, not pointers): no manual new/delete,
 *     so growing the trie never dangles a reference and cleanup is automatic.
 *   - Convention: patterns are assumed non-empty (empty patterns are ignored).
 */

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <cassert>
#include <iostream>
#include <cstddef>

class AhoCorasick {
public:
    AhoCorasick() { nodes_.emplace_back(); }   // node 0 is the root

    // Insert a pattern; its id is its insertion order (0, 1, 2, ...).
    void addPattern(const std::string& pattern) {
        int cur = 0;
        for (char c : pattern) {
            auto it = nodes_[cur].next.find(c);
            if (it == nodes_[cur].next.end()) {
                const int nxt = static_cast<int>(nodes_.size());
                nodes_.emplace_back();
                nodes_[cur].next[c] = nxt;      // safe: we hold indices, not pointers
                cur = nxt;
            } else {
                cur = it->second;
            }
        }
        const int id = static_cast<int>(patternLength_.size());
        patternLength_.push_back(pattern.size());
        nodes_[cur].patternIds.push_back(id);
    }

    // BFS to fill failure links and output links.
    void build() {
        std::queue<int> q;
        for (const auto& kv : nodes_[0].next) {  // depth-1 nodes fail back to root
            nodes_[kv.second].fail = 0;
            nodes_[kv.second].outputLink = -1;
            q.push(kv.second);
        }
        while (!q.empty()) {
            const int u = q.front(); q.pop();
            for (const auto& kv : nodes_[u].next) {
                const char c = kv.first;
                const int v = kv.second;

                // Follow u's failure chain to find where c leads.
                int f = nodes_[u].fail;
                while (f != 0 && nodes_[f].next.find(c) == nodes_[f].next.end())
                    f = nodes_[f].fail;
                auto it = nodes_[f].next.find(c);
                nodes_[v].fail = (it != nodes_[f].next.end() && it->second != v)
                                 ? it->second : 0;

                // Output link: jump straight to the nearest suffix that is a pattern.
                const int fv = nodes_[v].fail;
                nodes_[v].outputLink = nodes_[fv].patternIds.empty()
                                       ? nodes_[fv].outputLink : fv;
                q.push(v);
            }
        }
    }

    // Scan the text once; return, per pattern id, its sorted list of start offsets.
    std::vector<std::vector<std::size_t>> search(const std::string& text) const {
        std::vector<std::vector<std::size_t>> result(patternLength_.size());
        int cur = 0;
        for (std::size_t i = 0; i < text.size(); ++i) {
            const char c = text[i];
            while (cur != 0 && nodes_[cur].next.find(c) == nodes_[cur].next.end())
                cur = nodes_[cur].fail;                 // no edge: follow failure link
            auto it = nodes_[cur].next.find(c);
            cur = (it != nodes_[cur].next.end()) ? it->second : 0;

            // Emit cur's own matches, then every match along the output-link chain.
            for (int t = cur; t != 0 && t != -1; t = nodes_[t].outputLink)
                for (int id : nodes_[t].patternIds)
                    result[id].push_back(i + 1 - patternLength_[id]);  // start = end-len+1
        }
        return result;
    }

private:
    struct Node {
        std::unordered_map<char, int> next;   // trie edges
        int fail = 0;                          // failure link
        int outputLink = -1;                   // nearest pattern-ending suffix (or -1)
        std::vector<int> patternIds;           // patterns ending exactly at this node
    };
    std::vector<Node> nodes_;
    std::vector<std::size_t> patternLength_;
};

// ---- Naive per-pattern search used as the test oracle. ----
static std::vector<std::vector<std::size_t>> naiveMultiSearch(
        const std::string& text, const std::vector<std::string>& patterns) {
    std::vector<std::vector<std::size_t>> result(patterns.size());
    for (std::size_t p = 0; p < patterns.size(); ++p) {
        const std::string& pat = patterns[p];
        if (pat.empty()) continue;
        const std::size_t n = text.size(), m = pat.size();
        for (std::size_t i = 0; i + m <= n; ++i) {
            std::size_t j = 0;
            while (j < m && text[i + j] == pat[j]) ++j;
            if (j == m) result[p].push_back(i);
        }
    }
    return result;
}

static std::vector<std::vector<std::size_t>> runAho(
        const std::vector<std::string>& patterns, const std::string& text) {
    AhoCorasick ac;
    for (const std::string& p : patterns) ac.addPattern(p);
    ac.build();
    return ac.search(text);
}

int main() {
    struct Case { std::vector<std::string> patterns; std::string text; };
    const std::vector<Case> cases = {
        {{"he", "she", "his", "hers"}, "ushers"},          // classic overlaps
        {{"he", "she", "his", "hers"}, "ahishershe"},      // more overlaps
        {{"a", "aa", "aaa"},           "aaaa"},            // nested, overlapping
        {{"ab", "bc", "abc", "c"},     "xabcabcy"},        // shared substrings
        {{"cat", "dog"},               "birdfishcatdog"},  // disjoint patterns
        {{"needle"},                   "haystack"},        // no match
        {{"abc"},                      "ab"},              // pattern longer than text
        {{"aba", "ab"},                "ababab"},          // heavy overlap
    };

    for (const Case& c : cases) {
        std::vector<std::vector<std::size_t>> got = runAho(c.patterns, c.text);
        std::vector<std::vector<std::size_t>> exp = naiveMultiSearch(c.text, c.patterns);
        assert(got == exp);
    }

    // Exact expected positions for the textbook {he,she,his,hers} on "ushers".
    {
        std::vector<std::vector<std::size_t>> r =
            runAho({"he", "she", "his", "hers"}, "ushers");
        assert((r[0] == std::vector<std::size_t>{2}));   // he
        assert((r[1] == std::vector<std::size_t>{1}));   // she
        assert(r[2].empty());                            // his (absent)
        assert((r[3] == std::vector<std::size_t>{2}));   // hers
    }

    std::cout << "Aho-Corasick demo: {he, she, his, hers} in \"ushers\"\n";
    const std::vector<std::string> pats = {"he", "she", "his", "hers"};
    std::vector<std::vector<std::size_t>> r = runAho(pats, "ushers");
    for (std::size_t p = 0; p < pats.size(); ++p) {
        std::cout << "  \"" << pats[p] << "\" at:";
        for (std::size_t pos : r[p]) std::cout << ' ' << pos;
        std::cout << '\n';
    }
    std::cout << "All Aho-Corasick assertions passed.\n";
    return 0;
}
