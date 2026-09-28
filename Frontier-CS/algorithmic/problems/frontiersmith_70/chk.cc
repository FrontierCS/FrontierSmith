#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read problem input from inf ─────────────────────────────────────────
    int V = inf.readInt();
    int E = inf.readInt();
    long long H = inf.readLong();

    vector<long long> W(V + 1, 0);
    for (int i = 1; i <= V; i++) W[i] = inf.readLong();

    vector<long long> P(V + 1, 0), F(V + 1, 0);
    for (int i = 1; i <= V; i++) {
        P[i] = inf.readLong();
        F[i] = inf.readLong();
    }

    map<pair<int,int>, long long> edgeLen;
    vector<vector<pair<int,long long>>> adj(V + 1);
    for (int j = 0; j < E; j++) {
        int u = inf.readInt();
        int v = inf.readInt();
        long long L = inf.readLong();
        edgeLen[{u, v}] = L;
        edgeLen[{v, u}] = L;
        adj[u].push_back({v, L});
        adj[v].push_back({u, L});
    }

    // ── Shortest distances from vertex 1 (Dijkstra) ────────────────────────
    const long long BIG = (long long)4e18;
    vector<long long> distFrom1(V + 1, BIG);
    distFrom1[1] = 0;
    {
        priority_queue<pair<long long,int>,
                       vector<pair<long long,int>>,
                       greater<pair<long long,int>>> pq;
        pq.push({0LL, 1});
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d > distFrom1[u]) continue;
            for (auto [nb, len] : adj[u]) {
                if (distFrom1[u] + len < distFrom1[nb]) {
                    distFrom1[nb] = distFrom1[u] + len;
                    pq.push({distFrom1[nb], nb});
                }
            }
        }
    }

    // ── Compute baseline B ──────────────────────────────────────────────────
    long long T = 0;
    for (int i = 2; i <= V; i++) T += W[i];

    long long B = T;  // "do nothing"
    for (int i = 2; i <= V; i++) {
        if (distFrom1[i] < BIG && 2LL * distFrom1[i] <= H) {
            long long cand = T - W[i];
            if (cand < B) B = cand;
        }
    }

    // ── Read participant output from ouf ────────────────────────────────────
    // Format:
    //   K
    //   v_1 v_2 ... v_K
    //   b_1 b_2 ... b_K

    // Read K
    int K = ouf.readInt(1, 200000, "K");

    // Read vertex sequence
    vector<int> vseq(K);
    for (int i = 0; i < K; i++) {
        vseq[i] = ouf.readInt(1, V, "v[i]");
    }

    // Read buy sequence
    vector<int> bseq(K);
    for (int i = 0; i < K; i++) {
        bseq[i] = ouf.readInt(0, 1, "b[i]");
    }

    // ── Validate tour ───────────────────────────────────────────────────────

    // Must start and end at vertex 1
    if (vseq[0] != 1) {
        quitf(_wa, "Tour must start at vertex 1, but starts at %d", vseq[0]);
    }
    if (vseq[K - 1] != 1) {
        quitf(_wa, "Tour must end at vertex 1, but ends at %d", vseq[K - 1]);
    }

    long long fuel = H;
    long long C_fuel = 0;
    vector<bool> visited(V + 1, false);
    vector<bool> bought(V + 1, false);

    visited[1] = true;

    // Process b[0]: buy at starting vertex (v_1 = 1) before first move
    if (bseq[0] == 1) {
        int cur = vseq[0];
        if (F[cur] == 0) {
            quitf(_wa, "b[1]=1 but vertex %d has no fuel package", cur);
        }
        if (bought[cur]) {
            quitf(_wa, "b[1]=1 but package at vertex %d already purchased", cur);
        }
        bought[cur] = true;
        C_fuel += P[cur];
        fuel = min(H, fuel + F[cur]);
    }

    // Process moves t=0..K-2: move from v[t] to v[t+1], then optionally buy at v[t+1]
    for (int t = 0; t < K - 1; t++) {
        int u = vseq[t];
        int w = vseq[t + 1];

        // Check edge exists
        auto it = edgeLen.find({u, w});
        if (it == edgeLen.end()) {
            quitf(_wa, "No edge between %d and %d (step %d -> %d)", u, w, t + 1, t + 2);
        }
        long long L = it->second;
        fuel -= L;
        if (fuel < 0) {
            quitf(_wa,
                  "Fuel became negative (%lld) moving from %d to %d at step %d",
                  fuel, u, w, t + 1);
        }

        visited[w] = true;

        // Buy at destination w, triggered by b[t+1]
        if (bseq[t + 1] == 1) {
            if (F[w] == 0) {
                quitf(_wa, "b[%d]=1 but vertex %d has no fuel package", t + 2, w);
            }
            if (bought[w]) {
                quitf(_wa,
                      "b[%d]=1 but package at vertex %d already purchased",
                      t + 2, w);
            }
            bought[w] = true;
            C_fuel += P[w];
            fuel = min(H, fuel + F[w]);
        }
    }

    // ── Compute objective Y ─────────────────────────────────────────────────
    long long C_penalty = 0;
    for (int i = 2; i <= V; i++) {
        if (!visited[i]) C_penalty += W[i];
    }
    long long Y = C_fuel + C_penalty;

    // ── Scoring formula from problem statement ──────────────────────────────
    // score_int = min(10^9, floor(10^6 * (B+1) / (Y+1)))
    // ratio = score_int / 10^9, clamped to [0,1]
    long long score_int = 0;
    {
        // Use __int128 to avoid overflow: 10^6 * (B+1) can be up to ~5*10^14
        __int128 num = (__int128)1000000LL * (B + 1LL);
        __int128 den = (__int128)(Y + 1LL);
        __int128 s = num / den;
        if (s > (__int128)1000000000LL) s = (__int128)1000000000LL;
        if (s < 0) s = 0;
        score_int = (long long)s;
    }

    double ratio = (double)score_int / 1000000000.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
          "Feasible. Y=%lld (C_fuel=%lld, C_penalty=%lld), B=%lld, "
          "score=%lld. Ratio: %.9f",
          Y, C_fuel, C_penalty, B, score_int, ratio);

    return 0;
}