#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;

ll computeObjective(const vector<ll>& b, const vector<pair<int,int>>& specs) {
    int n = (int)b.size();
    ll total = 0;
    for (auto& sp : specs) {
        int w = sp.first;
        int c = sp.second;
        if (w > n) continue;
        for (int l = 0; l <= n - w; l++) {
            ll minXor = (ll)4e18;
            for (int x = l; x < l + w; x++) {
                for (int y = x + 1; y < l + w; y++) {
                    ll xv = b[x] ^ b[y];
                    if (xv < minXor) minXor = xv;
                }
            }
            total += (ll)c * minXor;
        }
    }
    return total;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int t = inf.readInt();

    double totalScore = 0.0;
    int totalCases = t;

    for (int tc = 0; tc < t; tc++) {
        int n = inf.readInt();
        int m = inf.readInt();

        vector<ll> a(n);
        for (int i = 0; i < n; i++) {
            a[i] = inf.readLong();
        }

        vector<pair<int,int>> specs(m);
        for (int j = 0; j < m; j++) {
            specs[j].first  = inf.readInt();
            specs[j].second = inf.readInt();
        }

        // Read participant permutation
        vector<int> perm(n);
        bool feasible = true;
        for (int i = 0; i < n; i++) {
            if (ouf.seekEof()) {
                feasible = false;
                for (int k = i; k < n; k++) perm[k] = 1;
                break;
            }
            int v = ouf.readInt();
            if (v < 1 || v > n) {
                feasible = false;
                perm[i] = 1;
                for (int k = i + 1; k < n; k++) {
                    if (!ouf.seekEof()) ouf.readInt();
                    perm[k] = 1;
                }
                break;
            }
            perm[i] = v;
        }

        // Check it is a valid permutation
        if (feasible) {
            vector<bool> seen(n + 1, false);
            for (int i = 0; i < n; i++) {
                if (seen[perm[i]]) {
                    feasible = false;
                    break;
                }
                seen[perm[i]] = true;
            }
        }

        // Compute baseline: stable sort (a_i, i) by a_i asc, ties by i asc
        vector<pair<ll,int>> sorted_pairs(n);
        for (int i = 0; i < n; i++) sorted_pairs[i] = {a[i], i};
        stable_sort(sorted_pairs.begin(), sorted_pairs.end(),
            [](const pair<ll,int>& x, const pair<ll,int>& y) {
                if (x.first != y.first) return x.first < y.first;
                return x.second < y.second;
            });

        vector<ll> baseline_b(n);
        for (int i = 0; i < n; i++) {
            baseline_b[i] = sorted_pairs[i].first;
        }

        ll B = computeObjective(baseline_b, specs);

        double caseScore = 0.0;
        if (!feasible) {
            caseScore = 0.0;
        } else {
            vector<ll> part_b(n);
            for (int i = 0; i < n; i++) {
                part_b[i] = a[perm[i] - 1];
            }

            ll O = computeObjective(part_b, specs);

            // Score_case = min(1000, 100 * (O+1) / (B+1))
            double score_case = 100.0 * (double)(O + 1) / (double)(B + 1);
            if (score_case > 1000.0) score_case = 1000.0;
            if (score_case < 0.0) score_case = 0.0;

            caseScore = score_case;
        }

        totalScore += caseScore;
    }

    // Strict EOF check: reject any trailing tokens in participant output
    if (!ouf.seekEof()) {
        quitf(_wa, "extra output after all test cases");
    }

    // meanScore is in [0, 1000]; return ratio in [0, 1].
    // config score=1000, so displayed score = ratio * 1000 = meanScore.
    double meanScore = totalScore / (double)totalCases;
    double finalRatio = meanScore / 1000.0;
    if (finalRatio < 0.0) finalRatio = 0.0;
    if (finalRatio > 1.0) finalRatio = 1.0;

    quitp(finalRatio, "Ratio: %.9f (mean score=%.4f/1000 over %d cases)", finalRatio, meanScore, totalCases);
}