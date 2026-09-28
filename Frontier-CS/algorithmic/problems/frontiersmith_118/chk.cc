#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---------- read input ----------
    int n = inf.readInt();
    int q = inf.readInt();
    int k = inf.readInt();

    vector<int> p(n + 1);
    for (int i = 1; i <= n; i++) p[i] = inf.readInt();

    vector<int> a(n + 1);
    for (int i = 1; i <= n; i++) a[i] = inf.readInt();

    struct Query { int l, r, w, d; };
    vector<Query> queries(q);
    long long U = 0;
    for (int t = 0; t < q; t++) {
        queries[t].l = inf.readInt();
        queries[t].r = inf.readInt();
        queries[t].w = inf.readInt();
        queries[t].d = inf.readInt();
        U += (long long)queries[t].w * queries[t].d;
    }

    // ---------- read participant output ----------
    // Read WITHOUT strict range check; range violations -> feasible=false -> Obj=0
    vector<int> b(n + 1);
    bool feasible = true;

    for (int i = 1; i <= n; i++) {
        if (ouf.seekEof()) {
            quitf(_wa, "Output has fewer than %d integers (missing b[%d])", n, i);
        }
        b[i] = ouf.readInt();
        if (b[i] < 1 || b[i] > n) {
            feasible = false;
            // continue reading so we can consume all output
        }
    }
    // Check for trailing garbage
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra characters found after the %d output values", n);
    }

    // ---------- feasibility: Hamming distance ----------
    // Only meaningful when all values are in range; otherwise already infeasible
    if (feasible) {
        int hamming = 0;
        for (int i = 1; i <= n; i++) {
            if (a[i] != b[i]) hamming++;
        }
        if (hamming > k) {
            feasible = false;
        }
    }

    // ---------- compute objective ----------
    // Uses timestamp trick to avoid O(n) clear per query.
    // present[v] == ts   => value v appears in current window
    // counted[v] == ts   => value v has already been processed in current window
    auto computeObj = [&](const vector<int>& seq) -> long long {
        vector<int> present(n + 1, 0);
        vector<int> counted(n + 1, 0);
        long long obj = 0;
        int ts = 0;
        for (int t = 0; t < q; t++) {
            int l = queries[t].l, r = queries[t].r;
            int w = queries[t].w, d = queries[t].d;
            ts++;

            // Mark presence
            for (int i = l; i <= r; i++) {
                present[seq[i]] = ts;
            }

            // Count distinct supported labels
            int c = 0;
            for (int i = l; i <= r; i++) {
                int x = seq[i];
                if (counted[x] == ts) continue; // already processed this value
                counted[x] = ts;
                // x is supported iff p[x] is also present in [l,r]
                if (present[p[x]] == ts) {
                    c++;
                }
            }

            obj += (long long)w * min(c, d);
        }
        return obj;
    };

    // ---------- baseline B (sequence a is always valid) ----------
    long long B = computeObj(a);

    // ---------- participant objective: 0 if infeasible ----------
    long long Obj = feasible ? computeObj(b) : 0LL;

    // ---------- scoring ----------
    double ratio;
    if (U == B) {
        // Baseline already achieves the upper bound; any feasible solution scores 100
        ratio = 1.0;
    } else {
        double num = (double)(Obj - B);
        double den = (double)(U   - B);
        ratio = num / den;
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
    }

    quitp(ratio,
          "Obj=%lld  Baseline=%lld  U=%lld  Ratio: %.6f",
          Obj, B, U, ratio);
}