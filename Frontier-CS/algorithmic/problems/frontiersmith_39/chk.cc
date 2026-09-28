#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Compute total penalty given:
//   perm[i] = member at position i (0-indexed), positions 0..n-1
//   sizeA = ceil(n/2), sizeB = floor(n/2)
//   pairs: map from {u,v} -> weight (1..5)
long long computePenalty(
    const vector<int>& perm,
    int n,
    int sizeA,
    const map<pair<int,int>, int>& pairs)
{
    // Build row and wing for each member (1-indexed member)
    vector<int> row(n + 1), wing(n + 1);
    for (int i = 0; i < n; i++) {
        int m = perm[i];
        if (i < sizeA) {
            wing[m] = 0; // Wing A
            row[m] = i + 1; // 1-indexed row
        } else {
            wing[m] = 1; // Wing B
            row[m] = i - sizeA + 1; // 1-indexed row
        }
    }

    long long total = 0;
    for (auto& kv : pairs) {
        int u = kv.first.first;
        int v = kv.first.second;
        int w = kv.second;
        int rowDiff = abs(row[u] - row[v]);
        long long contrib;
        if (wing[u] == wing[v]) {
            // same wing
            contrib = (long long)w * (20 + max(0, 15 - rowDiff));
        } else {
            // different wings
            contrib = (long long)w * max(0, 14 - rowDiff);
        }
        total += contrib;
    }
    return total;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read input ---
    int n = inf.readInt(1, 200000, "n");

    map<pair<int,int>, int> pairs;
    for (int day = 0; day < 5; day++) {
        int p = inf.readInt(0, 200000, "p");
        for (int j = 0; j < p; j++) {
            int u = inf.readInt(1, n, "u");
            int v = inf.readInt(1, n, "v");
            if (u > v) swap(u, v);
            pairs[{u, v}]++;
        }
    }

    int sizeA = (n + 1) / 2; // ceil(n/2)
    // int sizeB = n / 2;      // floor(n/2)

    // --- Baseline penalty: permutation 1 2 3 ... n ---
    vector<int> basePerm(n);
    for (int i = 0; i < n; i++) basePerm[i] = i + 1;
    long long P_base = computePenalty(basePerm, n, sizeA, pairs);

    // --- Read participant output ---
    vector<int> perm(n);
    vector<bool> seen(n + 1, false);

    for (int i = 0; i < n; i++) {
        int x = ouf.readInt(1, n, "member");
        if (seen[x]) {
            quitf(_wa, "Member %d appears more than once in the permutation", x);
        }
        seen[x] = true;
        perm[i] = x;
    }

    // Check all members present
    for (int m = 1; m <= n; m++) {
        if (!seen[m]) {
            quitf(_wa, "Member %d is missing from the permutation", m);
        }
    }

    // Check end of stream
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra data after the permutation");
    }

    // --- Compute participant penalty ---
    long long P = computePenalty(perm, n, sizeA, pairs);

    // --- Compute score ---
    // score = floor(500000 * (2 * P_base + 1) / (P + P_base + 1))
    // ratio = score / 1000000.0
    // Use exact integer arithmetic for numerator / denominator then convert
    long long numerator   = 500000LL * (2LL * P_base + 1LL);
    long long denominator = P + P_base + 1LL;
    long long score       = numerator / denominator; // floor division

    // Clamp score to [0, 1000000]
    if (score < 0)       score = 0;
    if (score > 1000000) score = 1000000;

    double ratio = (double)score / 1000000.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
        "P=%lld P_base=%lld score=%lld Ratio: %.6f",
        (long long)P, (long long)P_base, (long long)score, ratio);

    return 0;
}