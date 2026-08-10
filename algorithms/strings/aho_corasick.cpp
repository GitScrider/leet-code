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
