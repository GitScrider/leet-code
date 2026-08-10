/*
 * Ternary Search on a Unimodal Function (Algorithm - Searching)
 *
 * Idea:
 *   Ternary search locates the extremum (here: the MAXIMUM) of a UNIMODAL
 *   function f over a range. Unimodal for a maximum means f strictly increases
 *   up to a single peak and then strictly decreases (no flat plateaus at the
 *   top). It does NOT search for a specific value like binary search; it hunts
 *   the peak of a "hill".
 *
 *   Each step we split the current range [lo..hi] into three parts using two
 *   probe points m1 < m2 and compare f(m1) with f(m2):
 *     - If f(m1) < f(m2): the peak cannot lie left of m1 (function is still
 *       rising there), so discard [lo..m1)  -> lo = m1 + 1.
 *     - Else (f(m1) >= f(m2)): the peak cannot lie right of m2, so discard
 *       (m2..hi]  -> hi = m2 - 1.
 *   Why it converges: every step removes about a third of the range, so the
 *   width shrinks geometrically toward the single peak and the loop ends when
 *   lo == hi. The comparison is safe because unimodality guarantees the peak is
 *   on the side of the higher probe.
 *
 *   Real-valued version (note): for a continuous unimodal f on [lo, hi] (double),
 *   loop `while (hi - lo > eps)` computing m1 = lo + (hi-lo)/3 and
 *   m2 = hi - (hi-lo)/3, moving lo or hi to the probe as above, then return the
 *   midpoint. Choose eps by required precision; ~ log_1.5((hi-lo)/eps) iterations.
 *
 * Complexity (n = number of integer points in the range):
 *   +-----------+-------------+-------------+-------------+
 *   | Case      | Best        | Average     | Worst       |
 *   +-----------+-------------+-------------+-------------+
 *   | Time      | O(log3 n)   | O(log3 n)   | O(log3 n)   |
 *   | Space     | O(1)        | O(1)        | O(1)        |
 *   +-----------+-------------+-------------+-------------+
 *   Each iteration removes ~1/3 of the range => O(log_3 n) evaluations of f.
 *   (Binary/golden-section search uses fewer f-calls per step, but ternary is
 *   the clearest to reason about.)
 *
 * Complexity derivation (recurrence -> iteration table -> summation):
 *   Let n = hi - lo + 1 be the number of integer points in the range. Each
 *   iteration evaluates f at the two probes m1, m2 and discards ONE outer third:
 *   lo = m1+1 drops [lo..m1] (~n/3 points), hi = m2-1 drops [m2..hi] (~n/3
 *   points). Hence roughly 2/3 of the range SURVIVES each step. Let T(n) count
 *   the iterations on a range of n points:
 *
 *       T(n) = T(2n/3) + c ,        T(1) = c0    (base: loop ends when lo==hi)
 *
 *   where c = 2 evaluations of f + O(1) bookkeeping. Unfold as an iteration
 *   table (only the surviving width matters):
 *
 *       iteration k    surviving size      f-evals on the step
 *       -----------    ----------------    -------------------
 *       k = 0          n                   2
 *       k = 1          (2/3) n             2
 *       k = 2          (2/3)^2 n           2
 *       ...            ...                 ...
 *       k = K          (2/3)^K n           2
 *
 *   The loop stops when (2/3)^K n <= 1, i.e. (3/2)^K >= n, i.e. K = log_{3/2} n.
 *   Summing the constant per-step work:
 *
 *       T(n) = SUM_{k=0}^{log_{3/2} n} c = c*(log_{3/2} n + 1) = O(log_{3/2} n)
 *
 *   Master Theorem (a=1, b=3/2, f=Theta(1)): n^(log_b a) = n^0 = 1 -> case 2 ->
 *   T(n) = Theta(log n). NOTE on the base: the header writes O(log3 n) for the
 *   "remove 1/3" intuition, but the true per-step factor is 3/2 because 2/3
 *   survives. That is no asymptotic difference: log_3 n, log_{3/2} n and log2 n
 *   differ only by a constant (log_b n = ln n / ln b), so O(log3 n) =
 *   O(log_{3/2} n) = O(log n) -- the same Theta class the table states.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   The loop always shrinks [lo..hi] to a single point regardless of where the
 *   peak sits, so the work is INPUT-INDEPENDENT: the count is c*(log_{3/2} n + 1)
 *   for every unimodal input. With g = log n: c1*log n <= T(n) <= c2*log n for
 *   n >= 2  =>  T(n) = Theta(log n). Hence best = average = worst = Theta(log3 n),
 *   a single tight bound (no per-case split). This is a SEARCH, not a sort, so
 *   the comparison-sort lower bound Omega(n log n) does not apply; the relevant
 *   optimum for shrinking a range is Theta(log n).
 *
 * Key points / when to use:
 *   - Requires the function to be UNIMODAL over the range (single peak/valley).
 *     If it is not, the result is meaningless.
 *   - Use for optimizing a "hill-shaped" cost/score, not for locating a value.
 *   - For a minimum of a valley-shaped function, flip the comparison sign.
 */

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

