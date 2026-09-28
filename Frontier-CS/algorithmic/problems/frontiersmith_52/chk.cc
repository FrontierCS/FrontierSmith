#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Returns true if two chords (a,b) and (c,d) of a regular n-gon strictly
// cross at an interior point (not at a shared endpoint).
// Vertices are 1-indexed, CCW order.
bool chordsStrictlyCross(int a, int b, int c, int d, int n) {
    // Normalize to 0-indexed
    a--; b--; c--; d--;
    // Two chords strictly cross iff their endpoints alternate around the circle.
    // Ensure a < b for the "short arc" canonical form isn't needed;
    // we check if exactly one of {c,d} lies strictly inside the open arc a->b CCW.
    auto inOpenArcCCW = [&](int s, int e2, int x) -> bool {
        if (s == e2) return false;
        if (s < e2) return (x > s && x < e2);
        else        return (x > s || x < e2);
    };
    bool cIn = inOpenArcCCW(a, b, c);
    bool dIn = inOpenArcCCW(a, b, d);
    return cIn != dIn;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input from inf ----
    int n = inf.readInt();
    int K = inf.readInt();
    int m = inf.readInt();

    // cost[i][j]: -1 = forbidden, else construction cost (1-indexed)
    vector<vector<int>> cost(n + 1, vector<int>(n + 1, 0));
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++)
            cost[i][j] = inf.readInt();

    struct Client {
        long long profit;
        vector<int> stations;
    };
    vector<Client> clients(m);
    for (int t = 0; t < m; t++) {
        clients[t].profit = (long long)inf.readInt();
        int s = inf.readInt();
        clients[t].stations.resize(s);
        for (int i = 0; i < s; i++)
            clients[t].stations[i] = inf.readInt();
    }

    // ---- Compute baseline B ----
    // Option 1: build all allowed polygon sides (i, i%n+1) on channel 1
    long long sidesCost = 0;
    bool sidesAllowed = true;
    for (int i = 1; i <= n; i++) {
        int j = (i % n) + 1;
        if (cost[i][j] == -1) { sidesAllowed = false; break; }
        sidesCost += cost[i][j];
    }

    // If sides are allowed, compute profit from that solution (all n stations connected on ch 1)
    long long sidesProfit = 0;
    if (sidesAllowed) {
        // All stations connected on channel 1 => every client served
        for (int t = 0; t < m; t++)
            sidesProfit += clients[t].profit;
    }
    long long sidesObj = sidesAllowed ? (sidesProfit - sidesCost) : (long long)(-1e18);

    // Option 2: empty solution, objective = 0
    long long B = max(0LL, sidesObj);

    // ---- Read participant output from ouf ----
    // First token: number of segments e
    if (ouf.seekEof()) {
        // Empty output => 0 segments => objective 0
        long long O = 0;
        double ratio;
        if (B <= 0) {
            ratio = 1.0;
        } else {
            ratio = 0.0;
        }
        ratio = max(0.0, min(1.0, ratio));
        quitp(ratio, "Empty output (e=0). Objective=0, Baseline=%lld, Ratio: %.6f", B, ratio);
    }

    int e = ouf.readInt(0, n * (n - 1) / 2, "e");

    struct Seg { int u, v, ch; };
    vector<Seg> segs;
    segs.reserve(e);

    set<pair<int,int>> usedPairs;

    for (int i = 0; i < e; i++) {
        int u = ouf.readInt(1, n, "u");
        int v = ouf.readInt(1, n, "v");
        int ch = ouf.readInt(1, K, "ch");

        // u < v required
        if (u >= v)
            quitf(_wa, "Segment %d: need u < v, got u=%d v=%d", i + 1, u, v);

        // Check allowed
        if (cost[u][v] == -1)
            quitf(_wa, "Segment %d: (%d,%d) is forbidden", i + 1, u, v);

        // Check uniqueness
        auto key = make_pair(u, v);
        if (usedPairs.count(key))
            quitf(_wa, "Segment %d: (%d,%d) appears more than once", i + 1, u, v);
        usedPairs.insert(key);

        segs.push_back({u, v, ch});
    }

    // Check no extra output (but be lenient about trailing whitespace/newlines)
    // We do NOT call readEof() strictly; instead we skip any trailing whitespace.
    // If there is a non-whitespace token remaining, that's a format error.
    // Use a soft check: try to read one more token; if it's not EOF, warn.
    // Actually, per the playbook: assert end-of-stream to prevent trailing garbage.
    // We skip whitespace and check EOF.
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra content after the last segment in output");
    }

    // ---- Feasibility check: no two segments cross ----
    for (int i = 0; i < (int)segs.size(); i++) {
        for (int j = i + 1; j < (int)segs.size(); j++) {
            int a = segs[i].u, b = segs[i].v;
            int c = segs[j].u, d = segs[j].v;
            // Shared endpoint is fine
            if (a == c || a == d || b == c || b == d) continue;
            if (chordsStrictlyCross(a, b, c, d, n))
                quitf(_wa, "Segments (%d,%d) and (%d,%d) intersect at interior point", a, b, c, d);
        }
    }

    // ---- Compute objective ----
    long long totalCost = 0;
    for (auto& s : segs)
        totalCost += cost[s.u][s.v];

    // Union-Find per channel
    // parent[ch][node], 1-indexed nodes, ch 1-indexed
    vector<vector<int>> parent(K + 1, vector<int>(n + 1));
    vector<vector<int>> rnk(K + 1, vector<int>(n + 1, 0));
    for (int ch = 1; ch <= K; ch++)
        for (int i = 1; i <= n; i++)
            parent[ch][i] = i;

    function<int(int, int)> find = [&](int ch, int x) -> int {
        if (parent[ch][x] != x) parent[ch][x] = find(ch, parent[ch][x]);
        return parent[ch][x];
    };
    auto unite = [&](int ch, int x, int y) {
        x = find(ch, x); y = find(ch, y);
        if (x == y) return;
        if (rnk[ch][x] < rnk[ch][y]) swap(x, y);
        parent[ch][y] = x;
        if (rnk[ch][x] == rnk[ch][y]) rnk[ch][x]++;
    };

    for (auto& s : segs)
        unite(s.ch, s.u, s.v);

    long long totalProfit = 0;
    for (int t = 0; t < m; t++) {
        bool served = false;
        for (int ch = 1; ch <= K && !served; ch++) {
            int root = find(ch, clients[t].stations[0]);
            bool allSame = true;
            for (int idx = 1; idx < (int)clients[t].stations.size(); idx++) {
                if (find(ch, clients[t].stations[idx]) != root) {
                    allSame = false;
                    break;
                }
            }
            if (allSame) served = true;
        }
        if (served) totalProfit += clients[t].profit;
    }

    long long O = totalProfit - totalCost;

    // ---- Compute score ratio ----
    // Score = clamp((O - B) / (R - B), 0, 1)
    // R is not available; we approximate:
    //   if O >= B  => ratio = 1.0  (matches or beats baseline)
    //   if O < B   => partial credit proportional to how close O is to B
    //                 ratio = max(0, O / B)  when B > 0
    //                 ratio = 0              when B <= 0 and O < 0
    double ratio;
    if (O >= B) {
        ratio = 1.0;
    } else {
        // O < B
        if (B > 0) {
            // Linear partial credit: 0 at O=0 (empty solution) up to 1 at O=B
            // Use (O - 0) / (B - 0) if O >= 0, else 0
            if (O >= 0) {
                ratio = (double)O / (double)B;
            } else {
                // O < 0 < B: worse than empty solution => 0
                ratio = 0.0;
            }
        } else {
            // B <= 0 and O < B => worse than baseline
            // Give ratio = 0
            ratio = 0.0;
        }
    }
    ratio = max(0.0, min(1.0, ratio));

    quitp(ratio,
        "Feasible. e=%d, Cost=%lld, Profit=%lld, Objective=%lld, Baseline=%lld, Ratio: %.6f",
        e, totalCost, totalProfit, O, B, ratio);

    return 0;
}