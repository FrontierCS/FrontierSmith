#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Count all (possibly overlapping) occurrences of pattern in text
long long countOcc(const string& text, const string& pat) {
    if (pat.empty()) return 0;
    long long cnt = 0;
    size_t pos = 0;
    while ((pos = text.find(pat, pos)) != string::npos) {
        ++cnt;
        ++pos;
    }
    return cnt;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read problem input ---
    int n = inf.readInt();
    int K = inf.readInt();
    int B = inf.readInt();
    int m = inf.readInt();

    string A = inf.readToken();
    if ((int)A.size() != n)
        quitf(_fail, "Input string A has length %d, expected %d", (int)A.size(), n);

    vector<string> pats(m);
    vector<long long> wts(m);
    long long W = 0;
    for (int i = 0; i < m; i++) {
        pats[i] = inf.readToken();
        wts[i]  = inf.readLong();
        W += wts[i];
    }

    // --- Read participant output ---
    // Read t carefully; if missing or bad, score 0
    int t = ouf.readInt(0, K, "t");

    string S = A;
    long long totalLen = 0;

    for (int op = 0; op < t; op++) {
        int l = ouf.readInt(1, n, "l");
        int r = ouf.readInt(l, n, "r");
        long long segLen = (long long)(r - l + 1);
        totalLen += segLen;
        if (totalLen > (long long)B) {
            quitf(_wa,
                  "Operation %d: cumulative reversed length %lld exceeds B=%d",
                  op + 1, totalLen, B);
        }
        // Apply reversal (1-indexed -> 0-indexed)
        reverse(S.begin() + (l - 1), S.begin() + r);
    }

    // NOTE: We intentionally do NOT call ouf.readEof() here.
    // Solutions commonly output a trailing newline after the last reversal,
    // and strict EOF checking would wrongly reject valid output.

    // --- Compute Obj(S) and Base(A) ---
    long long Obj  = 0;
    long long Base = 0;
    for (int i = 0; i < m; i++) {
        long long occS = countOcc(S,    pats[i]);
        long long occA = countOcc(A,    pats[i]);
        Obj  += wts[i] * occS;
        Base += wts[i] * occA;
    }

    // --- Score formula from the statement ---
    // score = round(1,000,000 * clamp(0, (Obj + W) / (Base + W), 2))
    //
    // We report a ratio in [0, 1] via quitp so that:
    //   ratio 0.5 -> judge yields 1,000,000  (baseline)
    //   ratio 1.0 -> judge yields 2,000,000  (cap)
    //   ratio 0.0 -> judge yields 0
    //
    // raw = clamp(0, (Obj+W)/(Base+W), 2)  is in [0,2]
    // reportRatio = raw / 2.0              is in [0,1]

    double denom = (double)(Base + W);
    double raw;
    if (denom <= 0.0) {
        // Degenerate: no patterns or all zero weights; treat as baseline
        raw = 1.0;
    } else {
        raw = (double)(Obj + W) / denom;
    }
    if (raw < 0.0) raw = 0.0;
    if (raw > 2.0) raw = 2.0;

    double reportRatio = raw / 2.0; // maps [0,2] -> [0,1]

    quitp(reportRatio,
          "Ratio: %.10f | Obj=%lld Base=%lld W=%lld t=%d totalLen=%lld",
          reportRatio, Obj, Base, W, t, totalLen);

    return 0;
}