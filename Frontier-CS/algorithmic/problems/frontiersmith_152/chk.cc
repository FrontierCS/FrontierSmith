#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---------- read problem input ----------
    int N = inf.readInt();
    int M = inf.readInt();

    vector<long long> c(N);
    for (int i = 0; i < N; i++) c[i] = inf.readLong();

    vector<long long> Lj(M), Rj(M), Kj(M), Pj(M), Fj(M);
    for (int j = 0; j < M; j++) {
        Lj[j] = inf.readLong();
        Rj[j] = inf.readLong();
        Kj[j] = inf.readLong();
        Pj[j] = inf.readLong();
        Fj[j] = inf.readLong();
    }

    // ---------- derived constants ----------
    long long S = 0;
    for (int i = 0; i < N; i++) S += c[i];

    long long Pmax = 1;
    for (int j = 0; j < M; j++) Pmax = max(Pmax, Pj[j]);

    long long U = (Pmax * S) / 2;

    // ---------- baseline B0 ----------
    // Sort by decreasing value; ties by smaller original index.
    // Greedily assign each banknote to the currently poorer reserve;
    // if equal, assign to Kile.
    vector<int> order(N);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        if (c[a] != c[b]) return c[a] > c[b];
        return a < b;
    });

    long long A0 = 0, B0p = 0;
    for (int idx : order) {
        if (A0 <= B0p) {
            A0 += c[idx];
        } else {
            B0p += c[idx];
        }
    }
    long long B0 = min(A0, B0p);

    // ---------- read participant output ----------
    vector<int> d(N);
    for (int i = 0; i < N; i++) {
        d[i] = ouf.readInt(0, M + 1,
            ("d[" + to_string(i + 1) + "] must be in [0," + to_string(M + 1) + "]").c_str());
    }
    // Ensure no trailing tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra tokens found after the %d required integers", N);
    }

    // ---------- compute objective E ----------
    long long A = 0, B = 0;
    vector<long long> sumJ(M, 0), cntJ(M, 0);

    for (int i = 0; i < N; i++) {
        if (d[i] == 0) {
            A += c[i];
        } else if (d[i] == 1) {
            B += c[i];
        } else {
            int j = d[i] - 2; // 0-indexed table
            sumJ[j] += c[i];
            cntJ[j]++;
        }
    }

    long long C = 0;
    for (int j = 0; j < M; j++) {
        if (sumJ[j] >= Lj[j] && sumJ[j] <= Rj[j] && cntJ[j] <= Kj[j]) {
            long long Tj = Pj[j] * sumJ[j] - Fj[j];
            C += Tj;
        }
        // otherwise T_j = 0, contribute nothing
    }

    // E = min( floor((A+B+C)/2) , min(A,B)+C )
    long long total = A + B + C;
    long long E = min(total / 2, min(A, B) + C);

    // ---------- compute per-test score ratio ----------
    double score_ratio;
    if (U > B0) {
        double raw = (double)(E - B0) / (double)(U - B0);
        score_ratio = max(0.0, min(1.0, raw));
    } else {
        // U == B0 (or U < B0 by coincidence)
        score_ratio = (E >= U) ? 1.0 : 0.0;
    }

    quitp(score_ratio,
          "E=%lld B0=%lld U=%lld A=%lld B=%lld C=%lld Ratio: %.9f",
          E, B0, U, A, B, C, score_ratio);
}