// Find the integer argmax of a unimodal function f over the inclusive range
// [lo, hi]. Returns the index (an int) of the maximizing point. Precondition:
// lo <= hi and f is unimodal (rises then falls) on [lo, hi].
template <typename F>
long ternary_search_argmax(long lo, long hi, F f) {
    // Signed longs are used deliberately so that mid computations and the
    // shrinking bounds never underflow the way std::size_t would.
    while (lo < hi) {
        // Two interior probes at the one-third and two-third marks.
        // (hi - lo)/3 is overflow-safe: it is a width, not a sum of indices.
        long m1 = lo + (hi - lo) / 3;
        long m2 = hi - (hi - lo) / 3;
        // m1 < m2 whenever hi - lo >= 2; when hi - lo == 1 they may coincide,
        // but the branch below still shrinks the range by at least one.
        if (f(m1) < f(m2)) {
            lo = m1 + 1;  // peak is to the right of m1
        } else {
            hi = m2 - 1;  // peak is at or to the left of m2
        }
    }
    return lo;  // lo == hi: the single surviving point is the peak
}

// Linear reference: brute-force argmax over the same inclusive range.
template <typename F>
long linear_argmax(long lo, long hi, F f) {
    long best = lo;
    auto bestVal = f(lo);
    for (long x = lo + 1; x <= hi; ++x) {
        auto v = f(x);
        if (v > bestVal) { bestVal = v; best = x; }
    }
    return best;
}

int main() {
    // Classic unimodal parabola with its peak at x = 7: f(x) = -(x-7)^2.
    auto parabola = [](long x) -> long {
        long d = x - 7;
        return -(d * d);
    };

    // Peak found over a range that straddles 7.
    assert(ternary_search_argmax(0, 20, parabola) == 7);
    assert(ternary_search_argmax(0, 20, parabola) == linear_argmax(0, 20, parabola));

    // Peak exactly at the left endpoint (function only falls on [7, 15]).
    assert(ternary_search_argmax(7, 15, parabola) == 7);
    // Peak exactly at the right endpoint (function only rises on [-5, 7]).
    assert(ternary_search_argmax(-5, 7, parabola) == 7);

    // single element range: the only point is the peak.
    assert(ternary_search_argmax(7, 7, parabola) == 7);
    assert(ternary_search_argmax(3, 3, parabola) == 3);

    // Two-element range (ensures the hi-lo == 1 case still terminates).
    assert(ternary_search_argmax(6, 7, parabola) == 7);
    assert(ternary_search_argmax(7, 8, parabola) == 7);

    // Cross-check against the linear reference on several random-ish windows.
    for (long lo = -10; lo <= 5; ++lo)
        for (long hi = lo; hi <= 25; ++hi)
            assert(ternary_search_argmax(lo, hi, parabola) ==
                   linear_argmax(lo, hi, parabola));

    // A different unimodal shape whose peak is not at an integer of the formula:
    // f(x) = 100 - |x - 3|, peak at x = 3.
    auto tent = [](long x) -> long { long d = x - 3; return 100 - (d < 0 ? -d : d); };
    assert(ternary_search_argmax(-50, 50, tent) == 3);

    // Short demo.
    std::cout << "ternary_search_argmax demo on f(x) = -(x-7)^2\n";
    long peak = ternary_search_argmax(0, 20, parabola);
    std::cout << "  peak at x = " << peak << ", f(x) = " << parabola(peak) << "\n";
    std::cout << "  (real-valued variant: loop while hi-lo > eps, see header)\n";

    std::cout << "All assertions passed.\n";
    return 0;
}
