#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    double total_score = 0.0;
    int num_cases = 0;

    for (int tc = 0; tc < T; tc++) {
        num_cases++;

        long long m, n, e, x, k;
        m = inf.readLong();
        n = inf.readLong();
        e = inf.readLong();
        x = inf.readLong();
        k = inf.readLong();

        vector<long long> L(n), R(n), C(n), H(n), D(n);
        for (int i = 0; i < n; i++) {
            L[i] = inf.readLong();
            R[i] = inf.readLong();
            C[i] = inf.readLong();
            H[i] = inf.readLong();
            D[i] = inf.readLong();
        }

        vector<int> eu(e), ev(e);
        vector<long long> ew(e);
        for (int j = 0; j < e; j++) {
            eu[j] = inf.readInt() - 1;
            ev[j] = inf.readInt() - 1;
            ew[j] = inf.readLong();
        }

        // Read participant output: p_1 .. p_n
        vector<long long> P(n);
        for (int i = 0; i < n; i++) {
            if (ouf.seekEof()) {
                quitf(_wa, "Test case %d: not enough output tokens (expected %lld more values)", tc+1, n - (long long)i);
            }
            P[i] = ouf.readLong();
        }

        // --- Feasibility Checks ---
        bool feasible = true;
        string fail_reason;

        // Check 1: p_i = 0 or l_i <= p_i <= r_i
        for (int i = 0; i < n; i++) {
            if (P[i] != 0 && (P[i] < L[i] || P[i] > R[i])) {
                feasible = false;
                fail_reason = "Lot " + to_string(i+1) + " purchased outside availability window";
                break;
            }
        }

        if (feasible) {
            // Check 2: at most k lots purchased per month
            map<long long, int> month_count;
            for (int i = 0; i < n; i++) {
                if (P[i] != 0) {
                    month_count[P[i]]++;
                }
            }
            for (auto& [t, cnt] : month_count) {
                if (cnt > k) {
                    feasible = false;
                    fail_reason = "Month " + to_string(t) + " has " + to_string(cnt) + " purchases but k=" + to_string(k);
                    break;
                }
            }
        }

        if (feasible) {
            // Check 3: budget constraint
            // For every month t, total cost of lots purchased in months 1..t <= (t-1)*x
            // Collect (purchase_month, cost) sorted by month
            vector<pair<long long,long long>> purchases;
            for (int i = 0; i < n; i++) {
                if (P[i] != 0) {
                    purchases.push_back({P[i], C[i]});
                }
            }
            sort(purchases.begin(), purchases.end());

            long long cumulative_cost = 0;
            int idx = 0;
            int np = (int)purchases.size();
            // We only need to check at months where purchases happen
            for (int qi = 0; qi < np; ) {
                long long t = purchases[qi].first;
                // accumulate all purchases in month t
                while (qi < np && purchases[qi].first == t) {
                    cumulative_cost += purchases[qi].second;
                    qi++;
                }
                long long budget = (t - 1) * x;
                if (cumulative_cost > budget) {
                    feasible = false;
                    fail_reason = "Budget exceeded at month " + to_string(t) + ": spent " + to_string(cumulative_cost) + " > budget " + to_string(budget);
                    break;
                }
            }
        }

        // --- Compute U (upper bound) ---
        long long U = 0;
        for (int i = 0; i < n; i++) {
            if (H[i] > 0) U += H[i];
        }
        for (int j = 0; j < e; j++) {
            if (ew[j] > 0) U += ew[j];
        }

        // --- Compute Obj if feasible ---
        long long Obj = 0;
        if (feasible) {
            // Base value
            for (int i = 0; i < n; i++) {
                if (P[i] != 0) {
                    Obj += H[i] - D[i] * (P[i] - L[i]);
                }
            }
            // Interaction value
            vector<bool> bought(n, false);
            for (int i = 0; i < n; i++) {
                if (P[i] != 0) bought[i] = true;
            }
            for (int j = 0; j < e; j++) {
                if (bought[eu[j]] && bought[ev[j]]) {
                    Obj += ew[j];
                }
            }
        }

        // --- Compute Baseline B ---
        vector<bool> base_bought(n, false);
        vector<long long> base_purchase(n, 0);
        long long baseline_spent = 0;

        for (long long t = 1; t <= m; t++) {
            long long cash = (t - 1) * x - baseline_spent;
            int purchases_this_month = 0;

            while (purchases_this_month < k) {
                int best = -1;
                long long best_cur = 0;
                long long best_c = 0;
                // best_ratio as fraction: best_cur / best_c, compare cross-multiply
                // We'll track best_cur and best_c and compare

                for (int i = 0; i < n; i++) {
                    if (base_bought[i]) continue;
                    if (P[i] != 0 && false) continue; // not used
                    if (L[i] > t || R[i] < t) continue;
                    long long cur = H[i] - D[i] * (t - L[i]);
                    if (cur <= 0) continue;
                    if (C[i] > cash) continue;

                    bool better = false;
                    if (best == -1) {
                        better = true;
                    } else {
                        // Compare cur/C[i] vs best_cur/best_c
                        // cur * best_c vs best_cur * C[i]
                        long long lhs = cur * best_c;
                        long long rhs = best_cur * C[i];
                        if (lhs > rhs) {
                            better = true;
                        } else if (lhs == rhs) {
                            if (cur > best_cur) better = true;
                            else if (cur == best_cur) {
                                if (C[i] < best_c) better = true;
                                else if (C[i] == best_c) {
                                    if (i < best) better = true;
                                }
                            }
                        }
                    }

                    if (better) {
                        best = i;
                        best_cur = cur;
                        best_c = C[i];
                    }
                }

                if (best == -1) break;

                base_bought[best] = true;
                base_purchase[best] = t;
                baseline_spent += C[best];
                cash -= C[best];
                purchases_this_month++;
            }
        }

        long long B_base = 0;
        for (int i = 0; i < n; i++) {
            if (base_bought[i]) {
                B_base += H[i] - D[i] * (base_purchase[i] - L[i]);
            }
        }
        long long B_interact = 0;
        {
            for (int j = 0; j < e; j++) {
                if (base_bought[eu[j]] && base_bought[ev[j]]) {
                    B_interact += ew[j];
                }
            }
        }
        long long B = B_base + B_interact;

        // --- Compute per-test score ---
        double case_score;
        if (!feasible) {
            case_score = 0.0;
        } else if (U == B) {
            // Edge case: baseline already achieves upper bound
            case_score = (Obj >= U) ? 1.0 : 0.0;
        } else {
            // Continuous scoring: clamp((Obj - B) / (U - B), 0, 1)
            double ratio = (double)(Obj - B) / (double)(U - B);
            if (ratio < 0.0) ratio = 0.0;
            if (ratio > 1.0) ratio = 1.0;
            case_score = ratio;
        }

        total_score += case_score;
    }

    // Skip trailing whitespace/newlines in output without strict EOF fail
    // (do not call ouf.readEof() strictly to avoid spurious WA on trailing newlines)

    double final_ratio = (num_cases > 0) ? (total_score / (double)num_cases) : 0.0;
    if (final_ratio < 0.0) final_ratio = 0.0;
    if (final_ratio > 1.0) final_ratio = 1.0;

    quitp(final_ratio, "Ratio: %.6f (total_score=%.4f over %d cases)", final_ratio, total_score, num_cases);

    return 0;
}