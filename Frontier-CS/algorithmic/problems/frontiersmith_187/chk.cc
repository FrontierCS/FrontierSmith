#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// KMP occurrence count (overlapping allowed)
long long countOcc(const string &text, const string &pat) {
    if (pat.empty() || text.size() < pat.size()) return 0LL;
    int pm = (int)pat.size();
    int tn = (int)text.size();
    vector<int> fail(pm, 0);
    for (int i = 1; i < pm; i++) {
        int j = fail[i-1];
        while (j > 0 && pat[i] != pat[j]) j = fail[j-1];
        if (pat[i] == pat[j]) j++;
        fail[i] = j;
    }
    long long cnt = 0;
    int j = 0;
    for (int i = 0; i < tn; i++) {
        while (j > 0 && text[i] != pat[j]) j = fail[j-1];
        if (text[i] == pat[j]) j++;
        if (j == pm) {
            cnt++;
            j = fail[j-1];
        }
    }
    return cnt;
}

// Helper: score ratio given OBJ and BASE (per problem statement formula)
// Score = clamp(0, 1e6, 500000 + 500000 * (OBJ - BASE) / max(1, BASE))
// ratio = score / 1e6
double computeRatio(long long OBJ, long long BASE) {
    long long denom = max(1LL, BASE);
    double raw = 500000.0 + 500000.0 * (double)(OBJ - BASE) / (double)denom;
    double scoreVal = max(0.0, min(1000000.0, raw));
    return scoreVal / 1000000.0;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input from inf ----
    int n   = inf.readInt();
    int m   = inf.readInt();
    int X   = inf.readInt();
    long long B = inf.readLong();

    vector<int> par(n+1, 0);
    for (int v = 2; v <= n; v++)
        par[v] = inf.readInt();

    string A = inf.readToken(); // length n-1; A[v-2] = initial digit char for edge (v, par[v])
    vector<char> initDigit(n+1, '0');
    for (int v = 2; v <= n; v++)
        initDigit[v] = A[v-2];

    vector<long long> edgeCost(n+1, 0LL);
    for (int v = 2; v <= n; v++)
        edgeCost[v] = inf.readLong();

    struct Route {
        int x, y;
        long long w;
        string S;
    };
    vector<Route> routes(m);
    for (int i = 0; i < m; i++) {
        routes[i].x = inf.readInt();
        routes[i].y = inf.readInt();
        routes[i].w = inf.readLong();
        routes[i].S = inf.readToken();
    }

    // ---- Build LCA (binary lifting) ----
    const int LOG = 20;
    vector<array<int,20>> up(n+1);
    for (int v = 0; v <= n; v++) up[v].fill(0);
    vector<int> dep(n+1, 0);
    vector<vector<int>> children(n+1);
    for (int v = 2; v <= n; v++)
        children[par[v]].push_back(v);

    // BFS to set depths and binary lifting table
    {
        queue<int> q;
        q.push(1);
        dep[1] = 0;
        up[1][0] = 1;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int k = 1; k < LOG; k++)
                up[u][k] = up[up[u][k-1]][k-1];
            for (int cv : children[u]) {
                dep[cv] = dep[u] + 1;
                up[cv][0] = u;
                q.push(cv);
            }
        }
    }

    auto lca = [&](int u, int v) -> int {
        if (dep[u] < dep[v]) swap(u, v);
        int diff = dep[u] - dep[v];
        for (int k = 0; k < LOG; k++)
            if ((diff >> k) & 1) u = up[u][k];
        if (u == v) return u;
        for (int k = LOG-1; k >= 0; k--)
            if (up[u][k] != up[v][k]) { u = up[u][k]; v = up[v][k]; }
        return up[u][0];
    };

    auto kthAnc = [&](int u, int k) -> int {
        for (int i = 0; i < LOG; i++)
            if ((k >> i) & 1) u = up[u][i];
        return u;
    };

    // For each route, precompute the path as a list of nodes (edges represented
    // by their child node). Path from x to y: up-leg then down-leg.
    // The digit of the edge between node v and par[v] is stored at v.
    // path[i] = list of edge-nodes (child node of each edge), in order x->y.
    vector<vector<int>> pathCache(m);
    for (int i = 0; i < m; i++) {
        int x = routes[i].x, y = routes[i].y;
        int l = lca(x, y);
        vector<int> upLeg, dnLeg;
        // up from x to l (edges: x, par[x], ..., child-of-l)
        {
            int cur = x;
            while (cur != l) {
                upLeg.push_back(cur);
                cur = up[cur][0];
            }
        }
        // down from l to y: collect path from y up to l, then reverse
        {
            int cur = y;
            while (cur != l) {
                dnLeg.push_back(cur);
                cur = up[cur][0];
            }
            reverse(dnLeg.begin(), dnLeg.end());
        }
        pathCache[i] = upLeg;
        pathCache[i].insert(pathCache[i].end(), dnLeg.begin(), dnLeg.end());
        (void)kthAnc; // suppress unused warning
    }

    auto buildString = [&](const vector<int>& path, const vector<char>& digit) -> string {
        string s;
        s.reserve(path.size());
        for (int v : path) s += digit[v];
        return s;
    };

    // ---- Compute BASE (no recoloring) ----
    long long BASE = 0;
    for (int i = 0; i < m; i++) {
        string T = buildString(pathCache[i], initDigit);
        BASE += routes[i].w * countOcc(T, routes[i].S);
    }

    // ---- Helper: score with OBJ=0 (infeasible/malformed) ----
    auto quitInfeasible = [&](const char* fmt, ...) {
        // OBJ = 0 as per statement
        double ratio = computeRatio(0LL, BASE);
        // We must report the score correctly; infeasible => OBJ=0
        quitp(ratio, "INFEASIBLE: %s | OBJ=0 BASE=%lld Ratio: %.6f", fmt, BASE, ratio);
    };
    (void)quitInfeasible; // we'll inline this below

    // ---- Read participant output ----
    // Statement: "If the output is malformed or infeasible, its objective value is 0."
    // No special-casing for empty output — just read k normally.

    int k;
    if (ouf.seekEof()) {
        // Truly empty file — malformed (missing k). OBJ = 0.
        double ratio = computeRatio(0LL, BASE);
        quitp(ratio, "Malformed: empty output. OBJ=0 BASE=%lld Ratio: %.6f", BASE, ratio);
    }

    k = ouf.readInt();
    if (k < 0 || k > n-1) {
        double ratio = computeRatio(0LL, BASE);
        quitp(ratio, "Infeasible: k=%d out of range [0,%d]. OBJ=0 BASE=%lld Ratio: %.6f",
              k, n-1, BASE, ratio);
    }

    vector<char> finalDigit = initDigit;
    set<int> seen;
    bool infeasible = false;
    string infReason;

    for (int i = 0; i < k && !infeasible; i++) {
        if (ouf.seekEof()) {
            infeasible = true;
            infReason = "Fewer recoloring pairs than k";
            break;
        }
        int v = ouf.readInt();
        if (v < 2 || v > n) {
            infeasible = true;
            infReason = "Node v=" + to_string(v) + " out of range [2,n]";
            break;
        }
        if (ouf.seekEof()) {
            infeasible = true;
            infReason = "Missing digit for node " + to_string(v);
            break;
        }
        int d = ouf.readInt();
        if (d < 1 || d > 9) {
            infeasible = true;
            infReason = "Digit d=" + to_string(d) + " out of range [1,9]";
            break;
        }
        if (seen.count(v)) {
            infeasible = true;
            infReason = "Node " + to_string(v) + " listed more than once";
            break;
        }
        seen.insert(v);
        finalDigit[v] = (char)('0' + d);
    }

    if (infeasible) {
        double ratio = computeRatio(0LL, BASE);
        quitp(ratio, "Infeasible: %s. OBJ=0 BASE=%lld Ratio: %.6f",
              infReason.c_str(), BASE, ratio);
    }

    // ---- Check for trailing garbage ----
    if (!ouf.seekEof()) {
        double ratio = computeRatio(0LL, BASE);
        quitp(ratio, "Infeasible: extra output after %d recoloring pairs. OBJ=0 BASE=%lld Ratio: %.6f",
              k, BASE, ratio);
    }

    // ---- Check budget ----
    long long totalCost = 0;
    for (int v : seen) {
        if (finalDigit[v] != initDigit[v]) {
            totalCost += edgeCost[v];
            if (totalCost < 0) totalCost = (long long)2e18; // overflow guard
        }
    }
    if (totalCost > B) {
        double ratio = computeRatio(0LL, BASE);
        quitp(ratio, "Infeasible: total cost %lld exceeds budget B=%lld. OBJ=0 BASE=%lld Ratio: %.6f",
              totalCost, B, BASE, ratio);
    }

    // ---- Compute participant OBJ ----
    long long OBJ = 0;
    for (int i = 0; i < m; i++) {
        string T = buildString(pathCache[i], finalDigit);
        OBJ += routes[i].w * countOcc(T, routes[i].S);
    }

    // ---- Continuous scoring ----
    // Score = clamp(0, 1e6, 500000 + 500000 * (OBJ - BASE) / max(1, BASE))
    double ratio = computeRatio(OBJ, BASE);

    quitp(ratio, "OBJ=%lld BASE=%lld cost=%lld budget=%lld Ratio: %.6f",
          OBJ, BASE, totalCost, B, ratio);

    return 0;
}