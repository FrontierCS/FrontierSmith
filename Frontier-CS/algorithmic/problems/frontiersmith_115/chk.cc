#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

struct Trace {
    int a, b, r, c;
};

struct Demand {
    int s, t, w;
};

// Union-Find
struct UF {
    vector<int> p, rnk;
    UF(int n) : p(n), rnk(n, 0) { iota(p.begin(), p.end(), 0); }
    int find(int x) { return p[x] == x ? x : p[x] = find(p[x]); }
    void unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return;
        if (rnk[a] < rnk[b]) swap(a, b);
        p[b] = a;
        if (rnk[a] == rnk[b]) rnk[a]++;
    }
    bool same(int a, int b) { return find(a) == find(b); }
};

// Solve linear system Ax=b via Gaussian elimination, return false if singular
static bool gaussianSolve(vector<vector<double>> A, vector<double>& x) {
    int n = (int)A.size();
    x.assign(n, 0.0);
    for (int col = 0; col < n; col++) {
        int pivot = -1;
        double best = 0.0;
        for (int row = col; row < n; row++) {
            if (fabs(A[row][col]) > best) {
                best = fabs(A[row][col]);
                pivot = row;
            }
        }
        if (pivot == -1 || best < 1e-12) return false;
        swap(A[col], A[pivot]);
        double inv = 1.0 / A[col][col];
        for (int j = col; j <= n; j++) A[col][j] *= inv;
        for (int row = 0; row < n; row++) {
            if (row == col) continue;
            double f = A[row][col];
            for (int j = col; j <= n; j++) A[row][j] -= f * A[col][j];
        }
    }
    for (int i = 0; i < n; i++) x[i] = A[i][n];
    return true;
}

// Compute effective resistance between s and t (0-indexed) in a graph
// with given conductances. Returns -1 if disconnected, 0 if s==t.
static double effectiveResistance(int N, vector<tuple<int,int,double>>& edges, int s, int t) {
    if (s == t) return 0.0;
    if (N <= 1) return -1.0;

    // Build Laplacian
    vector<vector<double>> L(N, vector<double>(N, 0.0));
    for (auto& [u, v, g] : edges) {
        L[u][u] += g;
        L[v][v] += g;
        L[u][v] -= g;
        L[v][u] -= g;
    }

    // Fix node t = 0 voltage: remove row/col t, set RHS for current injection at s
    int sz = N - 1;
    auto idx = [&](int k) -> int { return k < t ? k : k - 1; };

    vector<vector<double>> A(sz, vector<double>(sz + 1, 0.0));
    for (int i = 0; i < N; i++) {
        if (i == t) continue;
        int ii = idx(i);
        for (int j = 0; j < N; j++) {
            if (j == t) continue;
            A[ii][idx(j)] += L[i][j];
        }
        if (i == s) A[ii][sz] += 1.0;
    }

    vector<double> x;
    if (!gaussianSolve(A, x)) return -1.0;

    // Voltage at s
    int si = idx(s);
    return x[si]; // v[s] - v[t] = v[s] - 0
}

// Compute objective F for a given installed set
static double computeF(int n, int /*m*/, int d,
                       const vector<Trace>& traces,
                       const vector<Demand>& demands,
                       const vector<bool>& installed) {
    // Step 1: contract 0-ohm components
    UF uf(n);
    for (int i = 0; i < (int)traces.size(); i++) {
        if (installed[i] && traces[i].r == 0) {
            uf.unite(traces[i].a - 1, traces[i].b - 1);
        }
    }

    // Build supernode mapping
    // Map each representative to a compressed index
    map<int, int> repToSuper;
    int numSuper = 0;
    for (int i = 0; i < n; i++) {
        int rep = uf.find(i);
        if (repToSuper.find(rep) == repToSuper.end()) {
            repToSuper[rep] = numSuper++;
        }
    }

    // Build conductance graph on supernodes
    // Accumulate parallel conductances
    map<pair<int,int>, double> condMap;
    for (int i = 0; i < (int)traces.size(); i++) {
        if (!installed[i]) continue;
        if (traces[i].r == 0) continue; // already contracted
        int su = repToSuper[uf.find(traces[i].a - 1)];
        int sv = repToSuper[uf.find(traces[i].b - 1)];
        if (su == sv) continue; // within same supernode (shouldn't happen for r=1 edge, but safe)
        if (su > sv) swap(su, sv);
        condMap[{su, sv}] += 1.0; // conductance = 1/resistance = 1/1 = 1
    }

    vector<tuple<int,int,double>> edges;
    for (auto& [uv, g] : condMap) {
        edges.emplace_back(uv.first, uv.second, g);
    }

    double F = 0.0;
    for (int j = 0; j < d; j++) {
        int sSuper = repToSuper[uf.find(demands[j].s - 1)];
        int tSuper = repToSuper[uf.find(demands[j].t - 1)];
        double R;
        if (sSuper == tSuper) {
            R = 0.0;
        } else {
            double er = effectiveResistance(numSuper, edges, sSuper, tSuper);
            if (er < 0.0) {
                R = (double)n; // disconnected
            } else {
                R = er;
            }
        }
        F += (double)demands[j].w * R;
    }
    return F;
}

