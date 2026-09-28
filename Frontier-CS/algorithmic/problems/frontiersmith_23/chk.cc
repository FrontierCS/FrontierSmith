#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---------- Read input ----------
    int N = inf.readInt();
    int R = inf.readInt();
    long long T = inf.readLong();

    // edge key: (min(a,b), max(a,b)) -> {length, scenic}
    map<pair<int,int>, pair<int,int>> edgeMap;
    // adjacency: adj[u] = list of {v, length}
    vector<vector<pair<int,int>>> adj(N + 1);

    for (int i = 0; i < R; i++) {
        int a = inf.readInt(), b = inf.readInt();
        int l = inf.readInt(), c = inf.readInt();
        auto key = make_pair(min(a, b), max(a, b));
        edgeMap[key] = {l, c};
        adj[a].push_back({b, l});
        adj[b].push_back({a, l});
    }

    const long long INF_DIST = (long long)2e18;

    // ---------- Dijkstra ----------
    auto dijkstra = [&](int src) -> vector<long long> {
        vector<long long> dist(N + 1, INF_DIST);
        priority_queue<pair<long long,int>,
                       vector<pair<long long,int>>,
                       greater<pair<long long,int>>> pq;
        dist[src] = 0;
        pq.push({0LL, src});
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d > dist[u]) continue;
            for (auto& [v, l] : adj[u]) {
                long long nd = dist[u] + (long long)l;
                if (nd < dist[v]) {
                    dist[v] = nd;
                    pq.push({nd, v});
                }
            }
        }
        return dist;
    };

    vector<long long> dist1 = dijkstra(1);

    long long shortestLen = dist1[N];
    if (shortestLen == INF_DIST) {
        quitf(_fail, "No path from 1 to N exists in input graph");
    }

    // ---------- Compute baseline B: lex-smallest shortest path ----------
    // Sort adjacency lists by neighbour id so greedy picks lex-smallest
    for (int u = 1; u <= N; u++) {
        sort(adj[u].begin(), adj[u].end(),
             [](const pair<int,int>& a, const pair<int,int>& b){ return a.first < b.first; });
    }

    long long baselineB = 0;
    {
        // Greedy walk: always step to the smallest-id neighbour that is
        // exactly dist1[neighbour] == dist1[current] + edge_length and
        // dist1[current]+edge_length == (some value on a shortest path)
        // We need dist from N backwards too.
        vector<long long> distN = dijkstra(N);

        int cur = 1;
        set<pair<int,int>> baseUsed;
        while (cur != N) {
            int best = -1;
            for (auto& [v, l] : adj[cur]) {
                // On a shortest path: dist1[cur] + l + distN[v] == shortestLen
                if (dist1[cur] + (long long)l + distN[v] == shortestLen) {
                    if (best == -1 || v < best) best = v;
                }
            }
            if (best == -1) {
                quitf(_fail, "Could not reconstruct lex-smallest shortest path at node %d", cur);
            }
            auto key = make_pair(min(cur, best), max(cur, best));
            if (baseUsed.insert(key).second) {
                baselineB += (long long)edgeMap[key].second;
            }
            cur = best;
        }
    }

    if (baselineB <= 0) {
        quitf(_fail, "Baseline B is non-positive (%lld), which violates problem guarantee", baselineB);
    }

    // ---------- Read participant output ----------
    // Do NOT call ouf.readEof() — trailing newlines are valid output.

    if (ouf.seekEof()) {
        // Empty output
        quitf(_wa, "Empty output");
    }

    int K = ouf.readInt();
    if (K < 2 || K > 200000) {
        quitf(_wa, "K=%d is out of range [2, 200000]", K);
    }

    vector<int> walk(K);
    for (int i = 0; i < K; i++) {
        walk[i] = ouf.readInt();
        if (walk[i] < 1 || walk[i] > N) {
            quitf(_wa, "Vertex %d at position %d is out of range [1, %d]",
                  walk[i], i + 1, N);
        }
    }

    // Check start and end
    if (walk[0] != 1) {
        quitf(_wa, "Walk starts at %d instead of 1", walk[0]);
    }
    if (walk[K - 1] != N) {
        quitf(_wa, "Walk ends at %d instead of N=%d", walk[K - 1], N);
    }

    // ---------- Validate edges, compute total length and objective O ----------
    long long totalLen = 0;
    set<pair<int,int>> usedEdges;
    long long O = 0;

    for (int i = 0; i + 1 < K; i++) {
        int u = walk[i], v = walk[i + 1];
        auto key = make_pair(min(u, v), max(u, v));
        auto it = edgeMap.find(key);
        if (it == edgeMap.end()) {
            quitf(_wa, "No road between %d and %d (step %d->%d)",
                  u, v, i + 1, i + 2);
        }
        totalLen += (long long)it->second.first;
        if (usedEdges.insert(key).second) {
            O += (long long)it->second.second;
        }
    }

    if (totalLen > T) {
        quitf(_wa, "Total travel length %lld exceeds budget T=%lld", totalLen, T);
    }

    // ---------- Scoring ----------
    // score = floor(1e6 * min(5, O/B))
    // checker ratio = min(5, O/B) / 5   (maps to [0,1] for quitp)
    double ratio_OB = (double)O / (double)baselineB;
    if (ratio_OB < 0.0) ratio_OB = 0.0;
    double clamped = (ratio_OB < 5.0 ? ratio_OB : 5.0);
    double score_ratio = clamped / 5.0;
    if (score_ratio < 0.0) score_ratio = 0.0;
    if (score_ratio > 1.0) score_ratio = 1.0;

    quitp(score_ratio,
          "OK walk K=%d len=%lld T=%lld O=%lld B=%lld O/B=%.6f Ratio: %.9f",
          K, totalLen, T, O, baselineB, ratio_OB, score_ratio);

    return 0;
}