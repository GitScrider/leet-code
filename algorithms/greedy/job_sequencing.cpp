/*
 * Problem: Job Sequencing with Deadlines            Category: Algorithm - Greedy
 * ----------------------------------------------------------------------------
 * We are given n jobs. Each job i takes exactly ONE unit of time, must finish on
 * or before its integer deadline d_i (>= 1), and yields profit p_i if scheduled
 * in time. A single machine runs one job per unit slot. Maximize total profit.
 *
 * GREEDY CHOICE (and WHY it is safe):
 *   Consider jobs in NON-INCREASING profit order. For each job, place it in the
 *   LATEST still-free slot t with t <= deadline (slots indexed 1..maxDeadline).
 *   If no such slot exists, drop the job.
 *
 *   Exchange argument: the set of schedulable job sets forms a MATROID (the
 *   "transversal / scheduling matroid": a subset S is independent iff its jobs
 *   can all meet their deadlines, testable by placing each in a free <= slot).
 *   For a matroid, the greedy that adds elements by decreasing weight yields a
 *   maximum-weight independent set -- this is exactly what we do. Placing a job
 *   in the LATEST free slot keeps earlier slots open for tighter-deadline jobs
 *   we may still meet later, never hurting a future (lower-profit) choice.
 *
 * Complexity:
 *   +-----------------------------+----------------------+
 *   | Step                        | Cost                 |
 *   +-----------------------------+----------------------+
 *   | Sort by profit DESC         | O(n log n)           |
 *   | Slot search (linear scan)   | O(n * maxDeadline)   |
 *   | Slot search (DSU variant)   | O(n * alpha(n))      |
 *   +-----------------------------+----------------------+
 *   Dominated by the sort when maxDeadline = O(n); the DSU "find next free slot"
 *   makes the placement near-linear.
 *
 * Complexity derivation (sort + amortized DSU slot assignment):
 *   Let n = number of jobs and D = maxDeadline (number of unit-time slots).
 *   PHASE 1 - sort by profit DESC. A comparison sort of n items costs
 *       T_sort(n) = O(n log n)
 *   (same recursion-tree summation as merge sort: log n levels * n work/level).
 *   PHASE 2 - place each job in its latest free slot. Two variants:
 *     (a) LINEAR-SCAN row O(n*maxDeadline): each job scans slots
 *         deadline, deadline-1, ..., 1 for the first free one; worst case every
 *         scan walks all D slots, so
 *             C_scan = SUM_{j=1}^{n} (slots scanned by job j)
 *                    <= SUM_{j=1}^{n} D  =  n*D  =  O(n * maxDeadline).
 *     (b) DSU row O(n*alpha(n)) (what THIS file runs): each job performs exactly
 *         ONE dsuFind(deadline) plus an O(1) relink parent[free]=free-1. A whole
 *         sequence of n such find operations with path halving over a universe of
 *         D+1 slots costs, by Tarjan's amortized union-find bound,
 *             C_dsu = O((n + D) * alpha(D))  =  O(n * alpha(n))   when D = O(n),
 *         where alpha is the inverse Ackermann function (alpha <= 4 in practice,
 *         effectively constant). Slot-array init adds O(D).
 *   TOTAL (DSU variant): T(n) = T_sort + O(D) + C_dsu
 *                              = O(n log n) + O(D) + O(n*alpha(n))
 *                              = O(n log n)      (sort dominates when D = O(n)).
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants):
 *     f(n) = O(g)      iff  EXISTS c2, n0 :       f(n) <= c2*g(n)  for n >= n0
 *     f(n) = Omega(g)  iff  EXISTS c1, n0 :  c1*g(n) <= f(n)        for n >= n0
 *     f(n) = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   Here f(n) = c1*n*log n (sort) + c2*n*alpha(n) (placement); since alpha(n)
 *   grows slower than log n, the sort term dominates. With g(n) = n log n:
 *     upper  O:     f(n) <= 2c1*(n log n)  for n >= n0   => O(n log n)
 *     lower  Omega: sorting n profits by comparisons needs Omega(n log n)
 *                   (decision-tree bound) and runs on EVERY input => Omega(n log n)
 *     tight  Theta: both hold  => Theta(n log n)  (assuming D = O(n)).
 *   The sort is data-independent (all n jobs sorted every time), so best =
 *   average = worst = Theta(n log n). The comparison lower bound Omega(n log n)
 *   DOES apply because the dominant step genuinely is a comparison sort of the
 *   profits; hashing cannot help since we need them in decreasing order. If
 *   deadlines are sparse (D >> n) the O(D + n*alpha(D)) placement can dominate,
 *   giving O(n log n + D).
 *
 * Key points:
 *   - Sort key: profit DESC. Ties may be broken arbitrarily (any order is
 *     optimal), but we keep a stable order for reproducible output.
 *   - Latest-free-slot rule is what makes the greedy correct, not earliest.
 *   - Greedy is NOT optimal if jobs have arbitrary DURATIONS or release times
 *     (that becomes a harder scheduling problem); this unit-time matroid case
 *     is the special structure that rescues greedy.
 */

#include <vector>
#include <algorithm>
#include <string>
#include <cstddef>
#include <cassert>
#include <iostream>

struct Job {
    std::string id;
    int deadline;   // >= 1, in unit-time slots
    int profit;     // >= 0
};

struct Result {
    int totalProfit = 0;
    int jobCount = 0;
    std::vector<std::string> scheduled; // slot 1..k -> job id ("" means idle)
};

// Disjoint-set "find latest free slot" helper: parent[t] points to the largest
// free slot <= t (parent[t] == t means slot t itself is free).
static int dsuFind(std::vector<int>& parent, int x) {
    while (parent[x] != x) {
        parent[x] = parent[parent[x]]; // path halving
        x = parent[x];
    }
    return x;
}