// Compute baseline solution as described in the problem
static vector<bool> computeBaseline(int n, int m, int d, long long B,
                                    const vector<Trace>& traces,
                                    const vector<Demand>& demands) {
    // Sort demand indices by decreasing w, then smaller index
    vector<int> order(d);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int i, int j) {
        if (demands[i].w != demands[j].w) return demands[i].w > demands[j].w;
        return i < j;
    });

    vector<bool> installed(m, false);
    long long B_rem = B;

    // For each demand, find forward path from s to t using a->b directed edges
    // minimizing: (1) total additional cost, (2) total resistance, (3) lex smallest edge indices
    for (int jj : order) {
        int sj = demands[jj].s - 1; // 0-indexed
        int tj = demands[jj].t - 1;

        // Dijkstra on directed graph (only a->b direction)
        // State: (additional_cost, resistance, node)
        // Priority: lex order on (additional_cost, resistance), then lex on edge indices

        const long long INF_COST = 1e18;
        vector<long long> bestCost(n, INF_COST);
        vector<long long> bestRes(n, INF_COST);
        vector<int> prevNode(n, -1);
        vector<int> prevEdge(n, -1);

        bestCost[sj] = 0;
        bestRes[sj] = 0;

        // Use priority queue: (add_cost, resistance, node, last_edge_idx)
        using State = tuple<long long, long long, int>;
        priority_queue<State, vector<State>, greater<State>> pq;
        pq.push({0, 0, sj});

        // Build adjacency list (directed a->b only), sorted by edge index for lex
        vector<vector<pair<int,int>>> adj(n); // adj[u] = {v, edge_idx}
        for (int i = 0; i < m; i++) {
            adj[traces[i].a - 1].push_back({traces[i].b - 1, i});
        }
        // Already sorted by edge index since we process i=0..m-1 in order

        while (!pq.empty()) {
            auto [cost, res, u] = pq.top(); pq.pop();
            if (cost > bestCost[u] || (cost == bestCost[u] && res > bestRes[u])) continue;
            for (auto& [v, ei] : adj[u]) {
                long long addCost = installed[ei] ? 0 : traces[ei].c;
                long long nc = cost + addCost;
                long long nr = res + traces[ei].r;
                if (nc < bestCost[v] || (nc == bestCost[v] && nr < bestRes[v])) {
                    bestCost[v] = nc;
                    bestRes[v] = nr;
                    prevNode[v] = u;
                    prevEdge[v] = ei;
                    pq.push({nc, nr, v});
                }
            }
        }

        if (bestCost[tj] >= INF_COST) continue; // no path
        if (bestCost[tj] > B_rem) continue; // over budget

        // Install path
        int cur = tj;
        while (cur != sj) {
            int ei = prevEdge[cur];
            if (!installed[ei]) {
                installed[ei] = true;
                B_rem -= traces[ei].c;
            }
            cur = prevNode[cur];
        }
    }
    return installed;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read input
    int n = inf.readInt();
    int m = inf.readInt();
    int d = inf.readInt();
    long long B = inf.readLong();

    vector<Trace> traces(m);
    for (int i = 0; i < m; i++) {
        traces[i].a = inf.readInt();
        traces[i].b = inf.readInt();
        traces[i].r = inf.readInt();
        traces[i].c = inf.readInt();
    }

    vector<Demand> demands(d);
    for (int j = 0; j < d; j++) {
        demands[j].s = inf.readInt();
        demands[j].t = inf.readInt();
        demands[j].w = inf.readInt();
    }

    // Read participant output
    int k = ouf.readInt(0, m, "k");

    vector<int> chosen(k);
    set<int> chosenSet;
    for (int i = 0; i < k; i++) {
        chosen[i] = ouf.readInt(1, m, "trace index");
        if (chosenSet.count(chosen[i])) {
            quitf(_wa, "duplicate trace index %d", chosen[i]);
        }
        chosenSet.insert(chosen[i]);
    }

    // Do NOT call readEof() -- solutions output trailing newlines which is fine

    // Check budget
    long long totalCost = 0;
    for (int idx : chosen) totalCost += traces[idx - 1].c;
    if (totalCost > B) {
        quitf(_wa, "total cost %lld exceeds budget %lld", totalCost, B);
    }

    // Build installed arrays (0-indexed internally)
    vector<bool> installedP(m, false);
    for (int idx : chosen) installedP[idx - 1] = true;

    // Compute participant's F
    double F = computeF(n, m, d, traces, demands, installedP);

    // Compute baseline
    vector<bool> installedBase = computeBaseline(n, m, d, B, traces, demands);
    double Fbase = computeF(n, m, d, traces, demands, installedBase);

    // Compute score_ratio in [0,1] per the problem's formula:
    // score = 2,000,000 if F=0
    // score = floor(1,000,000 * min(2, Fbase/F)) otherwise
    // map to [0,1]: score / 2,000,000
    double score_ratio;
    if (F <= 1e-12) {
        // F = 0 => perfect score = 2,000,000 / 2,000,000 = 1.0
        score_ratio = 1.0;
        quitp(score_ratio,
              "F=0 (perfect), Fbase=%.6f. Ratio: %.6f",
              Fbase, score_ratio);
    } else if (Fbase <= 1e-12) {
        // baseline is 0 but participant is not: ratio = 0
        score_ratio = 0.0;
        quitp(score_ratio,
              "F=%.6f, Fbase=0 (baseline perfect, participant not). Ratio: %.6f",
              F, score_ratio);
    } else {
        // score = floor(1,000,000 * min(2, Fbase/F))
        // normalized to [0,1]: score / 2,000,000 = min(2, Fbase/F) / 2
        double raw = Fbase / F;
        if (raw > 2.0) raw = 2.0;
        if (raw < 0.0) raw = 0.0;
        score_ratio = raw / 2.0; // in [0, 1]
        quitp(score_ratio,
              "F=%.6f, Fbase=%.6f, Fbase/F=%.6f, min(2,ratio)=%.6f. Ratio: %.6f",
              F, Fbase, Fbase / F, raw, score_ratio);
    }

    return 0;
}