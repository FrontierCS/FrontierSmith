#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef __int128 lll;

// Compute baseline greedy value exactly as described in the problem statement
ll computeBaseline(int L, int R, ll B,
                   const vector<ll>& a,
                   const vector<int>& b_cap,
                   const vector<ll>& c,
                   const vector<vector<int>>& adj) {
    vector<bool> assigned(R + 1, false);
    ll remaining = B;
    ll total_gain = 0;
    vector<bool> stall_open(L + 1, false);

    while (true) {
        int best_u = -1;
        ll best_gain = 0;
        ll best_a_val = 1;

        for (int u = 1; u <= L; u++) {
            if (stall_open[u]) continue;
            if (a[u] > remaining) continue;

            // Collect unassigned neighbors
            vector<pair<ll,int>> cands;
            for (int v : adj[u]) {
                if (!assigned[v]) {
                    cands.push_back({c[v], v});
                }
            }
            // sort by decreasing c, tie by smaller index
            sort(cands.begin(), cands.end(), [](const pair<ll,int>& x, const pair<ll,int>& y){
                if (x.first != y.first) return x.first > y.first;
                return x.second < y.second;
            });

            ll take = min((ll)cands.size(), (ll)b_cap[u]);
            ll gain = 0;
            for (ll k = 0; k < take; k++) gain += cands[k].first;

            if (gain == 0) continue;

            if (best_u == -1) {
                best_u = u;
                best_gain = gain;
                best_a_val = a[u];
            } else {
                // Compare gain/a[u] vs best_gain/best_a_val via cross multiply
                lll lhs = (lll)gain * (lll)best_a_val;
                lll rhs = (lll)best_gain * (lll)a[u];
                bool better = false;
                if (lhs > rhs) {
                    better = true;
                } else if (lhs == rhs) {
                    // tie: larger gain
                    if (gain > best_gain) {
                        better = true;
                    } else if (gain == best_gain) {
                        // tie: smaller u
                        if (u < best_u) {
                            better = true;
                        }
                    }
                }
                if (better) {
                    best_u = u;
                    best_gain = gain;
                    best_a_val = a[u];
                }
            }
        }

        if (best_u == -1) break;

        // Open best_u, assign Cand(best_u)
        stall_open[best_u] = true;
        remaining -= a[best_u];

        // Recompute Cand(best_u) and assign
        vector<pair<ll,int>> cands;
        for (int v : adj[best_u]) {
            if (!assigned[v]) {
                cands.push_back({c[v], v});
            }
        }
        sort(cands.begin(), cands.end(), [](const pair<ll,int>& x, const pair<ll,int>& y){
            if (x.first != y.first) return x.first > y.first;
            return x.second < y.second;
        });

        ll take = min((ll)cands.size(), (ll)b_cap[best_u]);
        for (ll k = 0; k < take; k++) {
            int v = cands[k].second;
            assigned[v] = true;
            total_gain += c[v];
        }
    }

    return total_gain;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt(1, 2000, "T");

    double total_score = 0.0;

    for (int tc = 1; tc <= T; tc++) {
        // Read input for this test case
        int L = inf.readInt(1, 500000, "L");
        int R = inf.readInt(1, 500000, "R");
        int M = inf.readInt(0, 500000, "M");
        ll B = inf.readLong(0, (ll)1e12, "B");

        vector<ll> a(L + 1);
        vector<int> b_cap(L + 1);
        vector<ll> c(R + 1);

        for (int u = 1; u <= L; u++) a[u] = inf.readLong(1, (ll)1e9, "a_u");
        for (int u = 1; u <= L; u++) b_cap[u] = inf.readInt(1, R, "b_u");
        for (int i = 1; i <= R; i++) c[i] = inf.readLong(1, (ll)1e12, "c_i");

        // adjacency: adj[u] = list of customer groups stall u can serve
        vector<vector<int>> adj(L + 1);
        // edge set for O(1) lookup
        set<pair<int,int>> edge_set;
        for (int e = 0; e < M; e++) {
            int u = inf.readInt(1, L, "u");
            int v = inf.readInt(1, R, "v");
            adj[u].push_back(v);
            edge_set.insert({u, v});
        }

        // Read participant output: R integers
        vector<int> x(R + 1);
        for (int i = 1; i <= R; i++) {
            x[i] = ouf.readInt(0, L, ("x_" + to_string(i)).c_str());
        }

        // Validate feasibility
        bool feasible = true;
        string fail_reason;

        // Check 1: edge existence
        for (int i = 1; i <= R && feasible; i++) {
            if (x[i] > 0) {
                if (edge_set.find({x[i], i}) == edge_set.end()) {
                    feasible = false;
                    fail_reason = "No edge between stall " + to_string(x[i]) +
                                  " and customer group " + to_string(i);
                }
            }
        }

        // Check 2: capacity
        vector<int> stall_count(L + 1, 0);
        for (int i = 1; i <= R && feasible; i++) {
            if (x[i] > 0) {
                stall_count[x[i]]++;
                if (stall_count[x[i]] > b_cap[x[i]]) {
                    feasible = false;
                    fail_reason = "Stall " + to_string(x[i]) + " exceeds capacity";
                }
            }
        }

        // Check 3: budget
        if (feasible) {
            ll total_cost = 0;
            for (int u = 1; u <= L; u++) {
                if (stall_count[u] > 0) {
                    total_cost += a[u];
                    if (total_cost > B) {
                        feasible = false;
                        fail_reason = "Budget exceeded";
                        break;
                    }
                }
            }
        }

        // Compute O_t
        ll O_t = 0;
        if (feasible) {
            for (int i = 1; i <= R; i++) {
                if (x[i] > 0) O_t += c[i];
            }
        }
        // If infeasible, O_t = 0

        // Compute U_t
        ll U_t = 0;
        for (int i = 1; i <= R; i++) U_t += c[i];

        // Compute G_t (baseline)
        ll G_t = computeBaseline(L, R, B, a, b_cap, c, adj);

        // Compute per-test score s_t
        double s_t;
        if (G_t == U_t) {
            // baseline already achieves full revenue
            s_t = (O_t == U_t) ? 1.0 : 0.0;
        } else {
            double num = (double)(O_t - G_t);
            double den = (double)(U_t - G_t);
            s_t = num / den;
            if (s_t < 0.0) s_t = 0.0;
            if (s_t > 1.0) s_t = 1.0;
        }

        total_score += s_t;
    }

    // After reading all test cases, check EOF
    if (!ouf.seekEof()) {
        quitf(_wa, "Trailing data in output after all test cases");
    }

    // ratio in [0,1]: average per-test score
    // The config uses score: 1000000, so the judge computes floor(1000000 * ratio)
    // which matches the statement formula floor(10^6 * (1/T) * sum(s_t))
    double ratio = total_score / (double)T;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio, "Ratio: %.9f", ratio);
}