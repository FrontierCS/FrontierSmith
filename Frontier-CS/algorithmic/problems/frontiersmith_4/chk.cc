#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read the input file ──────────────────────────────────────────────────
    int M = inf.readInt();
    int K = inf.readInt();

    vector<long long> cv(M), wv(M);
    long long C_max = 0;

    for (int i = 0; i < M; i++) {
        cv[i] = inf.readLong();
        wv[i] = inf.readLong();
        if (cv[i] > C_max) C_max = cv[i];
    }

    // W(c) = total weight for each distinct target value
    map<long long, long long> W;
    for (int i = 0; i < M; i++)
        W[cv[i]] += wv[i];

    // ── Compute baseline B ───────────────────────────────────────────────────
    // K == 0  →  B = 0
    // K  > 0  →  seed 1 is always chosen, then pick K-1 largest W(c)
    //            (ties broken by smaller c)
    long long B = 0;
    if (K > 0) {
        vector<pair<long long,long long>> wc_list;
        wc_list.reserve(W.size());
        for (auto& kv : W)
            wc_list.push_back({kv.second, kv.first});

        sort(wc_list.begin(), wc_list.end(),
             [](const pair<long long,long long>& a,
                const pair<long long,long long>& b) {
                 if (a.first != b.first) return a.first > b.first;
                 return a.second < b.second;
             });

        int take = min((int)wc_list.size(), K - 1);
        for (int i = 0; i < take; i++)
            B += wc_list[i].first;
    }

    // ── Read participant output ──────────────────────────────────────────────
    // Per the statement: "Any infeasible output receives objective value 0."
    // So we never hard-fail with _wa; instead we set R=0 and score normally.

    bool infeasible = false;
    int S = 0;
    vector<long long> seeds;

    // Try to read S
    if (ouf.seekEof()) {
        // Empty output => S=0, seeds empty => feasible
        S = 0;
    } else {
        // Read S as a raw integer (no auto-range-check so we can handle it ourselves)
        S = ouf.readInt();
        if (S < 0 || S > K) {
            infeasible = true;
        }
    }

    if (!infeasible) {
        set<long long> seed_set;
        seeds.reserve(S);
        bool reading_ok = true;

        for (int i = 0; i < S; i++) {
            if (ouf.seekEof()) {
                // Fewer seeds than claimed
                infeasible = true;
                reading_ok = false;
                break;
            }
            long long p = ouf.readLong();
            if (p < 1 || p > C_max) {
                infeasible = true;
                reading_ok = false;
                break;
            }
            if (seed_set.count(p)) {
                infeasible = true;
                reading_ok = false;
                break;
            }
            seed_set.insert(p);
            seeds.push_back(p);
        }

        // Check for extra tokens after the declared seeds
        if (reading_ok && !ouf.seekEof()) {
            // Extra trailing content: infeasible
            infeasible = true;
        }
    }

    // ── Compute R ────────────────────────────────────────────────────────────
    long long R = 0;

    if (!infeasible && !seeds.empty()) {
        sort(seeds.begin(), seeds.end());

        set<long long> covered_c;
        int sz = (int)seeds.size();
        for (int i = 0; i < sz; i++) {
            long long x = seeds[i];
            if (x > C_max) continue;
            for (int j = i + 1; j < sz; j++) {
                long long y = seeds[j];
                // x < y because seeds are sorted
                if (x > C_max / y)   // x*y > C_max (avoids overflow)
                    break;
                long long prod = x * y;
                if (__gcd(x, y) == 1LL)
                    covered_c.insert(prod);
            }
        }

        for (int i = 0; i < M; i++)
            if (covered_c.count(cv[i]))
                R += wv[i];
    }
    // If infeasible, R stays 0 (per problem statement).

    // ── Score ────────────────────────────────────────────────────────────────
    // score = 100            if B == 0
    //       = 100*min(1, R/(2B))  if B > 0
    double ratio;
    if (B == 0) {
        // B=0: any feasible output (including infeasible → R=0) scores 100
        ratio = 1.0;
    } else {
        ratio = (double)R / (2.0 * (double)B);
        if (ratio > 1.0) ratio = 1.0;
        if (ratio < 0.0) ratio = 0.0;
    }

    if (infeasible) {
        quitp(ratio, "INFEASIBLE (R=0) B=%lld Ratio: %.9f", B, ratio);
    } else {
        quitp(ratio, "R=%lld B=%lld Ratio: %.9f", R, B, ratio);
    }
    return 0;
}