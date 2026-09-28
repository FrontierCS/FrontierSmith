#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef pair<ll,int> pli;

static const ll INF = 2e18;

// Dijkstra from source src (0-indexed); returns dist array
vector<ll> dijkstra(int src, int n, const vector<vector<pair<int,ll>>>& adj) {
    vector<ll> d(n, INF);
    d[src] = 0;
    priority_queue<pli, vector<pli>, greater<pli>> pq;
    pq.push({0, src});
    while (!pq.empty()) {
        auto [cost, u] = pq.top(); pq.pop();
        if (cost > d[u]) continue;
        for (auto [v, w] : adj[u]) {
            if (d[u] + w < d[v]) {
                d[v] = d[u] + w;
                pq.push({d[v], v});
            }
        }
    }
    return d;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read input ----
    int n = inf.readInt();
    int e = inf.readInt();
    int m = inf.readInt();

    vector<ll> c(n), b(n), f(n);
    for (int i = 0; i < n; i++) c[i] = inf.readLong();
    for (int i = 0; i < n; i++) b[i] = inf.readLong();
    for (int i = 0; i < n; i++) f[i] = inf.readLong();

    vector<int> p(m);
    for (int j = 0; j < m; j++) {
        p[j] = inf.readInt() - 1; // 0-indexed
    }

    vector<vector<pair<int,ll>>> adj(n);
    for (int i = 0; i < e; i++) {
        int u = inf.readInt() - 1;
        int v = inf.readInt() - 1;
        ll w = inf.readLong();
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
    }

    // ---- All-pairs shortest paths (Dijkstra from each node) ----
    // dist_from[i][j] = shortest dist from node i to node j (0-indexed)
    vector<vector<ll>> dist_from(n);
    for (int i = 0; i < n; i++) {
        dist_from[i] = dijkstra(i, n, adj);
    }

    // ---- Compute Lower Bound L ----
    // L = sum_{j=1..m} min_{i=1..n} dist(p_j, i)
    // Since the graph is connected and p_j is one of the nodes, min dist = 0.
    // We still compute it correctly for correctness.
    ll L = 0;
    for (int j = 0; j < m; j++) {
        ll best = INF;
        for (int i = 0; i < n; i++) {
            best = min(best, dist_from[p[j]][i]);
        }
        L += best;
    }

    // ---- Compute Baseline B ----
    // Greedy: process moles 1..m in order.
    //   - If any burrow has a free place (regular or already-opened pantry),
    //     assign to the closest such burrow (ties: smaller index).
    //   - Otherwise open the pantry at the burrow minimizing dist(p_j,i)+f_i
    //     among burrows with b_i > 0 that are not yet opened (ties: smaller index),
    //     then assign there.
    {
        vector<ll> free_reg(n);   // remaining regular capacity
        vector<ll> free_emer(n);  // remaining emergency capacity (0 until opened)
        vector<bool> opened(n, false);
        for (int i = 0; i < n; i++) free_reg[i] = c[i];

        ll B = 0;
        vector<int> base_assign(m);

        for (int j = 0; j < m; j++) {
            int src = p[j];

            // Check if any free place exists
            bool any_free = false;
            for (int i = 0; i < n; i++) {
                if (free_reg[i] > 0 || free_emer[i] > 0) { any_free = true; break; }
            }

            if (any_free) {
                // Assign to closest burrow with a free place; ties: smaller index
                ll best_d = INF;
                int best_i = -1;
                for (int i = 0; i < n; i++) {
                    if (free_reg[i] > 0 || free_emer[i] > 0) {
                        ll d = dist_from[src][i];
                        if (d < best_d || (d == best_d && i < best_i)) {
                            best_d = d;
                            best_i = i;
                        }
                    }
                }
                B += best_d;
                base_assign[j] = best_i;
                // Consume a place: regular first, then emergency
                if (free_reg[best_i] > 0) free_reg[best_i]--;
                else free_emer[best_i]--;
            } else {
                // Open a pantry: among unopened burrows with b_i > 0,
                // pick the one minimizing dist(p_j,i)+f_i; ties: smaller index
                ll best_cost = INF;
                int best_i = -1;
                for (int i = 0; i < n; i++) {
                    if (!opened[i] && b[i] > 0) {
                        ll cost = dist_from[src][i] + f[i];
                        if (cost < best_cost || (cost == best_cost && i < best_i)) {
                            best_cost = cost;
                            best_i = i;
                        }
                    }
                }
                if (best_i == -1) {
                    // Should not happen per problem guarantee
                    quitf(_fail, "Baseline could not find a pantry to open for mole %d", j+1);
                }
                opened[best_i] = true;
                B += f[best_i];
                free_emer[best_i] = b[best_i];
                // Assign mole to best_i
                ll d = dist_from[src][best_i];
                B += d;
                base_assign[j] = best_i;
                free_emer[best_i]--;
            }
        }

        // ---- Read participant output ----
        // Line 1: n integers z_1 ... z_n
        vector<int> z(n);
        for (int i = 0; i < n; i++) {
            z[i] = ouf.readInt(0, 1,
                ("z[" + to_string(i+1) + "] must be 0 or 1").c_str());
        }

        // Line 2: m integers a_1 ... a_m
        vector<int> a(m);
        for (int j = 0; j < m; j++) {
            a[j] = ouf.readInt(1, n,
                ("a[" + to_string(j+1) + "] must be in [1," + to_string(n) + "]").c_str());
            a[j]--; // 0-indexed
        }

        // Check EOF
        if (!ouf.seekEof()) {
            quitf(_wa, "Extra data found after the expected output");
        }

        // ---- Validate capacity constraints ----
        vector<ll> assigned_count(n, 0);
        for (int j = 0; j < m; j++) {
            assigned_count[a[j]]++;
        }
        for (int i = 0; i < n; i++) {
            ll capacity = c[i] + b[i] * (ll)z[i];
            if (assigned_count[i] > capacity) {
                quitf(_wa,
                    "Burrow %d has %lld moles assigned but capacity is %lld (c=%lld, b=%lld, z=%d)",
                    i+1, (long long)assigned_count[i], (long long)capacity,
                    (long long)c[i], (long long)b[i], z[i]);
            }
        }

        // ---- Compute objective X ----
        ll X = 0;
        for (int j = 0; j < m; j++) {
            ll dval = dist_from[p[j]][a[j]];
            if (dval == INF) {
                quitf(_wa, "Mole %d assigned to unreachable burrow %d", j+1, a[j]+1);
            }
            X += dval;
        }
        for (int i = 0; i < n; i++) {
            if (z[i] == 1) {
                X += f[i];
            }
        }

        // ---- Compute score ----
        // Statement: Score = 1000 * clamp((B - X) / (B - L), 0, 1)
        // The judge multiplies by 1000; we output ratio in [0, 1].
        // quitp(ratio, ...) passes ratio in [0,1] to the judge.
        double score_ratio;
        if (B <= L) {
            // Degenerate: baseline equals lower bound.
            // Per problem statement, official tests satisfy B > L,
            // but handle gracefully: full score if X <= B.
            score_ratio = (X <= B) ? 1.0 : 0.0;
        } else {
            double num = (double)(B - X);
            double den = (double)(B - L);
            score_ratio = num / den;
            if (score_ratio < 0.0) score_ratio = 0.0;
            if (score_ratio > 1.0) score_ratio = 1.0;
        }

        // Required tag: "Ratio: <score_ratio>" bounded [0,1].
        // The judge reads this tag and multiplies by 1000 to get the displayed score.
        quitp(score_ratio,
            "X=%lld B=%lld L=%lld Ratio: %.6f",
            (long long)X, (long long)B, (long long)L, score_ratio);
    }
}