// Greedy scheduler using a DSU to locate the latest free slot in near-O(1).
static Result scheduleJobs(std::vector<Job> jobs) {
    // Sort by profit DESC; stable so equal-profit jobs keep input order.
    std::stable_sort(jobs.begin(), jobs.end(),
                     [](const Job& a, const Job& b) { return a.profit > b.profit; });

    int maxDeadline = 0;
    for (const Job& j : jobs) maxDeadline = std::max(maxDeadline, j.deadline);

    // Slots 1..maxDeadline; index 0 is the sentinel "no free slot available".
    std::vector<int> parent(static_cast<std::size_t>(maxDeadline) + 1);
    for (int t = 0; t <= maxDeadline; ++t) parent[static_cast<std::size_t>(t)] = t;

    std::vector<std::string> slotJob(static_cast<std::size_t>(maxDeadline) + 1, "");

    Result res;
    for (const Job& j : jobs) {
        int free = dsuFind(parent, j.deadline); // latest free slot <= deadline
        if (free > 0) {                          // 0 means everything <= deadline is taken
            slotJob[static_cast<std::size_t>(free)] = j.id;
            parent[static_cast<std::size_t>(free)] = free - 1; // link to next-lower slot
            res.totalProfit += j.profit;
            res.jobCount += 1;
        }
    }

    // Emit schedule in slot order 1..maxDeadline for the demo.
    for (int t = 1; t <= maxDeadline; ++t)
        res.scheduled.push_back(slotJob[static_cast<std::size_t>(t)]);
    return res;
}

// Brute-force optimum for small n: try every subset, keep the max-profit subset
// that is feasible (each chosen job assigned to a distinct slot <= its deadline).
static int bruteForceMaxProfit(const std::vector<Job>& jobs) {
    const std::size_t n = jobs.size();
    int best = 0;
    for (std::size_t mask = 0; mask < (static_cast<std::size_t>(1) << n); ++mask) {
        std::vector<const Job*> chosen;
        int profit = 0, maxD = 0;
        for (std::size_t i = 0; i < n; ++i) {
            if (mask & (static_cast<std::size_t>(1) << i)) {
                chosen.push_back(&jobs[i]);
                profit += jobs[i].profit;
                maxD = std::max(maxD, jobs[i].deadline);
            }
        }
        // Feasibility: greedily place chosen jobs (sorted by deadline ASC) into
        // earliest free slots; feasible iff all fit.
        std::sort(chosen.begin(), chosen.end(),
                  [](const Job* a, const Job* b) { return a->deadline < b->deadline; });
        std::vector<bool> used(static_cast<std::size_t>(maxD) + 1, false);
        bool feasible = true;
        for (const Job* j : chosen) {
            bool placed = false;
            for (int t = j->deadline; t >= 1; --t) {
                if (!used[static_cast<std::size_t>(t)]) { used[static_cast<std::size_t>(t)] = true; placed = true; break; }
            }
            if (!placed) { feasible = false; break; }
        }
        if (feasible) best = std::max(best, profit);
    }
    return best;
}

int main() {
    // Classic instance. Deadlines are unit-time slots; profits as shown. The
    // optimum uses slots 1..3 (see assertion note below).
    std::vector<Job> jobs = {
        {"a", 2, 100},
        {"b", 1, 19},
        {"c", 2, 27},
        {"d", 1, 25},
        {"e", 3, 15},
    };
    Result r = scheduleJobs(jobs);

    // Optimal: slots 1..3. Pick a(100,d2) in slot 2, c(27,d2) in slot 1, e(15,d3)
    // in slot 3 -> profit 142, three jobs. (d and b lose to c/a on their slots.)
    assert(r.totalProfit == 142);
    assert(r.jobCount == 3);
    assert(r.totalProfit == bruteForceMaxProfit(jobs));

    // A second known instance from textbooks.
    std::vector<Job> jobs2 = {
        {"J1", 4, 20}, {"J2", 1, 10}, {"J3", 1, 40}, {"J4", 1, 30}
    };
    Result r2 = scheduleJobs(jobs2);
    assert(r2.totalProfit == 60);   // J3(40) slot1, J1(20) slot4
    assert(r2.jobCount == 2);
    assert(r2.totalProfit == bruteForceMaxProfit(jobs2));

    // Edge case: single job.
    std::vector<Job> one = {{"solo", 1, 5}};
    Result r3 = scheduleJobs(one);
    assert(r3.totalProfit == 5 && r3.jobCount == 1);

    // Edge case: empty input -> zero profit, zero jobs, empty schedule.
    std::vector<Job> none;
    Result r4 = scheduleJobs(none);
    assert(r4.totalProfit == 0 && r4.jobCount == 0 && r4.scheduled.empty());

    // Edge case: all share deadline 1 -> only the single most profitable fits.
    std::vector<Job> clash = {{"x", 1, 3}, {"y", 1, 9}, {"z", 1, 7}};
    Result r5 = scheduleJobs(clash);
    assert(r5.totalProfit == 9 && r5.jobCount == 1);
    assert(r5.totalProfit == bruteForceMaxProfit(clash));

    std::cout << "Job Sequencing demo\n";
    std::cout << "  max profit = " << r.totalProfit
              << ", jobs scheduled = " << r.jobCount << "\n";
    std::cout << "  slots: ";
    for (std::size_t t = 0; t < r.scheduled.size(); ++t)
        std::cout << "[" << (t + 1) << ":" << (r.scheduled[t].empty() ? "-" : r.scheduled[t]) << "] ";
    std::cout << "\nAll assertions passed.\n";
    return 0;
}
