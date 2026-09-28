#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;

ll computeScore(
    const vector<int>& a,
    const vector<vector<int>>& belts_plates, // indices (0-based into a)
    const vector<ll>& L,
    const vector<ll>& c,
    const vector<ll>& q,
    const vector<ll>& b
) {
    int m = (int)belts_plates.size();
    ll total = 0;
    for (int t = 0; t < m; t++) {
        const vector<int>& pl = belts_plates[t];
        int sz = (int)pl.size();
        // belt preference bonus
        for (int j = 0; j < sz; j++) {
            ll g = __gcd((ll)a[pl[j]], q[t]);
            if (g > 1) total += b[t];
        }
        // adjacency harmony bonus
        for (int j = 0; j + 1 < sz; j++) {
            ll g = __gcd((ll)a[pl[j]], (ll)a[pl[j+1]]);
            if (g > 1) total += c[t] * (g - 1);
        }
    }
    return total;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read input
    int n = inf.readInt();
    int m = inf.readInt();

    vector<int> a(n);
    for (int i = 0; i < n; i++) a[i] = inf.readInt();

    vector<ll> L(m), c(m), q(m), b(m);
    for (int t = 0; t < m; t++) {
        L[t] = inf.readLong();
        c[t] = inf.readLong();
        q[t] = inf.readLong();
        b[t] = inf.readLong();
    }

    // ---- Read and validate participant output ----
    vector<bool> used(n + 1, false);
    vector<vector<int>> part_belts(m);

    for (int t = 0; t < m; t++) {
        part_belts[t].resize(L[t]);
        for (int j = 0; j < (int)L[t]; j++) {
            int idx = ouf.readInt(1, n, ("plate index on belt " + to_string(t+1)).c_str());
            if (used[idx]) {
                quitf(_wa, "Plate %d appears more than once in output", idx);
            }
            used[idx] = true;
            part_belts[t][j] = idx - 1; // 0-based
        }
        ouf.readEoln();
    }
    ouf.readEof();

    // Check all plates used
    for (int i = 1; i <= n; i++) {
        if (!used[i]) {
            quitf(_wa, "Plate %d is never placed on any belt", i);
        }
    }

    // ---- Compute participant score Y ----
    ll Y = computeScore(a, part_belts, L, c, q, b);

    // ---- Compute baseline score B ----
    // Baseline: sort plates by (a_i, i) increasing, fill belt 1 then 2 ... m in order
    vector<int> order(n);
    iota(order.begin(), order.end(), 0); // 0-based
    sort(order.begin(), order.end(), [&](int x, int y) {
        if (a[x] != a[y]) return a[x] < a[y];
        return x < y;
    });

    vector<vector<int>> base_belts(m);
    int pos = 0;
    for (int t = 0; t < m; t++) {
        base_belts[t].resize(L[t]);
        for (int j = 0; j < (int)L[t]; j++) {
            base_belts[t][j] = order[pos++];
        }
    }

    ll B = computeScore(a, base_belts, L, c, q, b);

    // ---- Compute ratio ----
    // TestScore = 100 * min(2, (Y+1)/(B+1))
    // With subtask score = 200, ratio in [0,1]:
    // ratio = min(1.0, (Y+1) / (2*(B+1)))
    double ratio = (double)(Y + 1) / (double)(2.0 * (B + 1));
    if (ratio > 1.0) ratio = 1.0;
    if (ratio < 0.0) ratio = 0.0;

    quitp(ratio, "Y=%lld B=%lld Ratio: %.6f", Y, B, ratio);

    return 0;
}