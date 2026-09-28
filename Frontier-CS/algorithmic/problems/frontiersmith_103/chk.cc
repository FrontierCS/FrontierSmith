#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

bool isSubseq(const string &pat, const string &txt) {
    int pi = 0;
    for (int i = 0; i < (int)txt.size() && pi < (int)pat.size(); i++) {
        if (txt[i] == pat[pi]) pi++;
    }
    return pi == (int)pat.size();
}

int main(int argc, char *argv[]) {
    registerTestlibCmd(argc, argv);

    // ---------- read problem input ----------
    int N = inf.readInt();
    int Q = inf.readInt();
    int C = inf.readInt();
    (void)C;
    string S = inf.readToken();
    string T = inf.readToken();

    vector<long long> w(Q);
    vector<string> R(Q);
    for (int j = 0; j < Q; j++) {
        w[j] = inf.readLong();
        R[j] = inf.readToken();
    }

    // ---------- read & validate participant output ----------
    string X = ouf.readToken();

    if ((int)X.size() != N) {
        quitf(_wa, "Output length is %d but expected %d", (int)X.size(), N);
    }
    for (int i = 0; i < N; i++) {
        if (X[i] != '0' && X[i] != '1') {
            quitf(_wa, "Invalid character '%c' at position %d (1-indexed)", X[i], i + 1);
        }
    }

    // no trailing tokens allowed
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra characters found after the binary string");
    }

    // ---------- build output channels ----------
    string A = S, B = T;
    for (int i = 0; i < N; i++) {
        if (X[i] == '1') {
            A[i] = T[i];
            B[i] = S[i];
        }
    }

    // ---------- compute V (participant) ----------
    long long V = 0;
    for (int j = 0; j < Q; j++) {
        if (isSubseq(R[j], A) && isSubseq(R[j], B)) {
            V += w[j];
        }
    }

    // ---------- compute baseline (X = 000...0) ----------
    long long baseline = 0;
    for (int j = 0; j < Q; j++) {
        if (isSubseq(R[j], S) && isSubseq(R[j], T)) {
            baseline += w[j];
        }
    }

    // ---------- compute U (theoretical max) ----------
    long long U = 0;
    for (int j = 0; j < Q; j++) {
        U += w[j];
    }

    // ---------- compute score exactly as statement specifies ----------
    // score = floor(10^9 * clamp((V - baseline) / (U - baseline), 0, 1))
    // special case: if U == baseline, score = 10^9 for any feasible output
    long long score_int;
    if (U == baseline) {
        score_int = 1000000000LL;
    } else {
        double raw = (double)(V - baseline) / (double)(U - baseline);
        if (raw < 0.0) raw = 0.0;
        if (raw > 1.0) raw = 1.0;
        score_int = (long long)floor(raw * 1e9);
        // clamp to [0, 10^9]
        if (score_int < 0LL) score_int = 0LL;
        if (score_int > 1000000000LL) score_int = 1000000000LL;
    }

    // quitp expects a value in [0, 1]; the judge reads Ratio: and multiplies by 1e9
    // We pass score_int / 1e9 so the judge recovers exactly score_int
    double ratio = (double)score_int / 1e9;

    quitp(ratio,
          "V=%lld baseline=%lld U=%lld score=%lld Ratio: %.9f",
          V, baseline, U, score_int, ratio);
}