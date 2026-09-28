#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Returns true if contract (opt, x) is satisfied somewhere in b[l-1 .. r-1]
// i == j is allowed per problem statement
bool contractSatisfied(int opt, long long x, const vector<long long>& b, int l, int r) {
    unordered_map<long long, int> freq;
    for (int i = l - 1; i < r; i++) freq[b[i]]++;

    if (opt == 1) {
        // |b_i - b_j| = x  (i=j allowed → x=0 trivially true)
        if (x == 0) return true;
        for (auto& kv : freq) {
            long long v = kv.first;
            if (freq.count(v + x)) return true;
        }
        return false;
    } else if (opt == 2) {
        // b_i + b_j = x  (i=j allowed: need 2*v=x, one occurrence suffices)
        for (auto& kv : freq) {
            long long v = kv.first;
            long long need = x - v;
            if (need < 0) continue;
            if (need == v) {
                // same position allowed, one occurrence is enough
                return true;
            }
            if (freq.count(need)) return true;
        }
        return false;
    } else {
        // opt == 3: b_i * b_j = x  (i=j allowed)
        if (x == 0) {
            return freq.count(0LL) > 0;
        }
        for (auto& kv : freq) {
            long long v = kv.first;
            if (v == 0) continue;
            if (x % v == 0) {
                long long need = x / v;
                if (need == v) {
                    // same position allowed
                    return true;
                }
                if (freq.count(need)) return true;
            }
        }
        return false;
    }
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input ----
    int n = inf.readInt();
    int m = inf.readInt();
    long long C = inf.readLong();
    long long H = inf.readLong();

    vector<long long> a(n);
    for (int i = 0; i < n; i++) a[i] = inf.readLong();

    struct Contract {
        int opt, l, r;
        long long x, w;
    };
    vector<Contract> contracts(m);
    for (int i = 0; i < m; i++) {
        contracts[i].opt = inf.readInt();
        contracts[i].l   = inf.readInt();
        contracts[i].r   = inf.readInt();
        contracts[i].x   = inf.readLong();
        contracts[i].w   = inf.readLong();
    }

    // ---- Compute W_base (no changes) ----
    vector<long long> bvec(a.begin(), a.end());
    long long W_base = 0;
    for (int i = 0; i < m; i++) {
        if (contractSatisfied(contracts[i].opt, contracts[i].x,
                              bvec, contracts[i].l, contracts[i].r)) {
            W_base += contracts[i].w;
        }
    }

    // ---- Compute W_all ----
    long long W_all = 0;
    for (int i = 0; i < m; i++) W_all += contracts[i].w;

    // ---- Read participant output ----
    int k = ouf.readInt();
    if (k < 0 || k > n) {
        quitf(_wa, "Invalid k=%d, must be in [0, %d]", k, n);
    }

    // Reset b to a before applying participant changes
    bvec.assign(a.begin(), a.end());
    set<int> changed_indices;

    for (int q = 0; q < k; q++) {
        int idx = ouf.readInt();
        long long val = ouf.readLong();

        if (idx < 1 || idx > n) {
            quitf(_wa, "Invalid idx=%d on change %d, must be in [1, %d]", idx, q + 1, n);
        }
        if (val < 0 || val > C) {
            quitf(_wa, "Invalid val=%lld on change %d, must be in [0, %lld]", val, q + 1, C);
        }
        if (changed_indices.count(idx)) {
            quitf(_wa, "Duplicate idx=%d in recalibration list", idx);
        }
        changed_indices.insert(idx);
        bvec[idx - 1] = val;
    }

    // ---- Enforce EOF: reject any trailing tokens ----
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output after the %d recalibration lines", k);
    }

    // ---- Check energy budget ----
    long long energy = 0;
    for (int i = 0; i < n; i++) energy += llabs(a[i] - bvec[i]);
    if (energy > H) {
        quitf(_wa, "Total energy %lld exceeds budget H=%lld", energy, H);
    }

    // ---- Compute W_sub ----
    long long W_sub = 0;
    for (int i = 0; i < m; i++) {
        if (contractSatisfied(contracts[i].opt, contracts[i].x,
                              bvec, contracts[i].l, contracts[i].r)) {
            W_sub += contracts[i].w;
        }
    }

    // ---- Scoring: ratio = max(0, W_sub - W_base) / (W_all - W_base) ----
    // Problem guarantees W_all > W_base on real test data;
    // guard against degenerate test cases just in case.
    long long denom = W_all - W_base;
    double ratio = 0.0;
    if (denom > 0) {
        long long numer = max(0LL, W_sub - W_base);
        ratio = (double)numer / (double)denom;
    }
    ratio = max(0.0, min(1.0, ratio));

    quitp(ratio,
          "W_sub=%lld W_base=%lld W_all=%lld energy=%lld Ratio: %.6f",
          W_sub, W_base, W_all, energy, ratio);

    return 0;
}