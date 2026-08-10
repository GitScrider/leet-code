/*
 * ============================================================================
 * Boolean Satisfiability (SAT / 3-SAT)
 * Category: Algorithm - NP-Complete
 *
 * DECISION PROBLEM:
 *   Given a Boolean formula in Conjunctive Normal Form (CNF) -- an AND of
 *   clauses, each clause an OR of literals, each literal a variable or its
 *   negation -- is there an assignment of true/false to the variables that
 *   makes the whole formula true? 3-SAT restricts every clause to exactly
 *   three literals; it is already NP-complete.
 *
 * COMPLEXITY CLASS:
 *   SAT is NP-complete. This is THE canonical result: the Cook-Levin theorem
 *   (1971) shows every problem in NP reduces to SAT, because the accepting
 *   computation of any polynomial-time nondeterministic Turing machine can be
 *   encoded as a CNF formula that is satisfiable iff the machine accepts. Karp
 *   then reduced SAT -> 3-SAT, so 3-SAT is NP-complete as well. Every other
 *   problem in this folder ultimately traces its hardness back to SAT.
 *
 * EXACT-ALGORITHM COMPLEXITY (DPLL backtracking):
 *   +-----------------------+-------------------------------------------------+
 *   | Aspect                | Cost                                            |
 *   +-----------------------+-------------------------------------------------+
 *   | Time (worst case)     | O(2^V) -- V = number of variables               |
 *   | Space                 | O(V + total literals) recursion + working copy  |
 *   +-----------------------+-------------------------------------------------+
 *   No approximation ratio applies: SAT is a decision problem (yes/no), not an
 *   optimization, so there is nothing to approximate here. In practice unit
 *   propagation + pure-literal elimination prune enormous parts of the 2^V tree.
 *
 * Complexity derivation (backtracking state-space tree -> recurrence):
 *   Let V = #variables, m = #clauses, L = total literals over all clauses (the
 *   input size; for 3-SAT L = 3m). DPLL fixes one variable per level and, when it
 *   must guess, tries the chosen literal TRUE then FALSE -> branching factor 2.
 *   With one variable pinned per level the tree has depth <= V, hence
 *
 *       #leaves <= 2^V ,   #nodes <= 2^(V+1) - 1 = O(2^V).
 *
 *   Work per node: one scan for a unit clause, one scan for a pure literal, and
 *   the assumeLiteral() simplification each walk every clause/literal once, plus
 *   copying the formula before a branch -> c*(V + L) operations per node. Ignoring
 *   the (only-ever-helpful) propagation, the worst case obeys the subtract-and-
 *   conquer recurrence
 *
 *       T(V) = 2*T(V-1) + c*(V + L) ,   T(0) = c0*(V + L)   (empty/conflict check)
 *
 *   Unfold it as a recursion tree and sum the work per level:
 *
 *       level d      #nodes     vars left     work on the level
 *       ---------    -------    ----------    ------------------------
 *       d = 0        1          V             c*(V+L)
 *       d = 1        2          V-1           2 * c*(V+L)
 *       d = 2        4          V-2           4 * c*(V+L)
 *       ...          ...        ...           ...
 *       d = k        2^k        V-k           2^k * c*(V+L)
 *
 *       T(V) = SUM_{d=0}^{V} 2^d * c*(V+L)
 *            = c*(V+L) * (2^(V+1) - 1)          (geometric series, ratio 2)
 *            = O(2^V * (V + L))
 *            = O(2^V)   (the polynomial per-node factor is lower order).
 *   The size shrinks by SUBTRACTION (V -> V-1), not division, so the Master
 *   Theorem does not apply; the level sum is dominated by its last term 2^V.
 *
 * Asymptotic bounds  O (upper) / Omega (lower) / Theta (tight):
 *   Formal definitions (c1, c2, n0 positive constants), parameter V:
 *     f = O(g)      iff  EXISTS c2, n0 :        f(V) <= c2*g(V)  for V >= n0
 *     f = Omega(g)  iff  EXISTS c1, n0 :  c1*g(V) <= f(V)         for V >= n0
 *     f = Theta(g)  iff  f = O(g) AND f = Omega(g)
 *   DPLL is data-dependent (propagation / pure-literal pruning), so bounds are
 *   PER-CASE:
 *     BEST case   a forced chain (all unit / pure literals) or an immediate empty
 *                 clause branches 0 times: <= V forced calls -> Theta(V*(V+L)),
 *                 polynomial.
 *     WORST case  no propagation ever fires (clauses keep >= 2 free literals until
 *                 the last level) -> the full 2^V tree is built, Theta(2^V*(V+L)),
 *                 i.e. Theta(2^V) up to the polynomial factor.
 *   Over ALL inputs the time is thus O(2^V) (from the worst case) and Omega(V + L)
 *   (must read the formula at least once); it is NOT a single Theta because
 *   best != worst. The comparison-sort Omega(n log n) bound is irrelevant -- this
 *   is a decision problem, not a sort. The relevant conditional lower bound is the
 *   Exponential Time Hypothesis: no 2^(o(V)) algorithm for 3-SAT is known.
 *
 * KEY POINTS:
 *   - Literal encoding: signed int, +v means "variable v is true", -v means
 *     "variable v is false" (variables numbered from 1). 0 is never a literal.
 *   - DPLL = backtracking DFS over assignments, accelerated by two rules:
 *       * Unit propagation: a clause with one remaining literal FORCES it.
 *       * Pure literal: a variable appearing with only one polarity is set to
 *         satisfy every clause it touches -- never a wrong guess.
 *   - The search returns a WITNESS assignment on SAT; main() verifies that the
 *     witness truly satisfies every clause, and cross-checks DPLL against a
 *     brute-force truth-table oracle on small formulas.
 * ============================================================================
 */

