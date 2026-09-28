#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read input ---
    int N = inf.readInt();
    int n = inf.readInt();
    int K = inf.readInt();
    long long F = inf.readLong();

    vector<long long> S(9), C(9);
    for (int i = 0; i < 9; i++) S[i] = inf.readLong();
    for (int i = 0; i < 9; i++) C[i] = inf.readLong();

    vector<long long> w(N);
    vector<string> r(N);
    for (int i = 0; i < N; i++) {
        w[i] = inf.readLong();
        r[i] = inf.readToken();
    }

    // --- Helpers ---
    auto palDist = [](char x, char y) -> int {
        int d = abs((int)(x - y));
        return min(d, 17 - d);
    };

    auto computeMatch = [&](const string &req, const string &des) -> long long {
        long long best = 0;
        for (int t = 0; t < n; t++) {
            long long val = 0;
            for (int p = 0; p < n; p++) {
                val += S[palDist(req[p], des[(p + t) % n])];
            }
            if (val > best) best = val;
        }
        return best;
    };

    auto designCost = [&](const string &des) -> long long {
        long long cost = F;
        for (int p = 0; p < n; p++) {
            cost += C[palDist(des[p], des[(p + 1) % n])];
        }
        return cost;
    };

    // --- Compute baseline ---
    vector<int> order(N);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        if (w[a] != w[b]) return w[a] > w[b];
        return a < b;
    });

    int m_base = min(N, K);
    vector<string> baseDesigns(m_base);
    for (int i = 0; i < m_base; i++) {
        baseDesigns[i] = r[order[i]];
    }

    long long baseUtility = 0;
    for (int i = 0; i < N; i++) {
        long long best = 0;
        for (int j = 0; j < m_base; j++) {
            long long mv = computeMatch(r[i], baseDesigns[j]);
            if (mv > best) best = mv;
        }
        baseUtility += w[i] * best;
    }

    long long baseCost = 0;
    for (int j = 0; j < m_base; j++) {
        baseCost += designCost(baseDesigns[j]);
    }

    long long BASE_raw = baseUtility - baseCost;
    long long BASE = (BASE_raw > 0) ? BASE_raw : 0LL;

    // --- Compute upper bound ---
    long long Smax = *max_element(S.begin(), S.end());
    long long sumW = 0;
    for (int i = 0; i < N; i++) sumW += w[i];
    long long UB = (long long)n * Smax * sumW;

    // --- Read participant output ---
    int m = ouf.readInt(0, K, "number of designs m");

    vector<string> designs(m);
    for (int j = 0; j < m; j++) {
        string ds = ouf.readToken();
        if ((int)ds.size() != n) {
            quitf(_wa, "Design %d has length %d, expected %d", j + 1, (int)ds.size(), n);
        }
        for (char ch : ds) {
            if (ch < 'A' || ch > 'Q') {
                quitf(_wa, "Design %d has invalid character '%c' (expected A..Q)", j + 1, ch);
            }
        }
        designs[j] = ds;
    }

    // Ensure no trailing tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "Unexpected trailing content in output after reading %d designs", m);
    }

    // --- Compute participant OBJ ---
    long long partUtility = 0;
    for (int i = 0; i < N; i++) {
        long long best = 0;
        for (int j = 0; j < m; j++) {
            long long mv = computeMatch(r[i], designs[j]);
            if (mv > best) best = mv;
        }
        partUtility += w[i] * best;
    }

    long long partCost = 0;
    for (int j = 0; j < m; j++) {
        partCost += designCost(designs[j]);
    }

    long long OBJ = partUtility - partCost;

    // --- Compute score ---
    // score = floor( 1,000,000 * max(0, OBJ - BASE) / max(1, UB - BASE) )
    // clamped to [0, 1,000,000]
    long long denom = (UB - BASE > 1LL) ? (UB - BASE) : 1LL;
    long long numer = (OBJ > BASE) ? (OBJ - BASE) : 0LL;

    // Apply floor and clamp to integer in [0, 1000000]
    long long score_int = (long long)floor(1000000.0 * (double)numer / (double)denom);
    if (score_int < 0LL) score_int = 0LL;
    if (score_int > 1000000LL) score_int = 1000000LL;

    // Convert to ratio in [0,1] for quitp
    double ratio = (double)score_int / 1000000.0;

    quitp(ratio,
          "OBJ=%lld, Utility=%lld, Cost=%lld, BASE=%lld, UB=%lld, score=%lld, Ratio: %.9f",
          OBJ, partUtility, partCost, BASE, UB, score_int, ratio);

    return 0;
}