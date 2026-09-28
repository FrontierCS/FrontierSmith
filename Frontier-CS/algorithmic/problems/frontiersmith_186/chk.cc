#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef array<int,3> A3;

A3 applyOri(int ori, int x, int y, int z) {
    switch (ori) {
        case 0: return {x, y, z};
        case 1: return {y, z, x};
        case 2: return {z, x, y};
        case 3: return {x, z, y};
        case 4: return {z, y, x};
        case 5: return {y, x, z};
    }
    return {x, y, z};
}

int gcdf(int a, int b) {
    while (b) { a %= b; swap(a, b); }
    return a;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    // Per-case scores in [0, 1000000]
    // Final = floor(sum / T), then ratio = final / 1000000.0
    double totalCaseScore = 0.0; // sum of per-case scores (each in [0,1000000])
    int caseCount = 0;

    for (int tc = 0; tc < T; tc++) {
        int m = inf.readInt();
        int n = inf.readInt();

        vector<int> X(m+1), Y(m+1), Z(m+1), P(m+1);
        for (int i = 1; i <= m; i++) {
            X[i] = inf.readInt();
            Y[i] = inf.readInt();
            Z[i] = inf.readInt();
            P[i] = inf.readInt();
        }

        // ---------- Read participant output ----------
        bool valid = true;
        string invalidReason;
        vector<int> ids(n, 0), oris(n, 0);

        for (int j = 0; j < n; j++) {
            if (ouf.seekEof()) {
                valid = false;
                invalidReason = format("Case %d, position %d: unexpected end of output", tc+1, j+1);
                break;
            }
            ids[j]  = ouf.readInt();
            oris[j] = ouf.readInt();
        }

        if (!valid) {
            totalCaseScore += 0.0;
            caseCount++;
            continue;
        }

        // ---------- Validate ----------
        {
            set<int> used;
            for (int j = 0; j < n && valid; j++) {
                if (ids[j] < 1 || ids[j] > m) {
                    valid = false;
                    invalidReason = format("Case %d pos %d: bead id %d out of [1,%d]", tc+1, j+1, ids[j], m);
                } else if (oris[j] < 0 || oris[j] > 5) {
                    valid = false;
                    invalidReason = format("Case %d pos %d: orientation %d not in [0,5]", tc+1, j+1, oris[j]);
                } else if (used.count(ids[j])) {
                    valid = false;
                    invalidReason = format("Case %d pos %d: duplicate bead id %d", tc+1, j+1, ids[j]);
                } else {
                    used.insert(ids[j]);
                }
            }
        }

        if (!valid) {
            totalCaseScore += 0.0;
            caseCount++;
            continue;
        }

        // ---------- Compute U ----------
        ll U = 0;
        // sum of beauty
        for (int j = 0; j < n; j++)
            U += P[ids[j]];

        // compute oriented triples
        vector<A3> triples(n);
        for (int j = 0; j < n; j++)
            triples[j] = applyOri(oris[j], X[ids[j]], Y[ids[j]], Z[ids[j]]);

        // cyclic seam resonances
        for (int j = 0; j < n; j++) {
            int nxt = (j + 1) % n;
            U += gcdf(triples[j][0], triples[nxt][0]);
            U += gcdf(triples[j][1], triples[nxt][1]);
            U += gcdf(triples[j][2], triples[nxt][2]);
        }

        // ---------- Compute B (baseline) ----------
        // Step 1: canonical orientation = lex smallest among 6 orientations
        vector<A3> canonical(m+1);
        for (int i = 1; i <= m; i++) {
            A3 best = applyOri(0, X[i], Y[i], Z[i]);
            for (int o = 1; o < 6; o++) {
                A3 candidate = applyOri(o, X[i], Y[i], Z[i]);
                if (candidate < best) best = candidate;
            }
            canonical[i] = best;
        }

        // Step 2: select n beads with largest p; ties: smaller canonical, then smaller index
        vector<int> order(m);
        iota(order.begin(), order.end(), 1); // 1-indexed
        sort(order.begin(), order.end(), [&](int a, int b) {
            if (P[a] != P[b])                return P[a] > P[b];
            if (canonical[a] != canonical[b]) return canonical[a] < canonical[b];
            return a < b;
        });
        vector<int> sel(order.begin(), order.begin() + n);

        // Step 3: arrange in nondecreasing canonical order; ties by smaller index
        sort(sel.begin(), sel.end(), [&](int a, int b) {
            if (canonical[a] != canonical[b]) return canonical[a] < canonical[b];
            return a < b;
        });

        // Step 4: compute B
        ll B = 0;
        for (int i = 0; i < n; i++)
            B += P[sel[i]];

        for (int i = 0; i < n; i++) {
            int nxt = (i + 1) % n;
            B += gcdf(canonical[sel[i]][0], canonical[sel[nxt]][0]);
            B += gcdf(canonical[sel[i]][1], canonical[sel[nxt]][1]);
            B += gcdf(canonical[sel[i]][2], canonical[sel[nxt]][2]);
        }

        // ---------- Per-case score: 1,000,000 * clamp(U/B, 0, 2) / 2 ----------
        double caseScore;
        if (B <= 0) {
            caseScore = (U > 0) ? 1000000.0 : 0.0;
        } else {
            double ratio = (double)U / (double)B;
            double clamped = min(2.0, max(0.0, ratio));
            caseScore = 1000000.0 * clamped / 2.0;
        }
        totalCaseScore += caseScore;
        caseCount++;
    }

    // ---------- EOF check: reject trailing garbage ----------
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output after all test cases");
    }

    // ---------- Final score: floor(sum / T) / 1,000,000 ----------
    double finalIntegerScore = 0.0;
    if (caseCount > 0) {
        finalIntegerScore = floor(totalCaseScore / (double)caseCount);
    }
    double finalRatio = finalIntegerScore / 1000000.0;
    finalRatio = min(1.0, max(0.0, finalRatio));

    quitp(finalRatio, "Score: %.0f Ratio: %.9f", finalIntegerScore, finalRatio);
}