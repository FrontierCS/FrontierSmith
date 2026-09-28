#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef __int128 lll;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read problem input
    int n = inf.readInt();
    int s = inf.readInt();
    int e_node = inf.readInt();
    ll T = inf.readLong();

    vector<ll> x(n+1), y(n+1), p(n+1), a(n+1), b(n+1), c(n+1), d(n+1);
    for (int i = 1; i <= n; i++) x[i] = inf.readLong();
    for (int i = 1; i <= n; i++) y[i] = inf.readLong();
    for (int i = 1; i <= n; i++) p[i] = inf.readLong();
    for (int i = 1; i <= n; i++) a[i] = inf.readLong();
    for (int i = 1; i <= n; i++) b[i] = inf.readLong();
    for (int i = 1; i <= n; i++) c[i] = inf.readLong();
    for (int i = 1; i <= n; i++) d[i] = inf.readLong();

    auto calcCost = [&](int i, int j) -> ll {
        ll dx = abs(x[i] - x[j]);
        ll dy = abs(y[i] - y[j]);
        ll overhead;
        if (x[j] < x[i]) overhead = c[i] + b[j];
        else overhead = d[i] + a[j];
        return dx + dy + overhead;
    };

    // Read participant output
    int m = ouf.readInt();
    if (m < 2 || m > n) {
        quitf(_wa, "Invalid m = %d (must be in [2, %d])", m, n);
    }

    vector<int> v(m);
    for (int i = 0; i < m; i++) {
        v[i] = ouf.readInt();
    }
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra content in output after the route");
    }

    // Validate start/end
    if (v[0] != s) quitf(_wa, "Route must start at s=%d, but starts at %d", s, v[0]);
    if (v[m-1] != e_node) quitf(_wa, "Route must end at e=%d, but ends at %d", e_node, v[m-1]);

    // Validate all chairs distinct and valid
    vector<bool> seen(n+1, false);
    for (int i = 0; i < m; i++) {
        if (v[i] < 1 || v[i] > n) quitf(_wa, "Chair index %d out of range [1,%d]", v[i], n);
        if (seen[v[i]]) quitf(_wa, "Chair %d appears more than once in route", v[i]);
        seen[v[i]] = true;
    }

    // Validate time budget
    ll total_time = 0;
    for (int i = 0; i < m-1; i++) {
        ll step = calcCost(v[i], v[i+1]);
        // Check for overflow
        if (total_time > T - step) {
            quitf(_wa, "Total travel time exceeds budget T=%lld", T);
        }
        total_time += step;
    }
    if (total_time > T) {
        quitf(_wa, "Total travel time %lld exceeds budget T=%lld", total_time, T);
    }

    // Compute O
    ll O = 0;
    for (int i = 0; i < m; i++) O += p[v[i]];

    // Compute U
    ll U = 0;
    for (int i = 1; i <= n; i++) U += p[i];

    // Compute baseline B using the greedy described in the problem
    ll B = 0;
    {
        vector<bool> used(n+1, false);
        int cur = s;
        ll used_time = 0;
        used[s] = true;
        B += p[s];
        // e_node is excluded from intermediate candidates but visited at end
        // We mark e_node as "used" so it doesn't appear as a candidate
        // We'll add p[e_node] at the end
        // But if s == e_node that's impossible per constraints (s != e)
        // We don't mark e_node here; we just skip it in the candidate loop

        while (true) {
            int best = -1;
            ll best_pv = 0;
            ll best_cost = 0;

            for (int vv = 1; vv <= n; vv++) {
                if (used[vv] || vv == e_node) continue;
                ll cv = calcCost(cur, vv);
                ll ce = calcCost(vv, e_node);
                // Check feasibility: used_time + cv + ce <= T
                if (cv > T - used_time || ce > T - used_time - cv) continue;

                if (best == -1) {
                    best = vv;
                    best_pv = p[vv];
                    best_cost = cv;
                } else {
                    // Compare p[vv]/cv vs p[best]/best_cost using cross multiplication (__int128)
                    lll lhs = (lll)p[vv] * (lll)best_cost;
                    lll rhs = (lll)best_pv * (lll)cv;
                    if (lhs > rhs) {
                        best = vv; best_pv = p[vv]; best_cost = cv;
                    } else if (lhs == rhs) {
                        // Tie-break: larger p_v
                        if (p[vv] > best_pv) {
                            best = vv; best_pv = p[vv]; best_cost = cv;
                        } else if (p[vv] == best_pv) {
                            // Tie-break: smaller index
                            if (vv < best) {
                                best = vv; best_pv = p[vv]; best_cost = cv;
                            }
                        }
                    }
                }
            }

            if (best == -1) break;
            used_time += best_cost;
            cur = best;
            used[best] = true;
            B += p[best];
        }
        // Jump to e_node
        B += p[e_node];
    }

    // Apply scoring formula
    double score_ratio;
    if (U == B) {
        // score = round(1e6 * O/U) clamped to [0, 1e6], ratio = score/1e6
        score_ratio = (double)O / (double)U;
    } else if (O <= B) {
        // score = round(500000 * O/B), ratio = score/1e6
        score_ratio = 0.5 * (double)O / (double)B;
    } else {
        // score = round(500000 + 500000 * (O-B)/(U-B)), ratio = score/1e6
        score_ratio = 0.5 + 0.5 * (double)(O - B) / (double)(U - B);
    }

    // Clamp to [0, 1]
    score_ratio = max(0.0, min(1.0, score_ratio));

    quitp(score_ratio, "Ratio: %.9f (O=%lld, B=%lld, U=%lld, time=%lld/%lld)",
          score_ratio, O, B, U, total_time, T);

    return 0;
}