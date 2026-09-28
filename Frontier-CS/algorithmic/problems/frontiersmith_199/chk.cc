#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input from inf ----
    int n = inf.readInt();
    int m = inf.readInt();
    long long B = inf.readLong();

    vector<string> grid(n);
    for (int i = 0; i < n; i++) {
        grid[i] = inf.readToken();
    }

    vector<vector<long long>> cost(n, vector<long long>(m, 0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            cost[i][j] = inf.readLong();
        }
    }

    // Compute baseline Z = number of initially clear cells
    long long Z = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            if (grid[i][j] == '0') Z++;

    // ---- Read participant output from ouf ----
    if (ouf.seekEof()) {
        // Empty output treated as k=0
        // score_test = min(1000000, floor(100000 * 0 / max(1,Z))) = 0
        quitp(0.0, "Empty output: k=0 Obj=0 Z=%lld score=0/1000000 Ratio: 0.000000", Z);
    }

    int k = ouf.readInt(0, n * m, "k");

    struct Rect { int a, b, c, d; };
    vector<Rect> rects(k);

    for (int r = 0; r < k; r++) {
        int a = ouf.readInt(1, n, "a");
        int b = ouf.readInt(1, m, "b");
        int c = ouf.readInt(1, n, "c");
        int d = ouf.readInt(1, m, "d");
        if (a > c) {
            quitf(_wa, "Rectangle %d: row start %d > row end %d", r+1, a, c);
        }
        if (b > d) {
            quitf(_wa, "Rectangle %d: col start %d > col end %d", r+1, b, d);
        }
        rects[r] = {a, b, c, d};
    }

    // Require end-of-file after consuming all rectangles
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output after the %d rectangles", k);
    }

    // ---- Validate feasibility ----

    // Check cell-disjoint and mark coverage
    vector<vector<int>> covered(n, vector<int>(m, 0));
    for (int r = 0; r < k; r++) {
        int ra = rects[r].a - 1;
        int rb = rects[r].b - 1;
        int rc = rects[r].c - 1;
        int rd = rects[r].d - 1;
        for (int i = ra; i <= rc; i++) {
            for (int j = rb; j <= rd; j++) {
                if (covered[i][j]) {
                    quitf(_wa,
                        "Infeasible: cell (%d,%d) is covered by more than one rectangle",
                        i+1, j+1);
                }
                covered[i][j] = 1;
            }
        }
    }

    // Compute total cleaning cost
    long long total_cost = 0;
    bool overflow = false;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (covered[i][j] && grid[i][j] == '1') {
                if (total_cost > (long long)2e18 - cost[i][j]) {
                    overflow = true;
                    total_cost = (long long)2e18;
                } else {
                    total_cost += cost[i][j];
                }
            }
        }
    }

    if (overflow || total_cost > B) {
        quitp(0.0,
            "Infeasible: cleaning cost %lld exceeds budget %lld. "
            "Obj=0 Z=%lld score=0/1000000 Ratio: 0.000000",
            total_cost, B, Z);
    }

    // ---- Compute objective ----
    long long obj = 0;
    for (int r = 0; r < k; r++) {
        long long h = (long long)(rects[r].c - rects[r].a + 1);
        long long w = (long long)(rects[r].d - rects[r].b + 1);
        long long area = h * w;
        long long contrib = area * area;
        // Guard overflow (area <= 14400, contrib <= ~2e8, sum <= ~2e8 * 14400 well within ll)
        if (obj > (long long)4e18 - contrib) {
            obj = (long long)4e18;
        } else {
            obj += contrib;
        }
    }

    // ---- Compute score using EXACT statement formula ----
    // Score_test = min(1000000, floor(100000 * Obj / max(1, Z)))
    long long denom = max(1LL, Z);
    long long score_test;
    // Avoid overflow: if obj >= 10 * denom then score would be >= 1000000
    if (obj >= (long long)10 * denom) {
        score_test = 1000000LL;
    } else {
        // 100000 * obj: obj < 10*denom <= 10*14400 = 144000, so 100000*obj < 1.44e10, safe
        score_test = (100000LL * obj) / denom;
        if (score_test > 1000000LL) score_test = 1000000LL;
    }

    // ratio passed to quitp must be in [0,1]
    double ratio = (double)score_test / 1000000.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
        "Feasible: k=%d cost=%lld/%lld Obj=%lld Z=%lld score=%lld/1000000 Ratio: %f",
        k, total_cost, B, obj, Z, score_test, ratio);

    return 0;
}