#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input from inf ----
    int n = inf.readInt();
    int m = inf.readInt();
    int D = inf.readInt();
    int A = inf.readInt();
    long long B = inf.readLong();

    vector<int> eu(m+1), ev(m+1);
    vector<long long> ec(m+1), ed(m+1);
    for (int i = 1; i <= m; i++) {
        eu[i] = inf.readInt();
        ev[i] = inf.readInt();
        ec[i] = inf.readLong();
        ed[i] = inf.readLong();
    }

    vector<long long> L(D+1);
    for (int t = 1; t <= D; t++) {
        L[t] = inf.readLong();
    }

    vector<int> ah(A+1), aa(A+1), ab(A+1);
    vector<long long> aw(A+1);
    for (int j = 1; j <= A; j++) {
        ah[j] = inf.readInt();
        aa[j] = inf.readInt();
        ab[j] = inf.readInt();
        aw[j] = inf.readLong();
    }

    // ---- Compute baseline damage ----
    // Baseline: build no roads, stay at house 1 every day.
    // House 1 is always visited. Attacks at house 1 are cleared.
    long long baseDamage = 0;
    for (int j = 1; j <= A; j++) {
        if (ah[j] != 1) {
            baseDamage += aw[j];
        }
    }

    // ---- Read participant output from ouf ----

    // Line 1: built roads
    int k = ouf.readInt();
    if (k < 0 || k > m) {
        quitf(_wa, "Invalid k=%d (must be 0..%d)", k, m);
    }

    vector<bool> built(m+1, false);
    long long totalBuildCost = 0;

    for (int i = 0; i < k; i++) {
        int e = ouf.readInt();
        if (e < 1 || e > m) {
            quitf(_wa, "Road index %d out of range [1,%d]", e, m);
        }
        if (built[e]) {
            quitf(_wa, "Duplicate road index %d in built list", e);
        }
        built[e] = true;
        totalBuildCost += ec[e];
    }

    if (totalBuildCost > B) {
        quitf(_wa, "Total build cost %lld exceeds budget %lld", totalBuildCost, B);
    }

    // Lines 2..D+1: daily patrols
    // visitedSet[t] = set of houses visited on day t
    // House 1 is always visited (start/end)
    vector<set<int>> visitedSet(D+1);
    for (int t = 1; t <= D; t++) {
        visitedSet[t].insert(1);
    }

    for (int t = 1; t <= D; t++) {
        int p = ouf.readInt();
        if (p < 0) {
            quitf(_wa, "Day %d: negative number of traversals %d", t, p);
        }

        int curHouse = 1;
        long long timeUsed = 0;

        for (int s = 0; s < p; s++) {
            int f = ouf.readInt();
            if (f < 1 || f > m) {
                quitf(_wa, "Day %d traversal %d: road index %d out of range [1,%d]", t, s+1, f, m);
            }
            if (!built[f]) {
                quitf(_wa, "Day %d traversal %d: road %d was not built", t, s+1, f);
            }
            int nextHouse = -1;
            if (eu[f] == curHouse) {
                nextHouse = ev[f];
            } else if (ev[f] == curHouse) {
                nextHouse = eu[f];
            } else {
                quitf(_wa, "Day %d traversal %d: road %d (connects %d-%d) not incident to current house %d",
                      t, s+1, f, eu[f], ev[f], curHouse);
            }
            timeUsed += ed[f];
            curHouse = nextHouse;
            visitedSet[t].insert(curHouse);
        }

        if (curHouse != 1) {
            quitf(_wa, "Day %d: patrol does not return to house 1 (ended at house %d)", t, curHouse);
        }
        if (timeUsed > L[t]) {
            quitf(_wa, "Day %d: total travel time %lld exceeds limit %lld", t, timeUsed, L[t]);
        }
    }

    // Ensure no trailing data
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra data found after the last patrol line");
    }

    // ---- Compute participant's damage ----
    long long yourDamage = 0;
    for (int j = 1; j <= A; j++) {
        bool cleared = false;
        for (int t = aa[j]; t <= ab[j] && !cleared; t++) {
            if (visitedSet[t].count(ah[j])) {
                cleared = true;
            }
        }
        if (!cleared) {
            yourDamage += aw[j];
        }
    }

    // ---- Compute score ----
    // Statement formula:
    //   Score = floor(1,000,000 * min(10, (BaseDamage+1) / (YourDamage+1)))
    //
    // For quitp, we need a ratio in [0,1].
    // ratio_raw = min(10.0, (baseDamage+1) / (yourDamage+1))  [range: 0..10]
    // ratio     = ratio_raw / 10.0                             [range: 0..1]
    //
    // The per-file integer score (for display) is:
    //   floor(1,000,000 * ratio_raw) = floor(10,000,000 * ratio)
    //
    // The "Ratio:" tag in the message must equal the quitp argument (ratio in [0,1]).

    double ratio_raw = (double)(baseDamage + 1) / (double)(yourDamage + 1);
    if (ratio_raw > 10.0) ratio_raw = 10.0;
    if (ratio_raw < 0.0) ratio_raw = 0.0;

    double ratio = ratio_raw / 10.0;  // in [0, 1]

    // Integer score as stated: floor(1,000,000 * min(10, ...))
    long long integerScore = (long long)floor(1000000.0 * ratio_raw);

    // quitp passes ratio (in [0,1]) to the judge.
    // The message contains "Ratio: <ratio>" which the judge reads.
    quitp(ratio,
          "Ratio: %.9f | BaseDamage: %lld | YourDamage: %lld | Score: %lld / 10000000",
          ratio, baseDamage, yourDamage, integerScore);

    return 0;
}