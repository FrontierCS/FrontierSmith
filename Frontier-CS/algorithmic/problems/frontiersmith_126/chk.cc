#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// __int128 helpers
typedef __int128 lll;

static lll toLLL(long long x) { return (lll)x; }

// Print __int128 as decimal string
static string i128str(lll v) {
    if (v == 0) return "0";
    bool neg = v < 0;
    if (neg) v = -v;
    string s;
    while (v > 0) { s += char('0' + (int)(v % 10)); v /= 10; }
    if (neg) s += '-';
    reverse(s.begin(), s.end());
    return s;
}

// Compute number of equal pairs (i,j) with i in [L1,R1], j in [L2,R2]
// using arr (1-indexed). cnt must be pre-zeroed, used is scratch.
static long long computeEq(
    const vector<int>& arr,
    int L1, int R1, int L2, int R2,
    vector<int>& cnt, vector<int>& used)
{
    used.clear();
    for (int i = L1; i <= R1; i++) {
        int v = arr[i];
        if (cnt[v] == 0) used.push_back(v);
        cnt[v]++;
    }

    long long eq = 0;
    for (int j = L2; j <= R2; j++) {
        eq += cnt[arr[j]];
    }

    for (int v : used) cnt[v] = 0;
    return eq;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read input ---
    int N = inf.readInt();

    vector<int> A(N + 1);
    vector<long long> C(N + 1);
    for (int i = 1; i <= N; i++) A[i] = inf.readInt();
    for (int i = 1; i <= N; i++) C[i] = inf.readLong();

    long long BUDGET = inf.readLong();

    int Q = inf.readInt();

    struct Request {
        int T, L1, R1, L2, R2;
        long long W;
    };
    vector<Request> reqs(Q);
    for (int q = 0; q < Q; q++) {
        reqs[q].T  = inf.readInt();
        reqs[q].L1 = inf.readInt();
        reqs[q].R1 = inf.readInt();
        reqs[q].L2 = inf.readInt();
        reqs[q].R2 = inf.readInt();
        reqs[q].W  = inf.readLong();
    }

    // --- Read participant output (N integers) ---
    vector<int> B(N + 1);
    for (int i = 1; i <= N; i++) {
        if (ouf.seekEof()) {
            quitf(_wa, "Output has fewer than %d integers (stopped at position %d)", N, i);
        }
        B[i] = ouf.readInt();
        if (B[i] < 1 || B[i] > N) {
            quitf(_wa, "B[%d] = %d is out of range [1, %d]", i, B[i], N);
        }
    }

    // Strict end-of-file check: reject any extra tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after the %d expected integers", N);
    }

    // --- Check budget ---
    long long totalCost = 0;
    for (int i = 1; i <= N; i++) {
        if (B[i] != A[i]) {
            totalCost += C[i];
        }
    }
    if (totalCost > BUDGET) {
        quitf(_wa, "Total edit cost %lld exceeds budget %lld", totalCost, BUDGET);
    }

    // --- Compute UPPER, BASE, OBJ(B) using __int128 to avoid overflow ---
    vector<int> cnt(N + 2, 0);
    vector<int> used;
    used.reserve(N);

    lll UPPER = 0;
    lll BASE  = 0;
    lll OBJB  = 0;

    for (int q = 0; q < Q; q++) {
        int T       = reqs[q].T;
        int L1      = reqs[q].L1;
        int R1      = reqs[q].R1;
        int L2      = reqs[q].L2;
        int R2      = reqs[q].R2;
        long long W = reqs[q].W;

        lll S = toLLL(R1 - L1 + 1) * toLLL(R2 - L2 + 1);
        UPPER += toLLL(W) * S;

        long long eqA = computeEq(A, L1, R1, L2, R2, cnt, used);
        long long eqB = computeEq(B, L1, R1, L2, R2, cnt, used);

        if (T == 1) {
            BASE += toLLL(W) * toLLL(eqA);
            OBJB += toLLL(W) * toLLL(eqB);
        } else {
            BASE += toLLL(W) * (S - toLLL(eqA));
            OBJB += toLLL(W) * (S - toLLL(eqB));
        }
    }

    // --- Compute score ratio per problem statement ---
    double ratio;
    if (UPPER == BASE) {
        // Already optimal by baseline; any feasible output scores full
        ratio = 1.0;
    } else {
        long double improvement =
            (long double)(OBJB - BASE) / (long double)(UPPER - BASE);
        if (improvement < 0.0L) improvement = 0.0L;
        if (improvement > 1.0L) improvement = 1.0L;
        ratio = (double)improvement;
    }

    long long scoreInt = (long long)(ratio * 1000000.0 + 0.5);

    quitp(ratio,
        "Ratio: %.9f | OBJ=%s BASE=%s UPPER=%s score=%lld/1000000",
        ratio,
        i128str(OBJB).c_str(),
        i128str(BASE).c_str(),
        i128str(UPPER).c_str(),
        scoreInt);

    return 0;
}