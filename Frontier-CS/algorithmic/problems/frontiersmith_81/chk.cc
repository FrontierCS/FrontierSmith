#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read input ----
    int n = inf.readInt();
    int q = inf.readInt();
    int m = inf.readInt();

    vector<int> parent(n + 1, 0);
    for (int i = 2; i <= n; i++) {
        parent[i] = inf.readInt();
    }

    vector<long long> cap(n + 1), cst(n + 1);
    for (int i = 1; i <= n; i++) cap[i] = inf.readLong();
    for (int i = 1; i <= n; i++) cst[i] = inf.readLong();

    struct Contract {
        int v;
        long long L, R, w;
    };
    vector<Contract> contracts(q);
    for (int j = 0; j < q; j++) {
        contracts[j].v = inf.readInt();
        contracts[j].L = inf.readLong();
        contracts[j].R = inf.readLong();
        contracts[j].w = inf.readLong();
    }

    struct IPair {
        int x, y;
        long long p;
    };
    vector<IPair> pairs(m);
    for (int i = 0; i < m; i++) {
        pairs[i].x = inf.readInt();
        pairs[i].y = inf.readInt();
        pairs[i].p = inf.readLong();
    }

    // ---- Read participant output ----
    vector<long long> a(n + 1, 0);
    for (int i = 1; i <= n; i++) {
        a[i] = ouf.readLong();
    }
    // Reject trailing garbage
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra data after the %d allocation values", n);
    }

    // ---- Validate feasibility ----
    for (int i = 1; i <= n; i++) {
        if (a[i] < 0) {
            quitf(_wa, "a[%d] = %lld is negative", i, a[i]);
        }
        if (a[i] > cap[i]) {
            quitf(_wa, "a[%d] = %lld exceeds cap[%d] = %lld", i, a[i], i, cap[i]);
        }
    }

    // ---- Compute cumulative allocations s_v ----
    // parent[i] < i, so processing 1..n in order is a valid topological order
    vector<long long> s(n + 1, 0);
    s[1] = a[1];
    for (int i = 2; i <= n; i++) {
        s[i] = s[parent[i]] + a[i];
    }

    // ---- Compute reward ----
    long long reward = 0;
    for (int j = 0; j < q; j++) {
        int v = contracts[j].v;
        if (s[v] >= contracts[j].L && s[v] <= contracts[j].R) {
            reward += contracts[j].w;
        }
    }

    // ---- Compute local cost ----
    long long total_cost = 0;
    for (int i = 1; i <= n; i++) {
        total_cost += cst[i] * a[i];
    }

    // ---- Compute interference penalty ----
    long long penalty = 0;
    for (int i = 0; i < m; i++) {
        if (a[pairs[i].x] > 0 && a[pairs[i].y] > 0) {
            penalty += pairs[i].p;
        }
    }

    long long profit = reward - total_cost - penalty;

    // ---- Compute baseline profit (all a_v = 0) ----
    // s_v = 0 for all v; contracts satisfied iff L_j = 0
    // cost = 0, penalty = 0
    long long base_reward = 0;
    for (int j = 0; j < q; j++) {
        if (contracts[j].L == 0) {
            base_reward += contracts[j].w;
        }
    }
    long long base = base_reward; // cost=0, penalty=0

    // ---- Compute upper bound ----
    long long upper_val = 0;
    for (int j = 0; j < q; j++) {
        upper_val += contracts[j].w;
    }

    // ---- Compute score ratio in [0, 1] ----
    // Statement formula: 1,000,000 * max(0, min(1, (your - base)/(upper - base)))
    // quitp expects a ratio in [0,1]; it multiplies internally by the max score.
    double score_ratio;
    if (upper_val == base) {
        // Special case: baseline already achieves the trivial upper bound
        // score = 1,000,000 if your == base, else 0
        score_ratio = (profit == base) ? 1.0 : 0.0;
    } else {
        double raw = (double)(profit - base) / (double)(upper_val - base);
        score_ratio = max(0.0, min(1.0, raw));
    }

    // The judge parses "Ratio: <value>" from this message; value must be in [0,1].
    quitp(score_ratio,
          "profit=%lld (reward=%lld cost=%lld penalty=%lld) "
          "base=%lld upper=%lld | Ratio: %.9f",
          profit, reward, total_cost, penalty,
          base, upper_val, score_ratio);

    return 0;
}