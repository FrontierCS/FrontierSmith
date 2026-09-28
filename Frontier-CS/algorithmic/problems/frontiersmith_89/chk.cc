#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef pair<ll,int> pli;

const ll INF = (ll)4e18;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── read problem input ──────────────────────────────────────────────────
    int n = inf.readInt();
    int e = inf.readInt();
    int q = inf.readInt();
    ll  B = inf.readLong();

    struct Road {
        int x, y;
        ll  r[3];
        ll  c[3];
    };
    vector<Road> roads(e + 1);
    for (int i = 1; i <= e; i++) {
        roads[i].x    = inf.readInt();
        roads[i].y    = inf.readInt();
        roads[i].r[0] = inf.readLong();
        roads[i].r[1] = inf.readLong();
        roads[i].c[1] = inf.readLong();
        roads[i].r[2] = inf.readLong();
        roads[i].c[2] = inf.readLong();
        roads[i].c[0] = 0LL;
    }

    struct Group {
        int s, t;
        ll  w;
        bool mask[10];
    };
    vector<Group> groups(q);
    ll W = 0;
    set<int> originSet;
    for (int j = 0; j < q; j++) {
        groups[j].s = inf.readInt();
        groups[j].t = inf.readInt();
        groups[j].w = inf.readLong();
        string ms   = inf.readToken();
        if ((int)ms.size() != 10)
            quitf(_fail, "mask string length != 10 for group %d", j);
        for (int d = 0; d < 10; d++)
            groups[j].mask[d] = (ms[d] == '1');
        W += groups[j].w;
        originSet.insert(groups[j].s);
    }

    vector<int> origins(originSet.begin(), originSet.end());
    int numOrigins = (int)origins.size();
    map<int,int> originIdx;
    for (int i = 0; i < numOrigins; i++)
        originIdx[origins[i]] = i;

    // ── helper: build adj and run Dijkstra from every distinct origin ───────
    // Returns objective value for given plan vector
    auto computeObj = [&](const vector<int>& plan) -> ll {
        // build directed adjacency list
        vector<vector<pair<int,ll>>> adj(n + 1);
        for (int i = 1; i <= e; i++) {
            int p  = plan[i];
            int xi = roads[i].x;
            int yi = roads[i].y;
            ll  rp = roads[i].r[p];
            adj[xi].push_back({yi, rp});
            adj[yi].push_back({xi, 1000LL - rp});
        }

        // dist[originIdx][node]
        vector<vector<ll>> dist(numOrigins, vector<ll>(n + 1, INF));

        for (int oi = 0; oi < numOrigins; oi++) {
            int src = origins[oi];
            dist[oi][src] = 0;
            priority_queue<pli, vector<pli>, greater<pli>> pq;
            pq.push({0LL, src});
            while (!pq.empty()) {
                auto [d, u] = pq.top(); pq.pop();
                if (d > dist[oi][u]) continue;
                for (auto [v, w] : adj[u]) {
                    ll nd = d + w;
                    if (nd < dist[oi][v]) {
                        dist[oi][v] = nd;
                        pq.push({nd, v});
                    }
                }
            }
        }

        ll obj = 0;
        for (int j = 0; j < q; j++) {
            int oi = originIdx[groups[j].s];
            ll dj  = dist[oi][groups[j].t];
            if (dj < INF) {
                int digit = (int)(dj % 10LL);
                if (groups[j].mask[digit])
                    obj += groups[j].w;
            }
        }
        return obj;
    };

    // ── compute BASE (all plan 0) ────────────────────────────────────────────
    vector<int> basePlan(e + 1, 0);
    ll BASE = computeObj(basePlan);

    // ── read participant output ──────────────────────────────────────────────
    int k = ouf.readInt(0, e, "k must be in [0, e]");

    vector<int> chosenPlan(e + 1, 0);
    set<int> changed;
    ll totalCost = 0;

    for (int idx = 0; idx < k; idx++) {
        int ri = ouf.readInt(1, e, "road index out of range [1,e]");
        int p  = ouf.readInt(1, 2, "plan must be 1 or 2");
        if (changed.count(ri))
            quitf(_wa, "road %d listed more than once", ri);
        changed.insert(ri);
        chosenPlan[ri] = p;
        ll cost_p = roads[ri].c[p];
        // overflow-safe accumulation
        if (cost_p > 0 && totalCost > B - cost_p)
            totalCost = B + 1; // mark exceeded
        else
            totalCost += cost_p;
    }

    // ── EOF check: reject trailing non-whitespace tokens ─────────────────────
    if (!ouf.seekEof())
        quitf(_wa, "extra output after the %d road-plan pairs", k);

    if (totalCost > B)
        quitf(_wa, "total retrofit cost %lld exceeds budget B=%lld", totalCost, B);

    // ── compute SUB ──────────────────────────────────────────────────────────
    ll SUB = computeObj(chosenPlan);

    // ── scoring formula ───────────────────────────────────────────────────────
    // score per case = 1,000,000              if W == BASE
    //                = round(1e6 * clamp((SUB - BASE) / (W - BASE), 0, 1))  otherwise
    //
    // Since quitp(ratio) awards ratio * (per-case max), and config gives each
    // case a max of 1,000,000, passing ratio directly implements the formula.
    double ratio;
    if (W == BASE) {
        // All commuters already satisfied under baseline; any feasible answer gets full score
        ratio = 1.0;
    } else {
        double num = (double)(SUB  - BASE);
        double den = (double)(W    - BASE);
        ratio = num / den;
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
    }

    // int_score is for human-readable message only; quitp(ratio) is what the
    // judge uses — it awards round(ratio * 1,000,000) because config score=10000000
    // and n_cases=10, giving 1,000,000 per case.
    ll int_score;
    if (W == BASE) {
        int_score = 1000000LL;
    } else {
        int_score = (ll)round(1000000.0 * ratio);
    }

    quitp(ratio,
          "W=%lld BASE=%lld SUB=%lld cost=%lld score=%lld Ratio: %f",
          W, BASE, SUB, totalCost, int_score, ratio);
    return 0;
}