#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>

// A clause is a disjunction (OR) of literals; a CNF formula is a conjunction
// (AND) of clauses. Literal encoding: +v / -v for variable v >= 1.
using Clause = std::vector<int>;
using CNF = std::vector<Clause>;

// Tri-state assignment stored per variable in a vector indexed 1..V:
//   +1 = true, -1 = false, 0 = not yet assigned. Index 0 is unused padding.
using Assignment = std::vector<int>;

// |literal| -> its variable id, without pulling in <cstdlib>'s std::abs.
static int variableOf(int literal) { return literal < 0 ? -literal : literal; }

// Highest variable id used anywhere in the formula (0 if there are none).
static int countVariables(const CNF& formula) {
    int maxVar = 0;
    for (const Clause& clause : formula)
        for (int literal : clause)
            if (variableOf(literal) > maxVar) maxVar = variableOf(literal);
    return maxVar;
}

// Assume `literal` is TRUE and simplify the formula accordingly:
//   - a clause that CONTAINS `literal` is already satisfied  -> drop it;
//   - the opposite literal `-literal` can never be true       -> delete it
//     from any clause that holds it.
// Returns false if this produces an EMPTY clause (a clause with no literals
// left can no longer be satisfied -> conflict on this branch).
static bool assumeLiteral(CNF& formula, int literal) {
    CNF reduced;
    reduced.reserve(formula.size());
    for (const Clause& clause : formula) {
        bool satisfied = false;
        Clause trimmed;
        trimmed.reserve(clause.size());
        for (int lit : clause) {
            if (lit == literal) { satisfied = true; break; } // clause is true
            if (lit == -literal) continue;                   // false literal
            trimmed.push_back(lit);
        }
        if (satisfied) continue;                 // whole clause removed
        if (trimmed.empty()) return false;        // conflict: unsatisfiable clause
        reduced.push_back(std::move(trimmed));
    }
    formula.swap(reduced);
    return true;
}

