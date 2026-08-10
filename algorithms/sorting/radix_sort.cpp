/*
 * Radix Sort (LSD, base 10)  -- Algorithm - Sorting
 *
 * Idea:
 *   Sort integers digit by digit, from the Least Significant Digit to the most.
 *   Each pass distributes numbers by ONE digit using a STABLE counting sort.
 *   Stability is the load-bearing property: when we later sort by a higher digit,
 *   numbers that tie on that digit keep the order established by the lower digits,
 *   so after processing all d digits the array is fully sorted. Invariant after
 *   pass p: the array is sorted by the least significant (p+1) digits.
 *
 * Complexity:
 *   Case    | Time          | Aux Space
 *   --------+---------------+-----------
 *   Best    | O(d*(n + b))  | O(n + b)
 *   Average | O(d*(n + b))  | O(n + b)
 *   Worst   | O(d*(n + b))  | O(n + b)
 *   n = count, b = base (radix, here 10), d = number of digits in the max value
 *   (d = floor(log_b(max)) + 1). Because d and b are fixed for a given input, this
 *   is effectively linear O(n) when d is small -- that is the whole appeal.
 *
 * Properties:
 *   Stable?    yes  (each digit pass is a stable counting sort; overall order kept)
 *   In-place?  no   (each pass uses a count array of size b and an output buffer)
 *   Adaptive?  no   (always performs d passes regardless of pre-existing order)
 *
 * When to use / notes:
 *   - Fixed-width integer (or fixed-length string) keys, large n, small key width.
 *   - Base choice is a time/space knob: base 256 (radix = 2^8) processes one BYTE
 *     per pass, so a 32-bit int needs only 4 passes with a 256-slot count array --
 *     far fewer passes than base 10, at the cost of a bigger (but still O(1)) count.
 *   - Negatives: this implementation assumes NON-NEGATIVE input. To support signed
 *     values, offset every key by -min first (see radixSortSigned below), which
 *     maps the minimum to 0 and preserves relative order.
 */

#include <vector>
#include <algorithm>   // std::sort, std::max_element, std::min_element, std::is_sorted
#include <utility>     // std::pair, std::move
#include <cstddef>     // std::size_t
#include <cassert>
#include <iostream>

namespace {

constexpr int kBase = 10;  // readable base 10; see header note on base 256.

// One stable pass: sort `a` by the digit at the given place value (1, 10, 100, ...)
// using counting sort over the b=kBase possible digit values. Stability comes from
// the right-to-left scan with pre-decremented prefix positions (same trick as the
// standalone counting sort).
void countingSortByDigit(std::vector<int>& a, int place) {
    std::vector<int> output(a.size());
    int count[kBase] = {0};

    for (const int v : a) {
        const int digit = (v / place) % kBase;
        ++count[digit];
    }
    // Prefix sums -> count[d] becomes the exclusive end index of digit d's block.
    for (int d = 1; d < kBase; ++d) {
        count[d] += count[d - 1];
    }
    // Stable placement, scanning right-to-left.
    for (std::size_t i = a.size(); i-- > 0; ) {
        const int digit = (a[i] / place) % kBase;
        output[static_cast<std::size_t>(--count[digit])] = a[i];
    }
    a = std::move(output);
}

}  // namespace

// LSD radix sort for NON-NEGATIVE integers. See radixSortSigned for negatives.
void radixSort(std::vector<int>& a) {
    if (a.empty()) return;

    const int maxVal = *std::max_element(a.begin(), a.end());
    // Advance the place value by one base-digit per pass until it exceeds maxVal.
    // Guard against overflow: stop once place would grow past maxVal.
    for (int place = 1; maxVal / place > 0; place *= kBase) {
        countingSortByDigit(a, place);
    }
}

// Negatives-safe wrapper: shift the domain so the minimum becomes 0, run the
// unsigned radix sort, then shift back. Offsetting is order-preserving, so the
// final result is correctly sorted across the full signed range.
void radixSortSigned(std::vector<int>& a) {
    if (a.empty()) return;
    const int lo = *std::min_element(a.begin(), a.end());
    if (lo < 0) {
        for (int& v : a) v -= lo;   // now all keys are >= 0
        radixSort(a);
        for (int& v : a) v += lo;   // undo the offset
    } else {
        radixSort(a);
    }
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

static void print(const char* label, const std::vector<int>& v) {
    std::cout << label;
    for (const int x : v) std::cout << x << ' ';
    std::cout << '\n';
}

static void expectSorted(std::vector<int> in) {
    std::vector<int> reference = in;
    std::sort(reference.begin(), reference.end());
    radixSortSigned(in);            // exercises the general (negatives-safe) path
    assert(in == reference);
}

int main() {
    // Required edge cases -----------------------------------------------------
    expectSorted({});                                  // empty
    expectSorted({42});                                // single element
    expectSorted({1, 2, 3, 4, 5});                     // already sorted
    expectSorted({9, 7, 5, 3, 1});                     // reverse sorted
    expectSorted({7, 7, 7, 7});                        // all equal
    expectSorted({170, 45, 75, 90, 2, 802, 2, 66});    // duplicates, varying widths
    expectSorted({-5, 3, -10, 0, 7, -1, 3, 1000});     // negatives (domain shift)

    // Varying digit lengths incl. a large value: verify against std::sort.
    {
        std::vector<int> v = {3, 30, 300, 3000, 30000, 1, 21, 210, 999999};
        std::vector<int> ref = v;
        std::sort(ref.begin(), ref.end());
        radixSort(v);
        assert(v == ref);
    }

    // Stability check: each radix pass is a stable counting sort. Here we run one
    // such pass keyed on .first over single-digit keys (0..2); .second is a tie-break
    // tag that must NOT be reordered among equal keys. If the pass were unstable, the
    // tags for equal keys would come out permuted.
    {
        std::vector<std::pair<int, int>> work = {
            {2, 0}, {1, 1}, {2, 2}, {1, 3}, {0, 4}, {2, 5}
        };
        std::vector<std::pair<int, int>> output(work.size());
        int count[kBase] = {0};
        for (const auto& p : work) ++count[p.first % kBase];
        for (int d = 1; d < kBase; ++d) count[d] += count[d - 1];
        for (std::size_t i = work.size(); i-- > 0; ) {
            output[static_cast<std::size_t>(--count[work[i].first % kBase])] = work[i];
        }
        work = std::move(output);

        // Keys ascending; within each key the .second tags keep their input order
        // (key 2 -> 0,2,5 ; key 1 -> 1,3).
        assert((work == std::vector<std::pair<int, int>>{
            {0, 4}, {1, 1}, {1, 3}, {2, 0}, {2, 2}, {2, 5}}));
    }

    // Before/after demo -------------------------------------------------------
    std::vector<int> demo = {170, 45, 75, 90, 2, 802, 2, 66};
    print("before: ", demo);
    radixSort(demo);
    print("after:  ", demo);
    assert(std::is_sorted(demo.begin(), demo.end()));

    std::cout << "All radix sort tests passed.\n";
    return 0;
}
