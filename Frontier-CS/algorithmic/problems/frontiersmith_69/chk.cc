#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- read problem input ----
    int n = inf.readInt();
    int m = inf.readInt();
    int L = inf.readInt();

    long long ALL = (L == 64) ? (long long)(-1LL) : ((1LL << L) - 1LL);

    vector<long long> M(m + 1);
    vector<long long> C(m + 1);
    for (int j = 1; j <= m; j++) {
        M[j] = inf.readLong();
        C[j] = inf.readLong();
    }

    vector<long long> T(n + 1);
    vector<long long> A(n + 1), B(n + 1);
    for (int i = 1; i <= n; i++) {
        T[i] = inf.readLong();
        A[i] = inf.readLong();
        B[i] = inf.readLong();
    }

    // ---- read participant output (n integers) ----
    vector<int> x(n + 1);
    for (int i = 1; i <= n; i++) {
        x[i] = ouf.readInt(1, m, ("x_" + to_string(i)).c_str());
    }
    // Do NOT call ouf.readEof() — solutions legitimately print a trailing newline.

    // ---- feasibility: all distinct ----
    {
        vector<bool> used(m + 1, false);
        for (int i = 1; i <= n; i++) {
            if (used[x[i]]) {
                quitf(_wa, "Keycard %d used more than once", x[i]);
            }
            used[x[i]] = true;
        }
    }

    // ---- helper: compute objective for an assignment (1-indexed, size n+1) ----
    auto computeObj = [&](const vector<int>& assign) -> long long {
        long long obj = 0LL;
        for (int i = 1; i <= n; i++) {
            obj += C[assign[i]];
            long long Vi = 0LL;
            for (int k = 1; k <= n; k++) {
                if (k != i) Vi |= M[assign[k]];
            }
            Vi &= ALL;
            long long miss  = __builtin_popcountll(T[i] & (~Vi) & ALL);
            long long extra = __builtin_popcountll((~T[i]) & Vi & ALL);
            obj += A[i] * miss + B[i] * extra;
        }
        return obj;
    };

    // ---- participant objective ----
    long long yourObj = computeObj(x);

    // ---- baseline: pick n cheapest keycards (tie-break: smaller index) ----
    vector<int> order(m);
    iota(order.begin(), order.end(), 1); // indices 1..m
    sort(order.begin(), order.end(), [&](int a, int b_) {
        if (C[a] != C[b_]) return C[a] < C[b_];
        return a < b_;
    });

    // Take the first n, sort by index
    vector<int> chosen(order.begin(), order.begin() + n);
    sort(chosen.begin(), chosen.end());

    // Build baseline assignment (1-indexed)
    vector<int> baseAssign(n + 1);
    for (int i = 1; i <= n; i++) {
        baseAssign[i] = chosen[i - 1];
    }

    long long baseObj = computeObj(baseAssign);

    // ---- compute score ratio ----
    // Score = 10^6 * min(5, (Base+1)/(Your+1))
    // We report score_ratio in [0,1]: rawRatio/5 clamped to [0,1].
    double rawRatio = (double)(baseObj + 1) / (double)(yourObj + 1);
    double cappedRatio = min(5.0, rawRatio);
    double scoreRatio = cappedRatio / 5.0;
    if (scoreRatio < 0.0) scoreRatio = 0.0;
    if (scoreRatio > 1.0) scoreRatio = 1.0;

    quitp(scoreRatio,
          "Ratio: %.9f | Your=%lld Base=%lld",
          scoreRatio, yourObj, baseObj);

    return 0;
}