#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read problem input ---
    int n = inf.readInt();
    int k = inf.readInt();
    int m = inf.readInt();

    vector<int> fixed_prefix(k);
    for (int i = 0; i < k; i++) {
        fixed_prefix[i] = inf.readInt();
    }

    vector<long long> w(m);
    vector<vector<int>> groups(m);
    for (int j = 0; j < m; j++) {
        w[j] = inf.readLong();
        int t = inf.readInt();
        groups[j].resize(t);
        for (int i = 0; i < t; i++) {
            groups[j][i] = inf.readInt();
        }
    }

    // --- Read participant output ---
    vector<int> perm(n);
    for (int i = 0; i < n; i++) {
        perm[i] = ouf.readInt(1, n, "permutation element out of range");
    }
    // Assert no trailing tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "Trailing tokens in output after reading %d elements", n);
    }

    // --- Validate: check for duplicates ---
    vector<int> cnt(n + 1, 0);
    for (int i = 0; i < n; i++) {
        cnt[perm[i]]++;
        if (cnt[perm[i]] > 1) {
            quitf(_wa, "Duplicate value %d in permutation at position %d", perm[i], i + 1);
        }
    }

    // --- Validate: check fixed prefix ---
    for (int i = 0; i < k; i++) {
        if (perm[i] != fixed_prefix[i]) {
            quitf(_wa, "Position %d: expected fixed value %d, got %d",
                  i + 1, fixed_prefix[i], perm[i]);
        }
    }

    // --- Build position map for participant permutation ---
    vector<int> pos(n + 1, 0);
    for (int i = 0; i < n; i++) {
        pos[perm[i]] = i + 1; // 1-indexed
    }

    // --- Compute participant cost C ---
    long double C = 0.0L;
    for (int j = 0; j < m; j++) {
        int mn = n + 1, mx = 0;
        for (int x : groups[j]) {
            if (pos[x] < mn) mn = pos[x];
            if (pos[x] > mx) mx = pos[x];
        }
        long long dj = (long long)(mx - mn + 1) - (long long)groups[j].size();
        C += (long double)w[j] * (long double)dj;
    }

    // --- Build baseline permutation: fixed prefix + remaining in increasing order ---
    vector<bool> used(n + 1, false);
    for (int i = 0; i < k; i++) {
        used[fixed_prefix[i]] = true;
    }

    vector<int> baseline(n);
    for (int i = 0; i < k; i++) {
        baseline[i] = fixed_prefix[i];
    }
    int idx = k;
    for (int x = 1; x <= n; x++) {
        if (!used[x]) {
            baseline[idx++] = x;
        }
    }

    // --- Build position map for baseline ---
    vector<int> bpos(n + 1, 0);
    for (int i = 0; i < n; i++) {
        bpos[baseline[i]] = i + 1;
    }

    // --- Compute baseline cost B ---
    long double B = 0.0L;
    for (int j = 0; j < m; j++) {
        int mn = n + 1, mx = 0;
        for (int x : groups[j]) {
            if (bpos[x] < mn) mn = bpos[x];
            if (bpos[x] > mx) mx = bpos[x];
        }
        long long dj = (long long)(mx - mn + 1) - (long long)groups[j].size();
        B += (long double)w[j] * (long double)dj;
    }

    // --- Compute score ratio ---
    long double ratio;
    if (B <= 0.0L) {
        // Baseline already zero; any feasible solution scores full marks
        ratio = 1.0L;
    } else {
        ratio = (B - C) / B;
        if (ratio < 0.0L) ratio = 0.0L;
        if (ratio > 1.0L) ratio = 1.0L;
    }

    // Required tag "Ratio: <score_ratio>" must appear in the message
    quitp((double)ratio,
          "Ratio: %.9Lf | C=%.0Lf B=%.0Lf",
          ratio, C, B);

    return 0;
}