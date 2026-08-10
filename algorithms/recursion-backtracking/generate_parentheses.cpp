/*
 * Generate Parentheses (Algorithm - Recursion/Backtracking)
 * ---------------------------------------------------------
 * Problem: Given n, generate all well-formed (balanced) strings that use
 *          exactly n pairs of parentheses, i.e. n '(' and n ')'.
 *
 * Idea (choose / explore / unchoose backtracking):
 *   Build the string one character at a time, tracking how many '(' we have
 *   opened and how many ')' we have closed. Two rules keep every prefix legal:
 *     - We may add '(' as long as opens < n            (still have opens left).
 *     - We may add ')' only while closes < opens        (never close what was
 *                                                        not opened -> balance).
 *   When the string reaches length 2n it is a complete valid combination.
 *   The template at each step is: append a char (CHOOSE), recurse (EXPLORE),
 *   pop_back the char (UNCHOOSE / backtrack) so the buffer is reused.
 *
 * Complexity:
 *   +-----------+---------------------------------------------------------+
 *   | Time      | O(4^n / sqrt(n)) ~ the nth Catalan number of leaves,    |
 *   |           | each built in O(n). It is exponential because the       |
 *   |           | number of valid strings itself grows exponentially;     |
 *   |           | the closes < opens pruning discards illegal branches    |
 *   |           | early so we ONLY ever walk valid prefixes (huge cut).    |
 *   | Space     | O(n) recursion depth + O(n) buffer (output not counted). |
 *   +-----------+---------------------------------------------------------+
 *
 * Key points / when to use:
 *   - Classic "generate all structurally valid sequences" backtracking.
 *   - Count of results equals the nth Catalan number C(n) = (2n)! / (n!(n+1)!).
 *   - Pruning on the two counters means we never generate then reject: every
 *     leaf reached is already balanced (no post-hoc validity filter needed).
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

// Recursive backtracking generator.
// buf   : current partial string, mutated in place (choose/unchoose).
// opens : number of '(' placed so far.
// closes: number of ')' placed so far.
// n     : target number of pairs.
static void generate(std::string &buf, std::size_t opens, std::size_t closes,
                     std::size_t n, std::vector<std::string> &out) {
    // Base case: used all n opens and n closes -> a complete valid string.
    if (buf.size() == 2 * n) {
        out.push_back(buf);
        return;
    }

    // Choice 1: place an opening bracket while we still have opens available.
    if (opens < n) {
        buf.push_back('(');              // CHOOSE
        generate(buf, opens + 1, closes, n, out); // EXPLORE
        buf.pop_back();                  // UNCHOOSE (backtrack)
    }

    // Choice 2: place a closing bracket only if it keeps the prefix balanced,
    // i.e. there is an unmatched '(' to close (closes < opens).
    if (closes < opens) {
        buf.push_back(')');              // CHOOSE
        generate(buf, opens, closes + 1, n, out); // EXPLORE
        buf.pop_back();                  // UNCHOOSE (backtrack)
    }
}

// Public entry point: returns all well-formed strings of n pairs.
static std::vector<std::string> generateParentheses(std::size_t n) {
    std::vector<std::string> out;
    std::string buf;
    buf.reserve(2 * n);
    generate(buf, 0, 0, n, out);
    return out;
}

// Helper for tests: is a parentheses string balanced?
static bool isBalanced(const std::string &s) {
    long long bal = 0; // running balance; must never dip below zero
    for (char c : s) {
        if (c == '(')
            ++bal;
        else if (c == ')') {
            --bal;
            if (bal < 0) return false; // a ')' with no matching '('
        } else {
            return false;              // unexpected character
        }
    }
    return bal == 0;                   // every '(' matched by a ')'
}

// nth Catalan number computed iteratively (exact for the small n we test).
static unsigned long long catalan(std::size_t n) {
    // C(0)=1, C(k+1) = C(k) * 2(2k+1) / (k+2).
    unsigned long long c = 1;
    for (std::size_t k = 0; k < n; ++k)
        c = c * 2ULL * (2ULL * k + 1ULL) / (k + 2ULL);
    return c;
}

int main() {
    // --- Assert result counts equal the Catalan numbers ---
    // C(0)=1, C(1)=1, C(2)=2, C(3)=5, C(4)=14, C(5)=42.
    for (std::size_t n = 0; n <= 5; ++n) {
        std::vector<std::string> res = generateParentheses(n);
        assert(res.size() == catalan(n));

        // Every produced string must be balanced and of the right length.
        for (const std::string &s : res) {
            assert(s.size() == 2 * n);
            assert(isBalanced(s));
        }
    }

    // Spot-check a known small answer set (n = 3 -> the 5 Catalan strings).
    {
        std::vector<std::string> res = generateParentheses(3);
        assert(res.size() == 5);
        // Because opens are always tried before closes, the output is emitted
        // in a fixed lexical order; verify the exact set/order.
        std::vector<std::string> expected = {"((()))", "(()())", "(())()",
                                             "()(())", "()()()"};
        assert(res == expected);
    }

    // --- Short std::cout demo ---
    std::cout << "Well-formed parentheses for n = 3:\n";
    for (const std::string &s : generateParentheses(3))
        std::cout << "  " << s << '\n';

    std::cout << "\nCounts match Catalan numbers C(0..5): ";
    for (std::size_t n = 0; n <= 5; ++n)
        std::cout << generateParentheses(n).size()
                  << (n == 5 ? '\n' : ' ');

    std::cout << "All assertions passed.\n";
    return 0;
}
