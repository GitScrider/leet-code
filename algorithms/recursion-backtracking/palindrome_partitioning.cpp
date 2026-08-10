/*
 * Palindrome Partitioning (Algorithm - Recursion/Backtracking)
 * ------------------------------------------------------------
 * Problem: Given a string s, partition it into contiguous substrings so that
 *          EVERY substring is a palindrome. Return all such partitions.
 *
 * Idea (choose / explore / unchoose backtracking):
 *   Walk a cut position `start` through the string. At each step try every
 *   possible next piece s[start..end]. If that piece is a palindrome we CHOOSE
 *   it (push onto the current partition), recurse on the remainder starting at
 *   end+1 (EXPLORE), then pop it (UNCHOOSE / backtrack) to try a longer piece.
 *   When start reaches the end of the string, the whole string has been split
 *   into palindromes and we record a copy of the current partition.
 *
 *   Recurrence: partitions(start) = for each end >= start with
 *               s[start..end] palindromic: { s[start..end] } + partitions(end+1)
 *
 * Complexity:
 *   +-----------+---------------------------------------------------------+
 *   | Time      | O(n * 2^(n-1)) worst case. There are up to 2^(n-1) ways  |
 *   |           | to place cuts between characters, and copying / checking |
 *   |           | costs O(n) per complete partition. Exponential because   |
 *   |           | the number of valid partitions is itself exponential     |
 *   |           | (e.g. "aaaa..."). The isPalindrome test PRUNES any       |
 *   |           | branch whose leading piece is not a palindrome, so we    |
 *   |           | skip whole subtrees instead of exploring all cuts.       |
 *   | Space     | O(n) recursion depth + O(n) current partition            |
 *   |           | (output list not counted).                              |
 *   +-----------+---------------------------------------------------------+
 *
 * Key points / when to use:
 *   - Textbook "partition into valid pieces" backtracking (also: word break,
 *     restore IP addresses share this shape).
 *   - The palindrome check is the pruning condition: only palindromic prefixes
 *     open a branch, everything else is cut off immediately.
 *   - Pass the accumulator (current partition) by reference and backtrack it;
 *     copy it only at a complete leaf.
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

// Is the closed range s[lo..hi] a palindrome? Two-pointer scan, no allocation.
static bool isPalindrome(const std::string &s, std::size_t lo, std::size_t hi) {
    while (lo < hi) {
        if (s[lo] != s[hi]) return false;
        ++lo;
        --hi; // safe: loop guard lo < hi means hi > 0 here
    }
    return true;
}

// Backtracking core.
// start : index where the next palindromic piece must begin.
// cur   : palindromic pieces chosen so far (mutated in place).
static void partition(const std::string &s, std::size_t start,
                      std::vector<std::string> &cur,
                      std::vector<std::vector<std::string>> &out) {
    // Base case: consumed the whole string -> `cur` is a complete partition.
    if (start == s.size()) {
        out.push_back(cur); // copy the current partition into results
        return;
    }

    // Try every end index; s[start..end] is a candidate leading piece.
    for (std::size_t end = start; end < s.size(); ++end) {
        // Pruning: only recurse when the leading piece is itself a palindrome.
        if (!isPalindrome(s, start, end)) continue;

        cur.push_back(s.substr(start, end - start + 1)); // CHOOSE the piece
        partition(s, end + 1, cur, out);                 // EXPLORE remainder
        cur.pop_back();                                  // UNCHOOSE (backtrack)
    }
}

// Public entry point.
static std::vector<std::vector<std::string>>
palindromePartition(const std::string &s) {
    std::vector<std::vector<std::string>> out;
    std::vector<std::string> cur;
    partition(s, 0, cur, out);
    return out;
}

int main() {
    // --- Assert on the classic small example "aab" ---
    // Valid partitions: ["a","a","b"] and ["aa","b"]  -> exactly 2.
    {
        auto res = palindromePartition("aab");
        assert(res.size() == 2);

        std::vector<std::vector<std::string>> expected = {
            {"a", "a", "b"}, {"aa", "b"}};
        assert(res == expected);

        // Every piece in every partition must be a palindrome, and pieces must
        // concatenate back to the original string.
        for (const auto &part : res) {
            std::string joined;
            for (const std::string &piece : part) {
                assert(isPalindrome(piece, 0, piece.size() - 1));
                joined += piece;
            }
            assert(joined == "aab");
        }
    }

    // --- A string of all equal chars maximises partitions (2^(n-1)) ---
    // "aaaa" -> 2^3 = 8 partitions.
    {
        auto res = palindromePartition("aaaa");
        assert(res.size() == 8);
        for (const auto &part : res)
            for (const std::string &piece : part)
                assert(isPalindrome(piece, 0, piece.size() - 1));
    }

    // --- A string with no repeats -> only the single-character partition ---
    // "abc" -> exactly 1 partition: ["a","b","c"].
    {
        auto res = palindromePartition("abc");
        assert(res.size() == 1);
        assert((res[0] == std::vector<std::string>{"a", "b", "c"}));
    }

    // Edge case: empty string -> one partition, the empty list.
    {
        auto res = palindromePartition("");
        assert(res.size() == 1);
        assert(res[0].empty());
    }

    // --- Short std::cout demo ---
    std::cout << "Palindrome partitions of \"aab\":\n";
    for (const auto &part : palindromePartition("aab")) {
        std::cout << "  [";
        for (std::size_t i = 0; i < part.size(); ++i)
            std::cout << part[i] << (i + 1 < part.size() ? " " : "");
        std::cout << "]\n";
    }

    std::cout << "All assertions passed.\n";
    return 0;
}
