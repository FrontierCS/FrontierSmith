#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input ----
    int M = inf.readInt(1, 4000, "M");
    int C = inf.readInt(1, 1200, "C");

    vector<long long> v(M + 1);
    for (int j = 1; j <= M; j++) {
        v[j] = inf.readLong(1LL, 10000000LL, "v_j");
    }

    vector<long long> t(C + 1);
    vector<long long> w(C + 1);
    long long W = 0;
    for (int i = 1; i <= C; i++) {
        t[i] = inf.readLong(1LL, 1000000000LL, "t_i");
        w[i] = inf.readLong(1LL, 1000000LL, "w_i");
        W += w[i];
    }

    // ---- Read participant output ----
    vector<int> a(M + 1);
    for (int j = 1; j <= M; j++) {
        a[j] = ouf.readInt(0, C, "a_j");
    }
    // Assert no trailing tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra tokens after the %d required integers", M);
    }

    // ---- Compute objective O ----
    long long O = 0;
    for (int i = 1; i <= C; i++) {
        long long sum = 0;
        set<long long> denoms;
        bool dup = false;
        for (int j = 1; j <= M; j++) {
            if (a[j] == i) {
                if (denoms.count(v[j])) { dup = true; break; }
                denoms.insert(v[j]);
                sum += v[j];
            }
        }
        if (!dup && sum == t[i]) {
            O += w[i];
        }
    }

    // ---- Compute baseline B ----
    // Sort children: larger w first, then smaller t, then smaller index
    vector<int> order(C);
    iota(order.begin(), order.end(), 1);
    sort(order.begin(), order.end(), [&](int x, int y) {
        if (w[x] != w[y]) return w[x] > w[y];
        if (t[x] != t[y]) return t[x] < t[y];
        return x < y;
    });

    vector<bool> used(M + 1, false);
    long long B = 0;

    for (int ci : order) {
        long long rem = t[ci];

        // Gather unused coins, sorted: denom desc, index asc
        vector<int> cands;
        cands.reserve(M);
        for (int j = 1; j <= M; j++) {
            if (!used[j]) cands.push_back(j);
        }
        sort(cands.begin(), cands.end(), [&](int x, int y) {
            if (v[x] != v[y]) return v[x] > v[y];
            return x < y;
        });

        set<long long> taken_denoms;
        vector<int> taken_coins;

        for (int j : cands) {
            if (rem == 0) break;
            if (v[j] <= rem && !taken_denoms.count(v[j])) {
                taken_denoms.insert(v[j]);
                taken_coins.push_back(j);
                rem -= v[j];
            }
        }

        if (rem == 0) {
            B += w[ci];
            for (int j : taken_coins) used[j] = true;
        }
        // else discard tentative selection (coins remain unused)
    }

    // ---- Compute integer score in [0, 1 000 000] per statement ----
    // Statement formula:
    //   if B == W : 1 000 000 if O == W, else 0
    //   else      : floor(1 000 000 * max(0, O - B) / (W - B)),  clamped to [0, 1 000 000]
    long long score_int;
    if (B == W) {
        score_int = (O == W) ? 1000000LL : 0LL;
    } else {
        long long gain = O - B;
        if (gain <= 0LL) {
            score_int = 0LL;
        } else {
            // Integer floor division: 1 000 000 * gain fits in long long
            // (gain <= W <= 1200 * 1e6 = 1.2e9; 1e6 * 1.2e9 = 1.2e15 < 9.2e18)
            score_int = 1000000LL * gain / (W - B);
            if (score_int > 1000000LL) score_int = 1000000LL;
        }
    }

    // Convert integer score to ratio in [0, 1] for quitp
    double ratio = (double)score_int / 1000000.0;

    quitp(ratio,
          "O=%lld B=%lld W=%lld Score=%lld | Ratio: %.9f",
          O, B, W, score_int, ratio);

    return 0;
}