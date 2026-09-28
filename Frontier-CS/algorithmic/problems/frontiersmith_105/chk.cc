#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int t = inf.readInt();

    double total_score = 0.0;

    for (int tc = 1; tc <= t; tc++) {
        // ---- read this test-case's problem data from inf ----
        int n = inf.readInt();
        int K = inf.readInt();
        long long B = inf.readLong();

        vector<int> a(n + 1), b(n + 1), w(n + 1);
        for (int i = 1; i <= n; i++) a[i] = inf.readInt();
        for (int i = 1; i <= n; i++) b[i] = inf.readInt();
        for (int i = 1; i <= n; i++) w[i] = inf.readInt();

        // ---- canonical sorted order: sort items by (a_i, original 1-index) ----
        vector<int> items(n);
        iota(items.begin(), items.end(), 1);
        sort(items.begin(), items.end(), [&](int x, int y) {
            if (a[x] != a[y]) return a[x] < a[y];
            return x < y;
        });

        // target[i] = 1-indexed target position of item i
        vector<int> target(n + 1);
        for (int p = 0; p < n; p++) {
            target[items[p]] = p + 1;
        }

        // ---- D_init: disorder of the initial arrangement ----
        long long D_init = 0;
        for (int i = 1; i <= n; i++) {
            D_init += (long long)w[i] * (long long)abs(i - target[i]);
        }

        // ---- read participant's answer for this test case ----
        int m = ouf.readInt(0, (int)2e9, "m");

        if (m > K) {
            quitf(_wa, "Test case %d: m=%d exceeds K=%d", tc, m, K);
        }

        // item_at[p] = which item is currently at position p (1-indexed)
        // pos_of[i]  = current position of item i
        vector<int> item_at(n + 1), pos_of(n + 1);
        for (int i = 1; i <= n; i++) {
            item_at[i] = i;
            pos_of[i]  = i;
        }

        long long total_energy = 0;

        for (int s = 1; s <= m; s++) {
            int x = ouf.readInt(1, n, "x");
            int y = ouf.readInt(1, n, "y");

            if (x == y) {
                quitf(_wa, "Test case %d, swap %d: x == y == %d", tc, s, x);
            }

            int u = item_at[x];
            int v = item_at[y];

            if (b[u] == b[v]) {
                quitf(_wa,
                    "Test case %d, swap %d: items at positions %d and %d "
                    "have the same type %d",
                    tc, s, x, y, b[u]);
            }

            long long cost = (long long)abs(x - y) * ((long long)w[u] + (long long)w[v]);
            total_energy += cost;

            if (total_energy > B) {
                quitf(_wa,
                    "Test case %d, swap %d: cumulative energy %lld exceeds budget B=%lld",
                    tc, s, total_energy, B);
            }

            // apply the swap
            item_at[x] = v;
            item_at[y] = u;
            pos_of[u]  = y;
            pos_of[v]  = x;
        }

        // ---- compute D_sub after participant's swaps ----
        long long D_sub = 0;
        for (int i = 1; i <= n; i++) {
            D_sub += (long long)w[i] * (long long)abs(pos_of[i] - target[i]);
        }

        // ---- per-test continuous score ----
        double score;
        if (D_init == 0) {
            // Already sorted initially; participant must not make it worse
            score = (D_sub == 0) ? 1.0 : 0.0;
        } else {
            score = (double)(D_init - D_sub) / (double)D_init;
            if (score < 0.0) score = 0.0;
            if (score > 1.0) score = 1.0;
        }

        total_score += score;
    }

    // NOTE: do NOT call ouf.readEof() — trailing newlines from participant
    // output would cause a spurious WA before any score is reported.

    double final_ratio = total_score / (double)t;
    if (final_ratio < 0.0) final_ratio = 0.0;
    if (final_ratio > 1.0) final_ratio = 1.0;

    quitp(final_ratio, "Ratio: %.9f", final_ratio);
}