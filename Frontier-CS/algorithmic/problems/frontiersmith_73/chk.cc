#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input from inf ----
    int n, m;
    long long T;
    n = inf.readInt();
    m = inf.readInt();
    T = (long long)inf.readInt();

    long long x0 = inf.readLong();
    long long y0 = inf.readLong();

    vector<long long> ox(m + 1), oy(m + 1);
    ox[0] = x0; oy[0] = y0;
    for (int i = 1; i <= m; i++) {
        ox[i] = inf.readLong();
        oy[i] = inf.readLong();
    }

    vector<long long> sx(n), sy(n);
    for (int i = 0; i < n; i++) {
        sx[i] = inf.readLong();
        sy[i] = inf.readLong();
    }

    // ---- Compute baseline B ----
    // One shot per distinct normalized line from base through any squad
    set<pair<long long, long long>> baselineLines;
    for (int i = 0; i < n; i++) {
        long long dx = sx[i] - x0;
        long long dy = sy[i] - y0;
        // (dx,dy) != (0,0) guaranteed by problem statement
        long long g = __gcd(abs(dx), abs(dy));
        if (g == 0) g = 1;
        dx /= g; dy /= g;
        if (dx < 0 || (dx == 0 && dy < 0)) { dx = -dx; dy = -dy; }
        baselineLines.insert({dx, dy});
    }
    double B = (double)T * (double)(int)baselineLines.size();

    // ---- Read and simulate participant output from ouf ----

    // Read q — must be in [0, 200000] (allow 0 for n=0 edge case)
    int q;
    {
        string tok = ouf.readToken("[0-9]+", "q (number of operations)");
        q = stoi(tok);
        if (q < 0 || q > 200000) {
            quitf(_wa, "q=%d out of range [0,200000]", q);
        }
    }

    int curPos = 0;
    double totalTravel = 0.0;
    int totalShots = 0;
    vector<bool> destroyed(n, false);

    for (int op = 0; op < q; op++) {
        string opType = ouf.readToken();

        if (opType == "M") {
            int j = ouf.readInt(0, m, "outpost index");
            double ddx = (double)(ox[j] - ox[curPos]);
            double ddy = (double)(oy[j] - oy[curPos]);
            totalTravel += sqrt(ddx * ddx + ddy * ddy);
            curPos = j;

        } else if (opType == "S") {
            long long u = ouf.readLong(-1000000000LL, 1000000000LL, "u");
            long long v = ouf.readLong(-1000000000LL, 1000000000LL, "v");
            if (u == 0 && v == 0) {
                quitf(_wa, "Shot direction (0,0) is invalid at operation %d", op + 1);
            }
            totalShots++;
            long long cx = ox[curPos], cy = oy[curPos];
            for (int i = 0; i < n; i++) {
                if (!destroyed[i]) {
                    long long ddx2 = sx[i] - cx;
                    long long ddy2 = sy[i] - cy;
                    __int128 lhs = (__int128)ddx2 * v;
                    __int128 rhs = (__int128)ddy2 * u;
                    if (lhs == rhs) {
                        destroyed[i] = true;
                    }
                }
            }
        } else {
            quitf(_wa, "Unknown operation '%s' at operation %d", opType.c_str(), op + 1);
        }
    }

    // ---- Feasibility checks ----

    // Must return to base
    if (curPos != 0) {
        quitf(_wa, "Han did not return to base after all operations; currently at outpost %d", curPos);
    }

    // All squads must be destroyed
    for (int i = 0; i < n; i++) {
        if (!destroyed[i]) {
            quitf(_wa, "Squad %d at (%lld,%lld) was never destroyed",
                  i + 1, sx[i], sy[i]);
        }
    }

    // ---- Compute score ----
    double C = totalTravel + (double)T * (double)totalShots;

    // ratio = clamp(B / C, 0, 1):
    //   C <= B  -> ratio = 1.0  (at least as good as baseline -> full score)
    //   C > B   -> ratio = B/C  (worse than baseline -> partial credit, continuously)
    // This maps continuously: better solutions get higher ratio up to 1.0
    // (The statement allows up to 2x better than baseline for 2,000,000 pts,
    //  but the judge multiplies ratio by 1,000,000, so we map:
    //   [0, 2*B] cost range -> [0, 1] ratio via ratio = min(1.0, B/C))
    // Actually to honour "2x better = 2,000,000 score", use ratio = min(1, B/C):
    //   C = 0.5*B -> ratio = 1.0 (capped)
    //   C = B     -> ratio = 1.0 (matches baseline, full marks = 1,000,000 per statement)
    // But per problem statement score = 1e6 * clamp(B/C, 0, 2), so
    //   we want quitp ratio in [0,1] = clamp(B/C, 0, 2) / 2
    //   C = B    -> B/C = 1.0 -> ratio = 0.5  (but judge multiplies by 1,000,000... hmm)
    // Per Frontier-CS: quitp(r, ...) means score = r * max_score_of_subtask
    // So to get 1,000,000 for baseline and up to 2,000,000 for 2x better:
    //   ratio = clamp(B/C, 0, 2) / 2
    double bOverC = (C > 1e-12) ? (B / C) : 2.0;
    if (bOverC < 0.0) bOverC = 0.0;
    if (bOverC > 2.0) bOverC = 2.0;
    double ratio = bOverC / 2.0;
    // ratio in [0, 1]: 0.5 = matches baseline, 1.0 = twice as good or better

    quitp((float)ratio,
          "OK travel=%.4f shots=%d cost=%.4f baseline=%.4f bOverC=%.6f Ratio: %.9f",
          totalTravel, totalShots, C, B, bOverC, ratio);

    return 0;
}