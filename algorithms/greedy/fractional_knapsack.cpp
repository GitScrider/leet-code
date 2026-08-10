/*
 * Fractional Knapsack  (Algorithm - Greedy)
 * ------------------------------------------------------------------
 * Problem:
 *   Given n items, each with a value v[i] and weight w[i], and a knapsack
 *   of capacity W, maximize the total value carried. Unlike 0/1 knapsack,
 *   we may take a FRACTION of any item (divisible goods, e.g. gold dust).
 *
 * Idea (the greedy choice and WHY it is safe):
 *   Greedy choice: take items in DECREASING order of value density
 *   (ratio = value / weight). Fill greedily; when the next item does not
 *   fit whole, take exactly the fraction that fills the remaining capacity.
 *
 *   Exchange argument (why it is safe):
 *     Consider any optimal solution. Suppose it takes some amount of a
 *     lower-density item while leaving spare capacity that a higher-density
 *     item could still occupy. Swap an epsilon of weight from the low-
 *     density item to the high-density one: total weight is unchanged but
 *     total value strictly increases (higher value per unit weight), so the
 *     original was not optimal -- contradiction. Repeating the swaps drives
 *     any solution toward the density-sorted greedy fill, which is therefore
 *     optimal. (This works ONLY because items are divisible.)
 *
 * Complexity:
 *   +-----------------------+------------------+
 *   | Step                  | Cost             |
 *   +-----------------------+------------------+
 *   | Sort by density DESC  | O(n log n)       |
 *   | Greedy fill scan      | O(n)             |
 *   | Extra space           | O(n)             |
 *   +-----------------------+------------------+
 *   Total: O(n log n), dominated by the sort.
 *
 * Complexity derivation (sort + linear fill scan):
 *   Two phases whose costs add. Let C(n) be the total work.
 *     Phase 1 -- sort items by value density. std::sort (introsort) is a
 *     comparison sort with a GUARANTEED O(n log n) worst case, performing
 *     Theta(n log n) comparisons of the cross-multiplied ratios v_a*w_b vs
 *     v_b*w_a.
 *     Phase 2 -- one greedy fill scan. Each iteration does O(1) work (one
 *     capacity test plus a constant-time whole-or-fraction update). The loop
 *     may early-exit once the sack is full, so the scan alone costs
 *         F(n) = SUM_{i=0}^{k-1} c1  =  c1 * k,   1 <= k <= n,
 *     i.e. between O(1) (first item overflows) and O(n) (every item fits).
 *   Adding the phases (using the worst-case k = n):
 *         C(n) = (c*n*log2 n) + (c1*n) = O(n log n) + O(n) = O(n log n),
 *   since n log n dominates n for n >= 2. The sort is the bottleneck; the
 *   at-most-one fractional split is a single O(1) event, not a loop.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Even though the fill scan is adaptive (best Theta(1) via early break,
 *   worst Theta(n)), it is always dominated by the mandatory density sort,
 *   which fixes the order. A comparison sort must separate all n! permutations,
 *   so its decision tree has >= n! leaves and height >= log2(n!) =
 *   Theta(n log n) (Stirling): sorting is Omega(n log n), and introsort meets
 *   it at O(n log n) -> Theta(n log n). Thus with g(n) = n log n:
 *     upper  O:     C(n) <= c2 * (n log2 n)  for n >= 2  => O(n log n)
 *     lower  Omega: C(n) >= c1 * (n log2 n)  for n >= 2  => Omega(n log n)
 *     tight  Theta: both hold                            => Theta(n log n)
 *   The overall bound is the SAME for best/average/worst -- the O(n) scan can
 *   never overtake the Theta(n log n) sort -- so no per-case split is needed.
 *
 * Key points:
 *   - Sort key is value/weight ratio, DESCENDING; comparing v_i * w_j vs
 *     v_j * w_i (cross-multiplication) avoids floating-point division in
 *     the comparator and its rounding noise.
 *   - The optimal value is generally fractional -> compare doubles with a
 *     small epsilon, never with ==.
 *   - Greedy FAILS for 0/1 knapsack (indivisible items): there the
 *     density heuristic can be arbitrarily bad and DP is required.
 *   - At most one item is ever split (the last one that does not fit whole).
 */

#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <cassert>

struct Item {
    double value;
    double weight;
};

// Returns the maximum achievable value for capacity W using fractional picks.
double fractionalKnapsack(std::vector<Item> items, double capacity) {
    // Greedy choice: highest value density first. Compare via cross-
    // multiplication (a.value/a.weight > b.value/b.weight) to sidestep
    // division. weight > 0 is assumed for every item.
    std::sort(items.begin(), items.end(),
              [](const Item& a, const Item& b) {
                  return a.value * b.weight > b.value * a.weight;
              });

    double total = 0.0;
    double remaining = capacity;
    for (const Item& it : items) {
        if (remaining <= 0.0) break;
        if (it.weight <= remaining) {
            // Whole item fits: take all of it.
            total += it.value;
            remaining -= it.weight;
        } else {
            // Take the fraction that exactly fills the remaining capacity.
            total += it.value * (remaining / it.weight);
            remaining = 0.0;
            break;  // knapsack is now full
        }
    }
    return total;
}

int main() {
    const double eps = 1e-9;

    // Known instance (classic textbook example):
    //   items (value, weight): (60,10), (100,20), (120,30); W = 50.
    //   densities: 6, 5, 4. Take item0 (10 -> 60) and item1 (20 -> 100),
    //   then 20/30 of item2 -> 120 * 2/3 = 80. Optimum = 240.
    std::vector<Item> items = {{60, 10}, {100, 20}, {120, 30}};
    double best = fractionalKnapsack(items, 50.0);
    assert(std::fabs(best - 240.0) < eps);

    // Edge cases.
    {
        // Zero capacity -> nothing can be taken.
        assert(std::fabs(fractionalKnapsack(items, 0.0) - 0.0) < eps);

        // Capacity exceeds total weight (10+20+30=60) -> take everything.
        double all = fractionalKnapsack(items, 100.0);
        assert(std::fabs(all - (60.0 + 100.0 + 120.0)) < eps);

        // Single item, partial take: half of (value 50, weight 4) at W=2.
        std::vector<Item> one = {{50.0, 4.0}};
        assert(std::fabs(fractionalKnapsack(one, 2.0) - 25.0) < eps);

        // Empty item list -> value 0.
        std::vector<Item> none;
        assert(std::fabs(fractionalKnapsack(none, 10.0) - 0.0) < eps);

        // Order independence: shuffled input yields the same optimum.
        std::vector<Item> shuffled = {{120, 30}, {60, 10}, {100, 20}};
        assert(std::fabs(fractionalKnapsack(shuffled, 50.0) - 240.0) < eps);
    }

    // Short demo.
    std::cout << "Fractional Knapsack (greedy by value/weight density)\n";
    std::cout << "Capacity 50, items (60,10)(100,20)(120,30)\n";
    std::cout << "Optimal value = " << best << " (expected 240)\n";
    std::cout << "All assertions passed.\n";
    return 0;
}