// DPLL core. Works on a REDUCED copy of the formula plus the running partial
// assignment (shared by reference; forced choices are always safe in both
// branches, so writing them through is fine). Returns true iff satisfiable.
static bool dpll(CNF formula, Assignment& value) {
    // Base case: nothing left to satisfy.
    if (formula.empty()) return true;
    // A pre-existing empty clause (e.g. the caller passed one) is a conflict.
    for (const Clause& clause : formula)
        if (clause.empty()) return false;

    // --- Unit propagation: a one-literal clause leaves no choice. ---
    for (const Clause& clause : formula) {
        if (clause.size() == 1) {
            const int lit = clause[0];
            value[static_cast<std::size_t>(variableOf(lit))] = lit > 0 ? 1 : -1;
            if (!assumeLiteral(formula, lit)) return false;
            return dpll(std::move(formula), value); // re-simplify from the top
        }
    }

    // --- Pure literal elimination: a variable seen with a single polarity can
    //     be fixed to satisfy every clause it appears in, risk-free. ---
    std::vector<char> seenPos(value.size(), 0), seenNeg(value.size(), 0);
    for (const Clause& clause : formula)
        for (int lit : clause) {
            if (lit > 0) seenPos[static_cast<std::size_t>(lit)] = 1;
            else         seenNeg[static_cast<std::size_t>(-lit)] = 1;
        }
    for (std::size_t v = 1; v < value.size(); ++v) {
        const bool pos = seenPos[v] != 0, neg = seenNeg[v] != 0;
        if (pos && !neg) {                       // only appears positive
            value[v] = 1;
            assumeLiteral(formula, static_cast<int>(v));  // cannot conflict
            return dpll(std::move(formula), value);
        }
        if (neg && !pos) {                       // only appears negative
            value[v] = -1;
            assumeLiteral(formula, -static_cast<int>(v)); // cannot conflict
            return dpll(std::move(formula), value);
        }
    }

    // --- Branch: pick a literal and try both truth values (backtracking). ---
    const int branchVar = variableOf(formula.front().front());

    // Try TRUE first. Work on private copies so a failed branch leaves no trace.
    {
        CNF branch = formula;
        Assignment trial = value;
        trial[static_cast<std::size_t>(branchVar)] = 1;
        if (assumeLiteral(branch, branchVar) && dpll(std::move(branch), trial)) {
            value = trial;
            return true;
        }
    }
    // Otherwise try FALSE.
    {
        CNF branch = formula;
        Assignment trial = value;
        trial[static_cast<std::size_t>(branchVar)] = -1;
        if (assumeLiteral(branch, -branchVar) && dpll(std::move(branch), trial)) {
            value = trial;
            return true;
        }
    }
    return false; // both polarities lead to conflict -> this subtree is UNSAT
}

struct SatResult {
    bool satisfiable;
    Assignment value; // valid only when satisfiable; index 1..V, each +1/-1
};

// Public entry point. On SAT, any variable the search never needed is a "don't
// care" -- we default it to true so the returned witness is total.
static SatResult solveSAT(const CNF& formula) {
    const int vars = countVariables(formula);
    Assignment value(static_cast<std::size_t>(vars) + 1, 0);
    if (dpll(formula, value)) {
        for (std::size_t v = 1; v < value.size(); ++v)
            if (value[v] == 0) value[v] = 1;      // free variable -> arbitrary
        return {true, value};
    }
    return {false, {}};
}

// Independent checker: does `value` satisfy EVERY clause? Used to validate the
// witness DPLL hands back (a solver is only trustworthy if its output checks).
static bool satisfies(const CNF& formula, const Assignment& value) {
    for (const Clause& clause : formula) {
        bool clauseTrue = false;
        for (int lit : clause) {
            const bool varTrue =
                value[static_cast<std::size_t>(variableOf(lit))] == 1;
            if ((lit > 0 && varTrue) || (lit < 0 && !varTrue)) {
                clauseTrue = true;
                break;
            }
        }
        if (!clauseTrue) return false;
    }
    return true;
}

// Brute-force oracle: try all 2^V truth assignments. Exponential; used only to
// confirm DPLL's yes/no answer on small formulas.
static bool bruteForceSAT(const CNF& formula) {
    const int vars = countVariables(formula);
    for (unsigned mask = 0; mask < (1u << vars); ++mask) {
        Assignment value(static_cast<std::size_t>(vars) + 1, 0);
        for (int v = 1; v <= vars; ++v)
            value[static_cast<std::size_t>(v)] =
                (mask >> (v - 1)) & 1u ? 1 : -1;
        if (satisfies(formula, value)) return true;
    }
    return false;
}

