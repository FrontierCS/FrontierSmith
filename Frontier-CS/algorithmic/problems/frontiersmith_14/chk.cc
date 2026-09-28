#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- read problem input ----
    int n, m, k, H;
    n = inf.readInt(); m = inf.readInt(); k = inf.readInt(); H = inf.readInt();

    vector<long long> w(n + 1);
    for (int i = 1; i <= n; i++) w[i] = inf.readLong();

    // adjacency
    vector<vector<int>> adj(n + 1);
    set<pair<int,int>> edgeSet;
    for (int i = 0; i < m; i++) {
        int u = inf.readInt(), v = inf.readInt();
        adj[u].push_back(v);
        adj[v].push_back(u);
        edgeSet.insert({min(u,v), max(u,v)});
    }

    auto hasEdge = [&](int u, int v) -> bool {
        return edgeSet.count({min(u,v), max(u,v)}) > 0;
    };

    // ---- helper: compute exposure given bee positions ----
    // bee_pos[i][t] = position of bee i at time t (0-indexed time 0..H)
    auto computeExposure = [&](const vector<vector<int>>& bee_pos) -> long long {
        // B_t = set of vertices occupied by any bee at time t
        // R_0 = V \ B_0
        // S_t = R_{t-1} \ B_t
        // R_t = { x not in B_t | exists y in S_t with edge(y,x) }

        long long E = 0;

        // Use bitsets or boolean arrays
        vector<bool> inB(n + 1, false);
        vector<bool> R(n + 1, false);

        // B_0
        for (int i = 0; i < k; i++) inB[bee_pos[i][0]] = true;

        // R_0
        for (int v = 1; v <= n; v++) {
            if (!inB[v]) {
                R[v] = true;
                E += w[v];
            }
        }

        for (int t = 1; t <= H; t++) {
            // compute B_t
            fill(inB.begin(), inB.end(), false);
            for (int i = 0; i < k; i++) inB[bee_pos[i][t]] = true;

            // S_t = R_{t-1} \ B_t
            vector<bool> S(n + 1, false);
            for (int v = 1; v <= n; v++)
                if (R[v] && !inB[v]) S[v] = true;

            // R_t = { x not in B_t | exists y in S_t with edge(y,x) }
            fill(R.begin(), R.end(), false);
            for (int y = 1; y <= n; y++) {
                if (!S[y]) continue;
                for (int x : adj[y]) {
                    if (!inB[x]) R[x] = true;
                }
            }
            for (int v = 1; v <= n; v++)
                if (R[v]) E += w[v];
        }
        return E;
    };

    // ---- compute baseline ----
    // BFS distances
    auto bfsDist = [&](int src) -> vector<int> {
        vector<int> dist(n + 1, -1);
        queue<int> q;
        dist[src] = 0;
        q.push(src);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v : adj[u]) {
                if (dist[v] == -1) { dist[v] = dist[u] + 1; q.push(v); }
            }
        }
        return dist;
    };

    // Greedy: pick k distinct starting vertices
    // At each step pick vertex v (not chosen) maximizing total weight of vertices
    // within distance 2 of C ∪ {v}
    // We maintain covered[v] = true if within dist 2 of some chosen vertex
    vector<bool> covered(n + 1, false);
    vector<bool> chosen(n + 1, false);
    // precompute dist-2 neighborhoods
    vector<vector<int>> nbr2(n + 1);
    for (int v = 1; v <= n; v++) {
        auto d = bfsDist(v);
        for (int u = 1; u <= n; u++)
            if (d[u] != -1 && d[u] <= 2) nbr2[v].push_back(u);
    }

    long long coveredWeight = 0;
    vector<int> baseStarts;
    for (int iter = 0; iter < k; iter++) {
        int best = -1;
        long long bestGain = -1;
        for (int v = 1; v <= n; v++) {
            if (chosen[v]) continue;
            // gain = sum w_u for u in nbr2[v] and not covered
            long long gain = 0;
            for (int u : nbr2[v])
                if (!covered[u]) gain += w[u];
            if (gain > bestGain || (gain == bestGain && (best == -1 || v < best))) {
                bestGain = gain;
                best = v;
            }
        }
        chosen[best] = true;
        baseStarts.push_back(best);
        for (int u : nbr2[best])
            if (!covered[u]) { covered[u] = true; coveredWeight += w[u]; }
    }

    // baseline: bees stay forever
    vector<vector<int>> base_pos(k, vector<int>(H + 1));
    for (int i = 0; i < k; i++)
        for (int t = 0; t <= H; t++)
            base_pos[i][t] = baseStarts[i];

    long long B = computeExposure(base_pos);

    // ---- read participant output ----
    vector<vector<int>> bee_pos(k, vector<int>(H + 1));
    for (int i = 0; i < k; i++) {
        for (int t = 0; t <= H; t++) {
            int v = ouf.readInt(1, n, ("bee " + to_string(i+1) + " time " + to_string(t) + ": vertex out of range").c_str());
            bee_pos[i][t] = v;
        }
    }
    // check no extra tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra tokens in output after reading k*(H+1) integers.");
    }

    // ---- check feasibility ----
    for (int i = 0; i < k; i++) {
        for (int t = 1; t <= H; t++) {
            int u = bee_pos[i][t-1], v = bee_pos[i][t];
            if (u != v && !hasEdge(u, v)) {
                quitf(_wa, "Bee %d moves from %d to %d at time %d, but no such edge exists.", i+1, u, v, t);
            }
        }
    }

    // ---- compute participant exposure ----
    long long E = computeExposure(bee_pos);

    // ---- score ----
    // score = floor(1,000,000 * min(3, (B+1)/(E+1)))
    // ratio for quitp: clamp to [0,1] where 1.0 means participant gets 3,000,000
    double raw = (double)(B + 1) / (double)(E + 1);
    if (raw > 3.0) raw = 3.0;
    // ratio in [0,1]: 1.0 means full 3,000,000
    double ratio = raw / 3.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio, "Exposure E=%lld, Baseline B=%lld, raw_ratio=%.6f, Ratio: %.6f", E, B, raw, ratio);
    return 0;
}