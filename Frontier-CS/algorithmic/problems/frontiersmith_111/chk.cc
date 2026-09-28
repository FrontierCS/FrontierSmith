#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read input ----
    int N = inf.readInt();
    int M = inf.readInt();

    vector<long long> A(N+1), B(N+1);
    for (int i = 1; i <= N; i++) {
        A[i] = inf.readLong();
        B[i] = inf.readLong();
    }

    // transition map: key=(u,v), value={RR,RB,BR,BB}
    map<pair<int,int>, array<long long,4>> trans;
    for (int k = 0; k < M; k++) {
        int u = inf.readInt();
        int v = inf.readInt();
        long long rr = inf.readLong();
        long long rb = inf.readLong();
        long long br = inf.readLong();
        long long bb = inf.readLong();
        trans[{u,v}] = {rr, rb, br, bb};
    }

    // ---- Compute baseline F_base ----
    vector<int> baseOri(N+1);
    vector<long long> W(N+1);
    for (int i = 1; i <= N; i++) {
        if (A[i] >= B[i]) { baseOri[i] = 0; W[i] = A[i]; }
        else               { baseOri[i] = 1; W[i] = B[i]; }
    }
    vector<int> baseOrder(N);
    iota(baseOrder.begin(), baseOrder.end(), 1);
    sort(baseOrder.begin(), baseOrder.end(), [&](int x, int y){
        if (W[x] != W[y]) return W[x] > W[y];
        return x < y;
    });
    long long F_base = 0;
    for (int j = 0; j < N; j++) {
        int pl = baseOrder[j];
        F_base += (baseOri[pl] == 0 ? A[pl] : B[pl]);
    }
    for (int j = 0; j+1 < N; j++) {
        int u = baseOrder[j], v = baseOrder[j+1];
        int ou = baseOri[u], ov = baseOri[v];
        auto it = trans.find({u,v});
        if (it != trans.end()) {
            F_base += it->second[ou*2 + ov];
        }
    }
    if (F_base <= 0) {
        quitf(_fail, "F_base is non-positive: %lld", F_base);
    }

    // ---- Read participant output using readLine to avoid EOF issues ----
    // Line 1: permutation
    string line1 = ouf.readLine();
    // Line 2: binary string
    string line2 = ouf.readLine();
    // Now assert EOF (allow trailing whitespace)
    ouf.readEof();

    // ---- Parse permutation from line1 ----
    vector<int> P;
    {
        istringstream iss(line1);
        int x;
        while (iss >> x) {
            P.push_back(x);
        }
    }
    if ((int)P.size() != N) {
        quitf(_wa, "Permutation has %d elements, expected %d", (int)P.size(), N);
    }

    // ---- Parse binary string from line2 ----
    // Trim any trailing whitespace/CR
    string S = line2;
    while (!S.empty() && (S.back() == '\r' || S.back() == '\n' || S.back() == ' ')) {
        S.pop_back();
    }

    // ---- Validate permutation ----
    vector<bool> seen(N+1, false);
    for (int j = 0; j < N; j++) {
        if (P[j] < 1 || P[j] > N) {
            quitf(_wa, "Placard index %d out of range [1,%d]", P[j], N);
        }
        if (seen[P[j]]) {
            quitf(_wa, "Duplicate placard %d in permutation", P[j]);
        }
        seen[P[j]] = true;
    }
    for (int i = 1; i <= N; i++) {
        if (!seen[i]) {
            quitf(_wa, "Placard %d missing from permutation", i);
        }
    }

    // ---- Validate binary string ----
    if ((int)S.size() != N) {
        quitf(_wa, "Binary string length %d, expected %d", (int)S.size(), N);
    }
    for (int j = 0; j < N; j++) {
        if (S[j] != '0' && S[j] != '1') {
            quitf(_wa, "Invalid character '%c' in binary string at position %d", S[j], j+1);
        }
    }

    // ---- Compute participant F ----
    long long F = 0;
    for (int j = 0; j < N; j++) {
        int pl = P[j];
        int ori = S[j] - '0';
        F += (ori == 0 ? A[pl] : B[pl]);
    }
    for (int j = 0; j+1 < N; j++) {
        int u = P[j], v = P[j+1];
        int ou = S[j] - '0', ov = S[j+1] - '0';
        auto it = trans.find({u,v});
        if (it != trans.end()) {
            F += it->second[ou*2 + ov];
        }
    }

    // ---- Compute score ratio ----
    // Score = round(1,000,000 * min(2.0, F / F_base))
    // We map [0, 2*F_base] -> [0, 1] for quitp
    // ratio_raw = F / F_base (can be > 2)
    // clamped   = min(2.0, ratio_raw)
    // score_ratio for quitp = clamped / 2.0  => maps [0,2] -> [0,1]
    // Special case: if F <= 0, score is 0
    double ratio_raw = (double)F / (double)F_base;
    double clamped = (ratio_raw < 0.0) ? 0.0 : (ratio_raw > 2.0 ? 2.0 : ratio_raw);
    double score_ratio = clamped / 2.0;
    // Clamp to [0,1] for safety
    if (score_ratio < 0.0) score_ratio = 0.0;
    if (score_ratio > 1.0) score_ratio = 1.0;

    long long score_per_1M = (long long)round(1000000.0 * clamped);

    quitp(score_ratio,
          "Ratio: %.9f | F=%lld F_base=%lld raw_ratio=%.6f score_per_1M=%lld",
          score_ratio,
          F, F_base, ratio_raw,
          score_per_1M);
}