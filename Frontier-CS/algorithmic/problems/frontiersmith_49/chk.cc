#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read problem input ──────────────────────────────────────────────────
    int n = inf.readInt();
    int p = inf.readInt();
    int m = inf.readInt();

    vector<long long> A(n+1), B(n+1), S(n+1);
    for (int i = 1; i <= n; i++) {
        A[i] = inf.readLong();
        B[i] = inf.readLong();
        S[i] = inf.readLong();
    }

    vector<long long> F(p+1), H(p+1), G(p+1), U(p+1);
    for (int j = 1; j <= p; j++) {
        F[j] = inf.readLong();
        H[j] = inf.readLong();
        G[j] = inf.readLong();
        U[j] = inf.readLong();
    }

    vector<long long> D(m+1), P(m+1);
    long long C0 = 0;
    for (int k = 1; k <= m; k++) {
        D[k] = inf.readLong();
        P[k] = inf.readLong();
        C0 += P[k] * D[k];
    }

    // Grove-to-hub edges
    int e1 = inf.readInt();
    map<pair<int,int>, long long> cost1;
    set<pair<int,int>> eset1;
    for (int e = 0; e < e1; e++) {
        int i = inf.readInt();
        int j = inf.readInt();
        long long c = inf.readLong();
        cost1[{i,j}] = c;
        eset1.insert({i,j});
    }

    // Hub-to-vault edges
    int e2 = inf.readInt();
    map<pair<int,int>, long long> cost2;
    set<pair<int,int>> eset2;
    for (int e = 0; e < e2; e++) {
        int j = inf.readInt();
        int k = inf.readInt();
        long long t = inf.readLong();
        cost2[{j,k}] = t;
        eset2.insert({j,k});
    }

    // ── Read participant output ─────────────────────────────────────────────
    int q1 = ouf.readInt();
    int q2 = ouf.readInt();

    if (q1 < 0) quitf(_wa, "q1=%d is negative", q1);
    if (q2 < 0) quitf(_wa, "q2=%d is negative", q2);

    map<pair<int,int>, long long> f; // grove->hub flow
    map<pair<int,int>, long long> g; // hub->vault flow

    set<pair<int,int>> seen1, seen2;

    for (int e = 0; e < q1; e++) {
        int i = ouf.readInt();
        int j = ouf.readInt();
        long long x = ouf.readLong();

        if (x <= 0)
            quitf(_wa, "grove->hub shipment (%d,%d) has non-positive amount %lld", i, j, x);
        if (i < 1 || i > n)
            quitf(_wa, "grove index %d out of range [1,%d]", i, n);
        if (j < 1 || j > p)
            quitf(_wa, "hub index %d out of range [1,%d]", j, p);
        if (seen1.count({i,j}))
            quitf(_wa, "duplicate grove->hub pair (%d,%d)", i, j);
        seen1.insert({i,j});
        if (!eset1.count({i,j}))
            quitf(_wa, "grove->hub edge (%d,%d) does not exist in input", i, j);

        f[{i,j}] = x;
    }

    for (int e = 0; e < q2; e++) {
        int j = ouf.readInt();
        int k = ouf.readInt();
        long long y = ouf.readLong();

        if (y <= 0)
            quitf(_wa, "hub->vault shipment (%d,%d) has non-positive amount %lld", j, k, y);
        if (j < 1 || j > p)
            quitf(_wa, "hub index %d out of range [1,%d]", j, p);
        if (k < 1 || k > m)
            quitf(_wa, "vault index %d out of range [1,%d]", k, m);
        if (seen2.count({j,k}))
            quitf(_wa, "duplicate hub->vault pair (%d,%d)", j, k);
        seen2.insert({j,k});
        if (!eset2.count({j,k}))
            quitf(_wa, "hub->vault edge (%d,%d) does not exist in input", j, k);

        g[{j,k}] = y;
    }

    // Assert no trailing tokens
    if (!ouf.seekEof())
        quitf(_wa, "extra data found after the expected output");

    // ── Feasibility checks ──────────────────────────────────────────────────

    // Grove capacities
    vector<long long> x_grove(n+1, 0);
    for (auto& kv : f) x_grove[kv.first.first] += kv.second;
    for (int i = 1; i <= n; i++) {
        if (x_grove[i] > S[i])
            quitf(_wa, "grove %d ships %lld > capacity %lld", i, x_grove[i], S[i]);
    }

    // Hub flow balance and capacity
    vector<long long> in_hub(p+1, 0), out_hub(p+1, 0);
    for (auto& kv : f) in_hub[kv.first.second] += kv.second;
    for (auto& kv : g) out_hub[kv.first.first] += kv.second;
    for (int j = 1; j <= p; j++) {
        if (in_hub[j] != out_hub[j])
            quitf(_wa, "hub %d: incoming=%lld != outgoing=%lld", j, in_hub[j], out_hub[j]);
        if (in_hub[j] > U[j])
            quitf(_wa, "hub %d throughput %lld > capacity %lld", j, in_hub[j], U[j]);
    }

    // Vault receives at most demand
    vector<long long> z_vault(m+1, 0);
    for (auto& kv : g) z_vault[kv.first.second] += kv.second;
    for (int k = 1; k <= m; k++) {
        if (z_vault[k] > D[k])
            quitf(_wa, "vault %d receives %lld > demand %lld", k, z_vault[k], D[k]);
    }

    // ── Compute total cost ──────────────────────────────────────────────────
    long long C = 0;

    // Grove harvesting costs
    for (int i = 1; i <= n; i++) {
        long long xi = x_grove[i];
        C += A[i] * xi * xi + B[i] * xi;
    }

    // Hub costs (fixed + handling)
    for (int j = 1; j <= p; j++) {
        long long yj = in_hub[j];
        if (yj > 0) {
            C += F[j] + H[j] * yj * yj + G[j] * yj;
        }
    }

    // Grove-to-hub transport costs
    for (auto& kv : f) {
        C += cost1[kv.first] * kv.second;
    }

    // Hub-to-vault transport costs
    for (auto& kv : g) {
        C += cost2[kv.first] * kv.second;
    }

    // Unmet demand penalties
    for (int k = 1; k <= m; k++) {
        long long unmet = D[k] - z_vault[k];
        C += P[k] * unmet;
    }

    // ── Compute score ratio ─────────────────────────────────────────────────
    // Per-test score = 1,000,000 * min(2, C0 / max(1, C))
    // ratio in [0,1] = that score / 2,000,000
    double ratio;
    if (C0 == 0LL) {
        // Every feasible output gets 1,000,000 = 0.5 * 2,000,000
        ratio = 0.5;
    } else {
        long long denom = max(1LL, C);
        double raw = (double)C0 / (double)denom;
        if (raw > 2.0) raw = 2.0;
        ratio = raw / 2.0;
    }

    quitp(ratio, "Ratio: %.9f, Cost: %lld, Baseline: %lld", ratio, C, C0);
}