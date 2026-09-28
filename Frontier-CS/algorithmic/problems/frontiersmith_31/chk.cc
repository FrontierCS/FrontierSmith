#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef unsigned long long ull;
typedef unsigned __int128 ulll;

// 2^127 - 1
static const ulll MAX_VAL = (((ulll)1) << 127) - 1;

static long long floorLog2ull(ull v) {
    if (v == 0) return -1;
    long long r = 0;
    while (v > 1) { v >>= 1; r++; }
    return r;
}

static long long popcount64(ull v) {
    return (long long)__builtin_popcountll(v);
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read problem input
    int M = inf.readInt();
    vector<ull> T(M + 1);
    for (int q = 1; q <= M; q++) {
        T[q] = (ull)inf.readLong();
    }

    // Compute baseline B
    // B = (M-1) + sum over q of (floor(log2(T_q)) + popcount(T_q))
    long long B = (long long)(M - 1);
    for (int q = 1; q <= M; q++) {
        long long fl = floorLog2ull(T[q]);
        long long pc = popcount64(T[q]);
        B += fl + pc;
    }
    if (B < 0) B = 0;

    // Read K
    if (ouf.seekEof()) {
        quitf(_wa, "Empty output: expected K");
    }
    long long K;
    {
        string tok = ouf.readToken();
        // parse K
        bool ok = true;
        K = 0;
        for (char c : tok) {
            if (c < '0' || c > '9') { ok = false; break; }
            K = K * 10 + (c - '0');
        }
        if (!ok) {
            quitf(_wa, "Invalid K: '%s'", tok.c_str());
        }
    }
    if (K < 0 || K > 500000) {
        quitf(_wa, "K=%lld is out of range [0, 500000]", K);
    }

    // Simulation state
    ulll A[7] = {0, 0, 0, 0, 0, 0, 0}; // A[1..6]
    vector<bool> completed(M + 1, false);
    long long S = 0; // counted instructions

    for (long long step = 0; step < K; step++) {
        if (ouf.seekEof()) {
            quitf(_wa, "Unexpected EOF at step %lld (expected %lld instructions)", step + 1, K);
        }
        string instr = ouf.readToken();

        if (instr == "INC") {
            string si = ouf.readToken();
            int i = 0;
            for (char c : si) { if (c < '0' || c > '9') { quitf(_wa, "Step %lld: invalid accumulator index '%s'", step+1, si.c_str()); } i = i*10+(c-'0'); }
            if (i < 1 || i > 6) quitf(_wa, "Step %lld: INC accumulator index %d out of range [1,6]", step+1, i);
            if (A[i] >= MAX_VAL) {
                quitf(_wa, "Step %lld: INC %d would overflow", step+1, i);
            }
            A[i]++;
            S++;
        } else if (instr == "ADD") {
            string si = ouf.readToken();
            string sj = ouf.readToken();
            int i = 0, j = 0;
            for (char c : si) { if (c < '0' || c > '9') quitf(_wa, "Step %lld: invalid index '%s'", step+1, si.c_str()); i = i*10+(c-'0'); }
            for (char c : sj) { if (c < '0' || c > '9') quitf(_wa, "Step %lld: invalid index '%s'", step+1, sj.c_str()); j = j*10+(c-'0'); }
            if (i < 1 || i > 6) quitf(_wa, "Step %lld: ADD first index %d out of range [1,6]", step+1, i);
            if (j < 1 || j > 6) quitf(_wa, "Step %lld: ADD second index %d out of range [1,6]", step+1, j);
            if (A[j] > 0 && A[i] > MAX_VAL - A[j]) {
                quitf(_wa, "Step %lld: ADD %d %d would overflow", step+1, i, j);
            }
            A[i] += A[j];
            S++;
        } else if (instr == "MOV") {
            string si = ouf.readToken();
            string sj = ouf.readToken();
            int i = 0, j = 0;
            for (char c : si) { if (c < '0' || c > '9') quitf(_wa, "Step %lld: invalid index '%s'", step+1, si.c_str()); i = i*10+(c-'0'); }
            for (char c : sj) { if (c < '0' || c > '9') quitf(_wa, "Step %lld: invalid index '%s'", step+1, sj.c_str()); j = j*10+(c-'0'); }
            if (i < 1 || i > 6) quitf(_wa, "Step %lld: MOV first index %d out of range [1,6]", step+1, i);
            if (j < 1 || j > 6) quitf(_wa, "Step %lld: MOV second index %d out of range [1,6]", step+1, j);
            A[i] = A[j];
            S++;
        } else if (instr == "CLR") {
            string si = ouf.readToken();
            int i = 0;
            for (char c : si) { if (c < '0' || c > '9') quitf(_wa, "Step %lld: invalid index '%s'", step+1, si.c_str()); i = i*10+(c-'0'); }
            if (i < 1 || i > 6) quitf(_wa, "Step %lld: CLR index %d out of range [1,6]", step+1, i);
            A[i] = 0;
            S++;
        } else if (instr == "OUT") {
            string sq = ouf.readToken();
            string si = ouf.readToken();
            int q = 0, i = 0;
            for (char c : sq) { if (c < '0' || c > '9') quitf(_wa, "Step %lld: OUT invalid order '%s'", step+1, sq.c_str()); q = q*10+(c-'0'); }
            for (char c : si) { if (c < '0' || c > '9') quitf(_wa, "Step %lld: OUT invalid index '%s'", step+1, si.c_str()); i = i*10+(c-'0'); }
            if (q < 1 || q > M) quitf(_wa, "Step %lld: OUT order %d out of range [1,%d]", step+1, q, M);
            if (i < 1 || i > 6) quitf(_wa, "Step %lld: OUT accumulator %d out of range [1,6]", step+1, i);
            if (completed[q]) {
                quitf(_wa, "Step %lld: OUT %d %d - order %d already completed", step+1, q, i, q);
            }
            if (A[i] != (ulll)T[q]) {
                quitf(_wa, "Step %lld: OUT %d %d - A[%d] does not equal T[%d]=%llu",
                      step+1, q, i, i, q, T[q]);
            }
            completed[q] = true;
            // OUT does not count toward S
        } else {
            quitf(_wa, "Step %lld: Unknown instruction '%s'", step+1, instr.c_str());
        }
    }

    // After reading K instructions, the stream should be at EOF (allowing trailing whitespace)
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra content after K=%lld instructions", K);
    }

    // Check all orders completed
    for (int q = 1; q <= M; q++) {
        if (!completed[q]) {
            quitf(_wa, "Order %d (T=%llu) was never completed", q, T[q]);
        }
    }

    // Compute score: score = round(10^9 * B / (B + S))
    // ratio = B / (B + S)
    double ratio;
    if (B == 0 && S == 0) {
        ratio = 1.0;
    } else if (B + S <= 0) {
        ratio = 1.0;
    } else {
        ratio = (double)B / (double)(B + S);
    }
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio, "Ratio: %.9f S=%lld B=%lld", ratio, S, B);
    return 0;
}