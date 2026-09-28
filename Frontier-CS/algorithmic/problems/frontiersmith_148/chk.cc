#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read input ---
    int D = inf.readInt();
    int E = inf.readInt();
    int H = inf.readInt();
    int m = inf.readInt();
    int k = inf.readInt();

    vector<long long> c(D);
    for (int j = 0; j < D; j++) c[j] = inf.readLong();

    vector<vector<int>> alarms(D);
    for (int j = 0; j < D; j++) {
        int q = inf.readInt();
        alarms[j].resize(q);
        for (int p = 0; p < q; p++) {
            alarms[j][p] = inf.readInt();
        }
        sort(alarms[j].begin(), alarms[j].end());
    }

    // --- Read participant output ---
    vector<int> x(D);
    for (int j = 0; j < D; j++) {
        x[j] = ouf.readInt(0, 1, ("x[" + to_string(j+1) + "]").c_str());
    }
    // Check no trailing output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output after %d values", D);
    }

    // --- Feasibility check via sliding window ---
    vector<int> activeCnt(H + 2, 0);
    for (int j = 0; j < D; j++) {
        if (x[j] == 1) {
            for (int t : alarms[j]) activeCnt[t]++;
        }
    }

    if (H >= m) {
        long long ws = 0;
        for (int t = 1; t <= m; t++) ws += activeCnt[t];
        if (ws >= (long long)k) {
            quitf(_wa, "Infeasible: window [1,%d] has %lld >= %d active alarm starts", m, ws, k);
        }
        for (int s = 2; s <= H - m + 1; s++) {
            ws -= activeCnt[s - 1];
            ws += activeCnt[s + m - 1];
            if (ws >= (long long)k) {
                quitf(_wa, "Infeasible: window [%d,%d] has %lld >= %d active alarm starts",
                      s, s + m - 1, ws, k);
            }
        }
    }

    // --- Compute participant cost C ---
    long long C = 0;
    for (int j = 0; j < D; j++) {
        if (x[j] == 0) C += c[j];
    }

    // --- Compute baseline B ---
    // Reset activeCnt for baseline simulation
    fill(activeCnt.begin(), activeCnt.end(), 0);
    vector<bool> baseOn(D, true);
    long long B = 0;

    for (int j = 0; j < D; j++) {
        for (int t : alarms[j]) activeCnt[t]++;
    }

    while (true) {
        // Find smallest s with window sum >= k
        int violS = -1;
        if (H >= m) {
            long long ws = 0;
            for (int t = 1; t <= m; t++) ws += activeCnt[t];
            if (ws >= (long long)k) {
                violS = 1;
            } else {
                for (int s = 2; s <= H - m + 1; s++) {
                    ws -= activeCnt[s - 1];
                    ws += activeCnt[s + m - 1];
                    if (ws >= (long long)k) {
                        violS = s;
                        break;
                    }
                }
            }
        }

        if (violS == -1) break; // feasible

        // For each active device, count alarms in [violS, violS+m-1]
        int wEnd = violS + m - 1;
        int bestJ = -1;
        long long bestRNum = 0;
        long long bestRDen = 1;

        for (int j = 0; j < D; j++) {
            if (!baseOn[j]) continue;
            // Count alarms in [violS, wEnd] using binary search
            int r = (int)(lower_bound(alarms[j].begin(), alarms[j].end(), wEnd + 1) -
                          lower_bound(alarms[j].begin(), alarms[j].end(), violS));
            if (r == 0) continue;

            if (bestJ == -1) {
                bestJ = j;
                bestRNum = c[j];
                bestRDen = (long long)r;
            } else {
                // Compare c[j]/r vs bestRNum/bestRDen
                // c[j]/r < bestRNum/bestRDen  <=>  c[j]*bestRDen < bestRNum*r
                __int128 lhs = (__int128)c[j] * bestRDen;
                __int128 rhs = (__int128)bestRNum * (long long)r;
                bool better = false;
                if (lhs < rhs) {
                    better = true;
                } else if (lhs == rhs) {
                    // tie on ratio: smaller c_j
                    if (c[j] < c[bestJ]) {
                        better = true;
                    } else if (c[j] == c[bestJ] && j < bestJ) {
                        better = true;
                    }
                }
                if (better) {
                    bestJ = j;
                    bestRNum = c[j];
                    bestRDen = (long long)r;
                }
            }
        }

        if (bestJ == -1) {
            // Should not happen on valid input
            break;
        }

        // Turn off bestJ
        baseOn[bestJ] = false;
        B += c[bestJ];
        for (int t : alarms[bestJ]) activeCnt[t]--;
    }

    // --- Compute score ---
    // Formula from statement: score = 100 * 2^(-C / max(1, B))
    // As a ratio in [0,1]: ratio = 2^(-C / max(1, B))
    // Special case: if B=0, max(1,B)=1, so ratio = 2^(-C)
    //   - C=0 => ratio=1.0 (score=100) as stated
    //   - C>0 => ratio=2^(-C) which is tiny but still correct

    long long denom = (B > 0) ? B : 1LL;
    double ratio = pow(2.0, -(double)C / (double)denom);
    // Clamp to [0,1] for safety (should always be in range)
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio, "Feasible. C=%lld B=%lld Ratio: %.9f", C, B, ratio);

    return 0;
}