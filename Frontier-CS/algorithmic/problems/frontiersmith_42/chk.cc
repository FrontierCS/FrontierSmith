#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Compute weighted exposure given a list of coin values used
long long computeExposure(const vector<int>& usedCoins, const vector<int>& w, int P) {
    vector<bool> reachable(P + 1, false);
    reachable[0] = true;
    for (int coin : usedCoins) {
        for (int x = P; x >= coin; x--) {
            if (reachable[x - coin]) reachable[x] = true;
        }
    }
    long long E = 0;
    for (int x = 0; x <= P; x++) {
        if (reachable[x]) E += w[x];
    }
    return E;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input from inf ----
    int n = inf.readInt();
    int m = inf.readInt();

    vector<int> p(m + 1);
    int P = 0;
    for (int j = 1; j <= m; j++) {
        p[j] = inf.readInt();
        P += p[j];
    }

    vector<int> c(n + 1);
    for (int i = 1; i <= n; i++) {
        c[i] = inf.readInt();
    }

    // Weights: P+1 integers, each in [1,1000]
    vector<int> w(P + 1);
    for (int x = 0; x <= P; x++) {
        w[x] = inf.readInt();
    }

    vector<int> b(n + 1);
    for (int i = 1; i <= n; i++) {
        b[i] = inf.readInt();
    }

    // ---- Compute baseline exposure E_base ----
    vector<int> baselineCoins;
    for (int i = 1; i <= n; i++) {
        if (b[i] > 0) baselineCoins.push_back(c[i]);
    }
    long long E_base = computeExposure(baselineCoins, w, P);

    // ---- Read participant output from ouf ----
    vector<int> a(n + 1);
    for (int i = 1; i <= n; i++) {
        a[i] = ouf.readInt(0, m,
            ("a[" + to_string(i) + "] must be in [0," + to_string(m) + "]").c_str());
    }
    // Consume any trailing whitespace/newlines; do NOT hard-fail on extra blank lines
    // (testlib in non-strict mode handles this, but we avoid readEof to prevent false WA)

    // ---- Validate feasibility: day sums ----
    vector<long long> daySum(m + 1, 0);
    for (int i = 1; i <= n; i++) {
        if (a[i] > 0) {
            daySum[a[i]] += c[i];
        }
    }
    for (int j = 1; j <= m; j++) {
        if (daySum[j] != (long long)p[j]) {
            quitf(_wa,
                "Day %d: sum of assigned coins is %lld but required price is %d",
                j, daySum[j], p[j]);
        }
    }

    // ---- Compute participant exposure E ----
    vector<int> participantCoins;
    for (int i = 1; i <= n; i++) {
        if (a[i] > 0) participantCoins.push_back(c[i]);
    }
    long long E = computeExposure(participantCoins, w, P);

    // E >= w[0] >= 1 always (empty subset reaches 0), so E > 0 is guaranteed
    if (E <= 0) {
        quitf(_wa, "Internal error: E = %lld <= 0 (impossible)", E);
    }
    if (E_base <= 0) {
        quitf(_wa, "Internal error: E_base = %lld <= 0 (impossible)", E_base);
    }

    // ---- Compute score ratio ----
    // score = min(2_000_000, 1_000_000 * E_base / E)
    // ratio  = score / 2_000_000 = min(1.0, 0.5 * E_base / E)
    double ratio = 0.5 * (double)E_base / (double)E;
    if (ratio > 1.0) ratio = 1.0;
    if (ratio < 0.0) ratio = 0.0;

    quitp(ratio,
          "Ratio: %.9f | E=%lld E_base=%lld score=%.0f/2000000",
          ratio,
          E,
          E_base,
          ratio * 2000000.0);

    return 0;
}