#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p, rnk;
    DSU(int n) : p(n), rnk(n, 0) { iota(p.begin(), p.end(), 0); }
    int find(int x) { return p[x] == x ? x : p[x] = find(p[x]); }
    bool unite(int x, int y) {
        x = find(x); y = find(y);
        if (x == y) return false;
        if (rnk[x] < rnk[y]) swap(x, y);
        p[y] = x;
        if (rnk[x] == rnk[y]) rnk[x]++;
        return true;
    }
    bool same(int x, int y) { return find(x) == find(y); }
};

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input ----
    int N = inf.readInt();
    int M = inf.readInt();
    long long Lambda = inf.readLong();

    vector<long long> A(N + 1);
    for (int i = 1; i <= N; i++) A[i] = inf.readLong();

    vector<int> EU(M + 1), EV(M + 1);
    vector<long long> EL(M + 1);
    for (int i = 1; i <= M; i++) {
        EU[i] = inf.readInt();
        EV[i] = inf.readInt();
        EL[i] = inf.readLong();
    }

    long long totalA = 0;
    for (int i = 1; i <= N; i++) totalA += A[i];

    // ---- Read participant output ----
    vector<int> chosen(N - 1);
    for (int i = 0; i < N - 1; i++)
        chosen[i] = ouf.readInt(1, M, "edge index out of range [1,M]");

    string S = ouf.readToken();

    // Assert end of stream
    if (!ouf.seekEof())
        quitf(_wa, "Unexpected trailing output after color string");

    // ---- Validate edge indices distinct ----
    {
        set<int> es(chosen.begin(), chosen.end());
        if ((int)es.size() != N - 1)
            quitf(_wa, "Edge indices are not distinct");
    }

    // ---- Validate color string ----
    if ((int)S.size() != N - 1)
        quitf(_wa, "Color string length %d != N-1=%d", (int)S.size(), N - 1);
    for (char c : S)
        if (c != 'W' && c != 'B')
            quitf(_wa, "Invalid character '%c' in color string", c);

    // ---- Validate spanning tree ----
    {
        DSU dsu(N + 1);
        for (int i = 0; i < N - 1; i++) {
            int e = chosen[i];
            if (dsu.same(EU[e], EV[e]))
                quitf(_wa, "Selected edges contain a cycle (edge %d)", e);
            dsu.unite(EU[e], EV[e]);
        }
        for (int i = 2; i <= N; i++)
            if (!dsu.same(1, i))
                quitf(_wa, "Selected edges do not form a spanning tree (vertex %d disconnected)", i);
    }

    // ---- Build adjacency list for submitted tree ----
    vector<vector<pair<int, long long>>> subTree(N + 1);
    for (int i = 0; i < N - 1; i++) {
        int e = chosen[i];
        subTree[EU[e]].push_back({EV[e], EL[e]});
        subTree[EV[e]].push_back({EU[e], EL[e]});
    }

    // ---- Compute CommCost for submitted tree ----
    long long subCommCost = 0;
    {
        // subtree weight sum DFS
        function<long long(int, int)> dfs = [&](int u, int par) -> long long {
            long long s = A[u];
            for (auto [v, len] : subTree[u]) {
                if (v == par) continue;
                long long sub = dfs(v, u);
                subCommCost += len * sub * (totalA - sub);
                s += sub;
            }
            return s;
        };
        dfs(1, -1);
    }

    // ---- Compute Wload, Bload, Penalty for submitted solution ----
    long long Wload = 0, Bload = 0;
    for (int i = 0; i < N - 1; i++) {
        if (S[i] == 'W') Wload += EL[chosen[i]];
        else             Bload += EL[chosen[i]];
    }
    long long subPenalty = Lambda * max(Wload, Bload);
    long long subCost    = subCommCost + subPenalty;

    // ---- Compute baseline ----
    // Step 1: Kruskal MST sorted by (L_i, i)
    vector<int> eorder(M);
    iota(eorder.begin(), eorder.end(), 1);
    sort(eorder.begin(), eorder.end(), [&](int a, int b) {
        return EL[a] != EL[b] ? EL[a] < EL[b] : a < b;
    });

    vector<int> mstEdges;
    {
        DSU dsu(N + 1);
        for (int e : eorder) {
            if (!dsu.same(EU[e], EV[e])) {
                dsu.unite(EU[e], EV[e]);
                mstEdges.push_back(e);
                if ((int)mstEdges.size() == N - 1) break;
            }
        }
    }

    // Step 2: Build baseline tree adjacency list
    vector<vector<pair<int, long long>>> baseTree(N + 1);
    for (int e : mstEdges) {
        baseTree[EU[e]].push_back({EV[e], EL[e]});
        baseTree[EV[e]].push_back({EU[e], EL[e]});
    }

    // Step 3: Compute baseline CommCost
    long long baseCommCost = 0;
    {
        function<long long(int, int)> dfs = [&](int u, int par) -> long long {
            long long s = A[u];
            for (auto [v, len] : baseTree[u]) {
                if (v == par) continue;
                long long sub = dfs(v, u);
                baseCommCost += len * sub * (totalA - sub);
                s += sub;
            }
            return s;
        };
        dfs(1, -1);
    }

    // Step 4: Color baseline edges greedily
    // Sort tree edges by (-L_i, i)
    vector<int> colorOrder = mstEdges;
    sort(colorOrder.begin(), colorOrder.end(), [&](int a, int b) {
        return EL[a] != EL[b] ? EL[a] > EL[b] : a < b;
    });

    long long bW = 0, bB = 0;
    for (int e : colorOrder) {
        // assign to crew with smaller load; ties -> W
        if (bW <= bB) bW += EL[e];
        else          bB += EL[e];
    }
    long long basePenalty = Lambda * max(bW, bB);
    long long baseCost    = baseCommCost + basePenalty;

    // ---- Compute score ratio ----
    // score = floor(1,000,000 * min(5, baseCost / subCost))
    // ratio in [0,1] = score / 5,000,000

    double rawRatio = 0.0;
    if (subCost <= 0) {
        // Shouldn't happen under problem constraints, but guard it
        rawRatio = 1.0;
    } else {
        rawRatio = (double)baseCost / (double)subCost;
        if (rawRatio > 5.0) rawRatio = 5.0;
    }
    double scoreRatio = rawRatio / 5.0;
    if (scoreRatio < 0.0) scoreRatio = 0.0;
    if (scoreRatio > 1.0) scoreRatio = 1.0;

    quitp(scoreRatio,
          "Ratio: %.9f | SubCost: %lld (CommCost=%lld Penalty=%lld) | BaseCost: %lld (CommCost=%lld Penalty=%lld)",
          scoreRatio,
          subCost, subCommCost, subPenalty,
          baseCost, baseCommCost, basePenalty);
}