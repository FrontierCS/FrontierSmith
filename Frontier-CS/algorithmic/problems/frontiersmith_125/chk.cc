#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read the input ----
    int N = inf.readInt();
    int M = inf.readInt();
    long long S = inf.readLong();

    struct Edge {
        int u, v, L, P, a;
    };
    vector<Edge> edges(M);
    map<pair<int,int>, int> edgeIdx;
    vector<int> curMask(M);

    for (int i = 0; i < M; i++) {
        edges[i].u = inf.readInt();
        edges[i].v = inf.readInt();
        edges[i].L = inf.readInt();
        edges[i].P = inf.readInt();
        edges[i].a = inf.readInt();
        curMask[i] = edges[i].a;
        int a = edges[i].u, b = edges[i].v;
        if (a > b) swap(a, b);
        edgeIdx[{a, b}] = i;
    }

    // ---- Compute baseline ----
    // BASE = sum over edges with a_i != 0 of (S + L_i)
    long long BASE = 0;
    for (int i = 0; i < M; i++) {
        if (edges[i].a != 0) {
            BASE += S + (long long)edges[i].L;
        }
    }

    // ---- Read participant output ----
    int K = ouf.readInt();
    if (K < 0 || K > 200000) {
        quitf(_wa, "K=%d out of range [0,200000]", K);
    }

    long long C_ops = 0;
    long long totalVertices = 0;

    for (int j = 0; j < K; j++) {
        int r = ouf.readInt();
        if (r < 2 || r > N) {
            quitf(_wa, "Operation %d: r=%d out of range [2,%d]", j+1, r, N);
        }
        int x = ouf.readInt();
        if (x < 1 || x > 15) {
            quitf(_wa, "Operation %d: x=%d out of range [1,15]", j+1, x);
        }

        totalVertices += r;
        if (totalVertices > 1000000LL) {
            quitf(_wa, "Total printed vertices exceeds 1000000");
        }

        vector<int> path(r);
        set<int> seen;
        for (int t = 0; t < r; t++) {
            path[t] = ouf.readInt();
            if (path[t] < 0 || path[t] >= N) {
                quitf(_wa, "Operation %d: vertex %d out of range [0,%d]", j+1, path[t], N-1);
            }
            if (seen.count(path[t])) {
                quitf(_wa, "Operation %d: duplicate vertex %d in path (not a simple path)", j+1, path[t]);
            }
            seen.insert(path[t]);
        }

        // Validate edges and apply XOR
        long long pathLen = 0;
        for (int t = 0; t + 1 < r; t++) {
            int a = path[t], b = path[t+1];
            if (a > b) swap(a, b);
            auto it = edgeIdx.find({a, b});
            if (it == edgeIdx.end()) {
                quitf(_wa, "Operation %d: no edge between %d and %d", j+1, path[t], path[t+1]);
            }
            int eid = it->second;
            pathLen += edges[eid].L;
            curMask[eid] ^= x;
        }

        C_ops += S + pathLen;
    }

    // ---- Strict end-of-file check ----
    // ouf.readEof() skips whitespace and checks no more tokens remain.
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output after all operations");
    }

    // ---- Compute remaining-fault penalty ----
    long long C_rem = 0;
    for (int i = 0; i < M; i++) {
        if (curMask[i] != 0) {
            C_rem += (long long)edges[i].P * (long long)curMask[i];
        }
    }

    long long OBJ = C_ops + C_rem;

    // ---- Score calculation ----
    // Per statement:
    //   if BASE == 0:
    //     OBJ == 0  => score = 1,000,000   => ratio = 1.0
    //     OBJ  > 0  => score = 0            => ratio = 0.0
    //   else:
    //     score = floor(10^6 * min(5, BASE/OBJ))
    //     max score = 5,000,000 (when OBJ <= BASE/5)
    //     baseline score = 1,000,000 (when OBJ == BASE)
    //
    // Mapping to quitp ratio in [0, 1]:
    //   ratio = min(1.0, (5 * 10^6) * (BASE/OBJ) / (5 * 10^6))
    //         = min(1.0, BASE / OBJ)
    //
    // This makes:
    //   baseline (OBJ == BASE)  => ratio = 1.0  => full score from platform
    //   better than baseline    => ratio = 1.0  (clamped; the 5x cap is in platform config)
    //   worse than baseline     => ratio = BASE/OBJ in (0, 1)
    //
    // Note: the platform awards floor(10^6 * ratio) per test up to 10^6,
    // giving baseline = 1,000,000 as stated.

    if (BASE == 0) {
        if (OBJ == 0) {
            quitp(1.0, "BASE=0 OBJ=0 per_test_score=1000000 Ratio: 1.000000");
        } else {
            quitp(0.0, "BASE=0 OBJ=%lld>0 per_test_score=0 Ratio: 0.000000", OBJ);
        }
    } else {
        double ratio;
        if (OBJ <= 0) {
            // Shouldn't happen since all costs are positive; treat as perfect
            ratio = 1.0;
        } else {
            // ratio = min(5, BASE/OBJ) / 5 ... but mapped so baseline=1.0:
            // score = floor(10^6 * min(5, BASE/OBJ))
            // To make baseline=1.0 in [0,1]: ratio = min(5, BASE/OBJ) / 5
            // BUT this makes baseline = 0.2, not 1.0.
            //
            // Fix: ratio = min(1.0, BASE/OBJ) so that:
            //   baseline (OBJ=BASE):  ratio = 1.0
            //   5x bonus (OBJ=BASE/5): ratio = 1.0 (clamped)
            //   worse:                ratio = BASE/OBJ < 1.0
            double raw = (double)BASE / (double)OBJ;
            if (raw > 1.0) raw = 1.0;
            if (raw < 0.0) raw = 0.0;
            ratio = raw;
        }

        long long perTestScore = (long long)floor(1000000.0 * ratio);

        quitp(ratio,
              "OBJ=%lld BASE=%lld C_ops=%lld C_rem=%lld per_test_score=%lld Ratio: %f",
              OBJ, BASE, C_ops, C_rem, perTestScore, ratio);
    }

    return 0;
}