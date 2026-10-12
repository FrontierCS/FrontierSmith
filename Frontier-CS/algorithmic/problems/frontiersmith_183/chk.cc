#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- read problem input ----
    int n = inf.readInt();
    int m = inf.readInt();
    long long B = inf.readLong();

    vector<double> T(m + 1), W(m + 1);
    for (int j = 1; j <= m; j++) {
        T[j] = inf.readDouble();
        W[j] = inf.readDouble();
    }

    vector<long long> sal(n + 1);
    vector<int> cap_limit(n + 1), rec(n + 1), dcnt(n + 1);
    vector<vector<pair<int, double>>> contrib(n + 1);

    for (int i = 1; i <= n; i++) {
        sal[i] = inf.readLong();
        cap_limit[i] = inf.readInt();
        rec[i] = inf.readInt();
        dcnt[i] = inf.readInt();
        contrib[i].resize(dcnt[i]);
        for (int t = 0; t < dcnt[i]; t++) {
            int cj = inf.readInt();
            double aj = inf.readDouble();
            contrib[i][t] = {cj, aj};
        }
    }

    // ---- helper: compute objective from capability totals ----
    auto computeObj = [&](const vector<double>& X) -> double {
        double obj = 0.0;
        for (int j = 1; j <= m; j++) {
            obj += W[j] * (-expm1(-X[j] / T[j]));
        }
        return obj;
    };

    // ---- baseline greedy (exactly as specified) ----
    auto runGreedy = [&]() -> double {
        vector<bool> inS(n + 1, false);
        vector<int> childCount(n + 1, 0);
        long long totalCost = 0;
        vector<double> X(m + 1, 0.0);

        for (;;) {
            double curObj = computeObj(X);
            int best = -1;
            double bestRatio = -1e300, bestDelta = -1e300;
            long long bestSal = LLONG_MAX;

            for (int i = 1; i <= n; i++) {
                if (inS[i]) continue;
                int r = rec[i];
                if (r > 0 && !inS[r]) continue;
                if (r > 0 && childCount[r] >= cap_limit[r]) continue;
                if (totalCost + sal[i] > B) continue;

                // compute delta
                double delta = 0.0;
                for (auto& [cj, aj] : contrib[i]) {
                    delta += W[cj] * (exp(-X[cj] / T[cj]) * -expm1(-aj / T[cj]));
                }

                if (best == -1) {
                    best = i;
                    bestDelta = delta;
                    bestRatio = delta / (double)sal[i];
                    bestSal = sal[i];
                } else {
                    double ratio = delta / (double)sal[i];
                    if (ratio > bestRatio + 1e-15 ||
                        (fabs(ratio - bestRatio) <= 1e-15 && delta > bestDelta + 1e-15) ||
                        (fabs(ratio - bestRatio) <= 1e-15 && fabs(delta - bestDelta) <= 1e-15 && sal[i] < bestSal) ||
                        (fabs(ratio - bestRatio) <= 1e-15 && fabs(delta - bestDelta) <= 1e-15 && sal[i] == bestSal && i < best)) {
                        best = i;
                        bestDelta = delta;
                        bestRatio = ratio;
                        bestSal = sal[i];
                    }
                }
            }

            if (best == -1) break;
            if (bestDelta <= 0.0) break;

            inS[best] = true;
            totalCost += sal[best];
            for (auto& [cj, aj] : contrib[best]) X[cj] += aj;
            if (rec[best] > 0) childCount[rec[best]]++;
        }

        return computeObj(X);
    };

    double G = runGreedy();

    // The answer file stores the objective of a fixed feasible reference team.
    // Missing or degenerate reference data is a judge error, never full credit.
    if (ans.seekEof()) quitf(_fail, "missing reference objective in answer file");
    double R = ans.readDouble();
    if (!ans.seekEof()) quitf(_fail, "extra data after reference objective");
    if (!isfinite(R)) quitf(_fail, "reference objective must be finite");
    double tolerance = 1e-9 * max({1.0, fabs(R), fabs(G)});
    if (R < 0.0 || R + tolerance < G)
        quitf(_fail, "reference objective must be nonnegative and no worse than baseline");
    double upperBound = accumulate(W.begin() + 1, W.end(), 0.0);
    if (R > upperBound + tolerance)
        quitf(_fail, "reference objective exceeds total capability value");

    // ---- read participant output ----
    int q = ouf.readInt();
    if (q < 0 || q > n) {
        quitf(_wa, "invalid q=%d (must be in [0, %d])", q, n);
    }

    vector<int> chosen(q);
    vector<bool> picked(n + 1, false);
    for (int i = 0; i < q; i++) {
        chosen[i] = ouf.readInt();
        if (chosen[i] < 1 || chosen[i] > n) {
            quitf(_wa, "candidate ID %d out of range [1, %d]", chosen[i], n);
        }
        if (picked[chosen[i]]) {
            quitf(_wa, "duplicate candidate id %d", chosen[i]);
        }
        picked[chosen[i]] = true;
    }

    // check EOF
    if (!ouf.seekEof()) {
        quitf(_wa, "extra data after the team selection");
    }

    // ---- validate feasibility ----
    // budget
    long long totalSal = 0;
    for (int i : chosen) totalSal += sal[i];
    if (totalSal > B) {
        quitf(_wa, "budget exceeded: cost=%lld > B=%lld", totalSal, B);
    }

    // referral closure & supervision limit
    vector<int> selectedChildCount(n + 1, 0);
    for (int i : chosen) {
        int r = rec[i];
        if (r > 0) {
            if (!picked[r]) {
                quitf(_wa, "referral closure violated: candidate %d selected but recommender %d not selected", i, r);
            }
            selectedChildCount[r]++;
        }
    }
    for (int i : chosen) {
        if (selectedChildCount[i] > cap_limit[i]) {
            quitf(_wa, "supervision limit violated: candidate %d has %d selected children but cap is %d",
                  i, selectedChildCount[i], cap_limit[i]);
        }
    }

    // ---- compute participant objective ----
    vector<double> X(m + 1, 0.0);
    for (int i : chosen) {
        for (auto& [cj, aj] : contrib[i]) {
            X[cj] += aj;
        }
    }
    double O = computeObj(X);

    // expm1 avoids cancellation for very small contributions.
    // Some original inputs already have an optimal greedy baseline (R == G).
    // Normalize actual quality there, instead of giving all valid teams full credit.
    double raw = R - G > tolerance ? (O - G) / (R - G)
                                  : (R > 0.0 ? O / R : 0.0);
    double ratio = max(0.0, min(1.0, raw));
    quitp(ratio, "O=%.12f G=%.12f R=%.12f Ratio: %.12f", O, G, R, ratio);
}
