#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Returns sorted list of primes whose exponent in x is odd (square-free signature primes)
vector<int> sfPrimes(int x) {
    vector<int> primes;
    for (int p = 2; (long long)p * p <= x; p++) {
        int e = 0;
        while (x % p == 0) { e++; x /= p; }
        if (e & 1) primes.push_back(p);
    }
    if (x > 1) primes.push_back(x);
    return primes;
}

// Size of symmetric difference of two sorted prime lists
int symDiffSize(const vector<int>& A, const vector<int>& B) {
    int i = 0, j = 0, cnt = 0;
    while (i < (int)A.size() && j < (int)B.size()) {
        if (A[i] == B[j]) { i++; j++; }
        else if (A[i] < B[j]) { cnt++; i++; }
        else { cnt++; j++; }
    }
    cnt += (int)(A.size() - i) + (int)(B.size() - j);
    return cnt;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    long long totalScore = 0;

    for (int t = 0; t < T; t++) {
        int n = inf.readInt();
        long long C = inf.readLong();
        long long F = inf.readLong();

        vector<int> a(n + 1), w(n + 1);
        for (int i = 1; i <= n; i++) a[i] = inf.readInt();
        for (int i = 1; i <= n; i++) w[i] = inf.readInt();

        // Precompute square-free prime sets for each crystal
        vector<vector<int>> sfP(n + 1);
        for (int i = 1; i <= n; i++) sfP[i] = sfPrimes(a[i]);

        // ------ Compute baseline B ------
        // Group by sf signature; representative = smallest index with that signature
        map<vector<int>, int> sigToRep;
        for (int i = 1; i <= n; i++) {
            if (!sigToRep.count(sfP[i])) sigToRep[sfP[i]] = i;
        }
        map<int, long long> baseGroupW;
        for (int i = 1; i <= n; i++) {
            baseGroupW[sigToRep[sfP[i]]] += w[i];
        }
        long long B = 0;
        for (auto& kv : baseGroupW) B += kv.second * kv.second;
        B -= F * (long long)baseGroupW.size();
        // Problem statement guarantees B > 0
        if (B <= 0) {
            quitf(_fail, "Judge error: baseline B=%lld is not positive for test case %d", B, t + 1);
        }

        // ------ Read participant output ------
        vector<int> b(n + 1);
        for (int i = 1; i <= n; i++) {
            b[i] = ouf.readInt(1, n,
                ("b[" + to_string(i) + "] out of range [1," + to_string(n) + "]").c_str());
        }

        // ------ Compute V ------
        map<int, long long> groupW;
        for (int i = 1; i <= n; i++) groupW[b[i]] += w[i];

        long long resonance = 0;
        for (auto& kv : groupW) resonance += kv.second * kv.second;

        long long tuningCost = 0;
        for (int i = 1; i <= n; i++) {
            int rep = b[i];
            tuningCost += C * (long long)symDiffSize(sfP[i], sfP[rep]);
        }

        long long fixedCost = F * (long long)groupW.size();

        long long V = resonance - tuningCost - fixedCost;

        // Score per test case: clamp(0, floor(1e6 * V / B), 2e6)
        long long scoreTC;
        if (V <= 0) {
            scoreTC = 0;
        } else {
            // V and B can be large; use long double for precision
            long double ratio = (long double)V / (long double)B;
            long long raw = (long long)floorl((long double)1000000.0L * ratio);
            scoreTC = max(0LL, min(raw, 2000000LL));
        }

        totalScore += scoreTC;
    }

    // Strict EOF check: reject any trailing non-whitespace output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after all expected tokens");
    }

    // Final score per statement: floor(totalScore / T), result in [0, 2000000]
    // Normalize to [0, 1] for quitp
    long long finalScore = totalScore / (long long)T;  // integer floor division
    finalScore = max(0LL, min(finalScore, 2000000LL));

    double reportRatio = (double)finalScore / 2000000.0;
    reportRatio = max(0.0, min(1.0, reportRatio));

    quitp(reportRatio,
          "Ratio: %.9f (final_score=%lld, total_score=%lld, T=%d)",
          reportRatio, finalScore, totalScore, T);

    return 0;
}