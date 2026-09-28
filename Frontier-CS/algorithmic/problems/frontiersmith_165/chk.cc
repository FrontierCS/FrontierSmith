#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read problem input ──────────────────────────────────────────────────
    int n = inf.readInt();
    int m = inf.readInt();
    long long s = inf.readLong();

    // A[1..n]: initial arrangement (1-indexed)
    vector<int> A(n + 1);
    for (int i = 1; i <= n; i++) A[i] = inf.readInt();

    // Exhibit data (1-indexed by exhibit number)
    vector<long long> Px(n + 1), Bx(n + 1), Dx(n + 1);
    for (int x = 1; x <= n; x++) {
        Px[x] = inf.readLong();
        Bx[x] = inf.readLong();
        Dx[x] = inf.readLong();
    }

    // Synergy relations
    map<pair<int,int>, long long> synergy;
    long long totalSynergyW = 0;
    for (int i = 0; i < m; i++) {
        int u = inf.readInt();
        int v = inf.readInt();
        long long w = inf.readLong();
        if (u > v) swap(u, v);
        synergy[{u, v}] = w;
        totalSynergyW += w;
    }

    // ── Helper: compute value of an arrangement ─────────────────────────────
    auto computeValue = [&](const vector<int>& arr) -> long long {
        long long val = 0;
        for (int j = 1; j <= n; j++) {
            int x = arr[j];
            long long dist = (long long)abs((long long)j - Px[x]);
            long long rv = Bx[x] - Dx[x] * dist;
            if (rv > 0) val += rv;
        }
        for (int j = 1; j < n; j++) {
            int a = arr[j], b = arr[j + 1];
            if (a > b) swap(a, b);
            auto it = synergy.find({a, b});
            if (it != synergy.end()) val += it->second;
        }
        return val;
    };

    // ── Baseline: no operations ─────────────────────────────────────────────
    long long baselineV = computeValue(A);

    // ── Upper bound U ───────────────────────────────────────────────────────
    long long U = totalSynergyW;
    for (int x = 1; x <= n; x++) U += Bx[x];

    // ── Read participant output ─────────────────────────────────────────────
    int q;
    if (ouf.seekEof()) {
        // Empty output: treat as q=0
        q = 0;
    } else {
        q = ouf.readInt(0, 2000000, "q");
    }

    long long totalCost = 0;
    vector<int> F = A; // working arrangement (1-indexed)

    for (int op = 1; op <= q; op++) {
        // Read k
        if (ouf.seekEof()) {
            quitf(_wa, "Operation %d: expected k but found EOF", op);
        }
        int k = ouf.readInt(2, n, "k");

        vector<int> rooms(k);
        set<int> seen;
        for (int i = 0; i < k; i++) {
            if (ouf.seekEof()) {
                quitf(_wa, "Operation %d: expected room index %d but found EOF", op, i + 1);
            }
            rooms[i] = ouf.readInt(1, n, "room_index");
            if (seen.count(rooms[i])) {
                quitf(_wa, "Operation %d: duplicate room index %d", op, rooms[i]);
            }
            seen.insert(rooms[i]);
        }

        totalCost += (long long)k;
        if (totalCost > s) {
            quitf(_wa,
                  "Total cost %lld after operation %d exceeds s=%lld",
                  totalCost, op, s);
        }

        // Apply rotation:
        // exhibit in r_1 -> r_2, r_2 -> r_3, ..., r_{k-1} -> r_k, r_k -> r_1
        // saved = old F[r_k]; then shift F[r_k]=F[r_{k-1}], ..., F[r_1]=saved
        int saved = F[rooms[k - 1]];
        for (int i = k - 1; i > 0; i--) {
            F[rooms[i]] = F[rooms[i - 1]];
        }
        F[rooms[0]] = saved;
    }

    // ── Strict EOF check ────────────────────────────────────────────────────
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after all operations were read");
    }

    // ── Compute final value ─────────────────────────────────────────────────
    long long V = computeValue(F);

    // ── Scoring ─────────────────────────────────────────────────────────────
    // Statement: score = 1,000,000 * clamp((V-B)/(U-B), 0, 1)
    // Special case: 1,000,000 if U == B
    // Ratio tag must hold the [0,1] clamp value.
    double ratio;
    if (U == baselineV) {
        // Nothing to improve; any feasible output gets full score
        ratio = 1.0;
    } else {
        ratio = (double)(V - baselineV) / (double)(U - baselineV);
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
    }

    double score = 1000000.0 * ratio;

    quitp(score,
          "V=%lld Baseline=%lld U=%lld Ratio: %.9f",
          V, baselineV, U, ratio);

    return 0;
}