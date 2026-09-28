#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;

ll computeObjective(const vector<ll>& b, int n, int L, const vector<ll>& w) {
    ll F = 0;
    for (int d = 2; d <= L; d++) {
        ll wd = w[d - 2];
        for (int s = 0; s <= n - d; s++) {
            ll minXor = LLONG_MAX;
            for (int i = s; i < s + d; i++) {
                for (int j = i + 1; j < s + d; j++) {
                    ll x = b[i] ^ b[j];
                    if (x < minXor) minXor = x;
                }
            }
            F += wd * minXor;
        }
    }
    return F;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int t = inf.readInt();

    double totalRatio = 0.0;

    for (int tc = 1; tc <= t; tc++) {
        int n = inf.readInt();
        int L = inf.readInt();

        vector<ll> a(n);
        for (int i = 0; i < n; i++) {
            a[i] = inf.readLong();
        }

        vector<ll> w(L - 1);
        for (int i = 0; i < L - 1; i++) {
            w[i] = inf.readLong();
        }

        // Read n integers from participant output without hard bounds enforcement.
        // We manually validate feasibility and assign score 0 if invalid.
        vector<int> perm(n);
        bool feasible = true;

        for (int i = 0; i < n; i++) {
            // readInt() with no bounds: reads any integer, fails only on non-integer or EOF.
            // We use the lenient form so we can assign score 0 rather than WA the whole run.
            perm[i] = ouf.readInt(-2000000000, 2000000000,
                ("test case " + to_string(tc) + " position " + to_string(i + 1)).c_str());
            if (perm[i] < 1 || perm[i] > n) {
                feasible = false;
            }
        }

        if (feasible) {
            // Check for duplicates / missing values
            vector<bool> seen(n + 1, false);
            for (int i = 0; i < n; i++) {
                if (seen[perm[i]]) {
                    feasible = false;
                    break;
                }
                seen[perm[i]] = true;
            }
        }

        if (!feasible) {
            // This test case scores 0; continue to next
            totalRatio += 0.0;
            continue;
        }

        // Build participant sequence b
        vector<ll> bPart(n);
        for (int i = 0; i < n; i++) {
            bPart[i] = a[perm[i] - 1];
        }

        ll F = computeObjective(bPart, n, L, w);

        // Build baseline: sort indices by (a_i, i) ascending (0-indexed i)
        vector<int> baseIdx(n);
        iota(baseIdx.begin(), baseIdx.end(), 0);
        sort(baseIdx.begin(), baseIdx.end(), [&](int x, int y) {
            if (a[x] != a[y]) return a[x] < a[y];
            return x < y;
        });

        vector<ll> bBase(n);
        for (int i = 0; i < n; i++) {
            bBase[i] = a[baseIdx[i]];
        }

        ll B = computeObjective(bBase, n, L, w);

        // per-test score = 100 * clamp((F+1)/(B+1), 0, 2)
        // map to [0,1] by dividing by 2 for quitp (subtask score = 200)
        double raw = (double)(F + 1) / (double)(B + 1);
        if (raw < 0.0) raw = 0.0;
        if (raw > 2.0) raw = 2.0;
        double perTestRatio = raw / 2.0;

        totalRatio += perTestRatio;
    }

    // Verify no trailing data in participant output
    if (!ouf.seekEof()) {
        quitf(_wa, "Trailing data found in participant output after all test cases");
    }

    // Final ratio = mean of per-test ratios in [0,1]
    double finalRatio = totalRatio / (double)t;
    if (finalRatio < 0.0) finalRatio = 0.0;
    if (finalRatio > 1.0) finalRatio = 1.0;

    // quitp(r, ...) => display score = r * subtask_score = r * 200
    // This implements: mean of [100 * clamp((F+1)/(B+1), 0, 2)] over all test cases
    quitp(finalRatio, "Ratio: %.9f", finalRatio);

    return 0;
}