int main() {
    // --- Classic UNSAT: (x1 OR x2) AND (NOT x1) AND (NOT x2). ---
    // NOT x1 and NOT x2 force both false, but then (x1 OR x2) is false.
    {
        const CNF f = {{1, 2}, {-1}, {-2}};
        const SatResult r = solveSAT(f);
        assert(!r.satisfiable);
        assert(!bruteForceSAT(f)); // oracle agrees
    }

    // --- Satisfiable: verify the returned witness really satisfies it. ---
    // (x1 OR NOT x2) AND (NOT x1 OR x2) AND (x1 OR x2)  =>  forces x1 = x2 = true.
    {
        const CNF f = {{1, -2}, {-1, 2}, {1, 2}};
        const SatResult r = solveSAT(f);
        assert(r.satisfiable);
        assert(satisfies(f, r.value));            // WITNESS is valid
        assert(r.value[1] == 1 && r.value[2] == 1); // unique solution here
    }

    // --- A genuine 3-SAT instance that is satisfiable. ---
    {
        const CNF f = {{1, 2, 3}, {-1, -2, 3}, {1, -2, -3}, {-1, 2, -3}};
        const SatResult r = solveSAT(f);
        assert(r.satisfiable);
        assert(satisfies(f, r.value));
        assert(bruteForceSAT(f));
    }

    // --- A 3-SAT instance that is UNSAT: ALL eight sign-patterns over three
    //     variables. Every possible assignment is explicitly forbidden. ---
    {
        CNF f;
        for (int a = 0; a < 2; ++a)
            for (int b = 0; b < 2; ++b)
                for (int c = 0; c < 2; ++c)
                    f.push_back({a ? 1 : -1, b ? 2 : -2, c ? 3 : -3});
        const SatResult r = solveSAT(f);
        assert(!r.satisfiable);
        assert(!bruteForceSAT(f));
    }

    // --- Edge cases. ---
    {
        const CNF empty = {};                     // no clauses -> trivially SAT
        assert(solveSAT(empty).satisfiable);

        const CNF hasEmptyClause = {{}};          // an empty clause -> UNSAT
        assert(!solveSAT(hasEmptyClause).satisfiable);

        const CNF singleUnit = {{5}};             // one unit clause -> SAT, x5 true
        const SatResult r = solveSAT(singleUnit);
        assert(r.satisfiable && r.value[5] == 1 && satisfies(singleUnit, r.value));
    }

    // --- Cross-check DPLL against the truth-table oracle on assorted formulas,
    //     and validate every reported witness. ---
    {
        const std::vector<CNF> suite = {
            {{1, 2}, {2, 3}, {-1, -3}},
            {{1}, {-1, 2}, {-2, 3}, {-3}},        // chain forcing a contradiction
            {{1, -2, 3}, {-1, 2, 3}, {1, 2, -3}, {-1, -2, -3}},
            {{1, 2, 3, 4}, {-1, -2}, {-3, -4}, {2, 4}},
        };
        for (const CNF& f : suite) {
            const SatResult r = solveSAT(f);
            assert(r.satisfiable == bruteForceSAT(f));
            if (r.satisfiable) assert(satisfies(f, r.value));
        }
    }

    // --- Short demo. ---
    const CNF demo = {{1, -2}, {-1, 2}, {1, 2}};
    const SatResult r = solveSAT(demo);
    std::cout << "Formula (x1 v -x2) ^ (-x1 v x2) ^ (x1 v x2): "
              << (r.satisfiable ? "SAT" : "UNSAT") << '\n';
    if (r.satisfiable) {
        std::cout << "  witness: ";
        for (std::size_t v = 1; v < r.value.size(); ++v)
            std::cout << "x" << v << "=" << (r.value[v] == 1 ? "T" : "F") << ' ';
        std::cout << '\n';
    }
    std::cout << "All SAT / 3-SAT tests passed.\n";
    return 0;
}
