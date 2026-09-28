#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read problem input from inf ─────────────────────────────────────────
    // First line: N H W K
    int N  = inf.readInt();
    long long H = inf.readLong();
    long long W = inf.readLong();
    int K  = inf.readInt();

    vector<long long> sv(N+1), pv(N+1);
    for (int i = 1; i <= N; i++) {
        sv[i] = inf.readLong();
        pv[i] = inf.readLong();
    }

    vector<double> uj(K+1), vj(K+1), qj(K+1), rj(K+1);
    for (int j = 1; j <= K; j++) {
        uj[j] = (double)inf.readLong();
        vj[j] = (double)inf.readLong();
        qj[j] = (double)inf.readLong();
        rj[j] = (double)inf.readLong();
    }

    // ── Engagement helper ───────────────────────────────────────────────────
    auto computeEngagement = [&](int i, long long xi, long long yi) -> double {
        double cx = xi + sv[i] / 2.0;
        double cy = yi + sv[i] / 2.0;
        double bonus_sum = 0.0;
        for (int j = 1; j <= K; j++) {
            double dx = cx - uj[j];
            double dy = cy - vj[j];
            double d  = sqrt(dx*dx + dy*dy);
            bonus_sum += qj[j] * rj[j] / (rj[j] + d);
        }
        return (double)pv[i] * (1.0 + bonus_sum);
    };

    // ── Compute baseline (FFDH) ─────────────────────────────────────────────
    // Step 1: filter artworks that fit
    vector<int> validIdx;
    for (int i = 1; i <= N; i++)
        if (sv[i] <= H && sv[i] <= W)
            validIdx.push_back(i);

    // Step 2: sort decreasing s, decreasing p, increasing id
    sort(validIdx.begin(), validIdx.end(), [&](int a, int b) {
        if (sv[a] != sv[b]) return sv[a] > sv[b];
        if (pv[a] != pv[b]) return pv[a] > pv[b];
        return a < b;
    });

    // Step 3: FFDH shelves
    struct Shelf { long long y, h, x_used; };
    vector<Shelf> shelves;
    long long total_height = 0;

    vector<long long> bx(N+1, -1), by_coord(N+1, -1);

    for (int idx : validIdx) {
        long long si = sv[idx];
        bool placed = false;

        // Try to find an existing shelf
        for (auto& shelf : shelves) {
            if (shelf.h >= si && shelf.x_used + si <= W) {
                bx[idx] = shelf.x_used;
                by_coord[idx] = shelf.y;
                shelf.x_used += si;
                placed = true;
                break;
            }
        }

        if (!placed) {
            // Try to open a new shelf
            if (total_height + si <= H) {
                Shelf ns;
                ns.y = total_height;
                ns.h = si;
                ns.x_used = si;
                bx[idx] = 0;
                by_coord[idx] = total_height;
                total_height += si;
                shelves.push_back(ns);
            }
            // else: skip
        }
    }

    // Compute baseline objective B
    double B = 0.0;
    for (int i = 1; i <= N; i++) {
        if (bx[i] >= 0)
            B += computeEngagement(i, bx[i], by_coord[i]);
    }

    // ── Read participant output from ouf ────────────────────────────────────
    int T = ouf.readInt();
    if (T < 0 || T > N)
        quitf(_wa, "T=%d is out of range [0, %d]", T, N);

    vector<int> placed_ids;
    vector<long long> px(N+1, -1), py(N+1, -1);
    vector<bool> used(N+1, false);

    for (int t = 0; t < T; t++) {
        int id = ouf.readInt();
        long long x  = ouf.readLong();
        long long y  = ouf.readLong();

        if (id < 1 || id > N)
            quitf(_wa, "Invalid artwork id=%d (must be in [1,%d])", id, N);
        if (used[id])
            quitf(_wa, "Artwork id=%d placed more than once", id);

        // Check wall bounds
        if (x < 0 || y < 0 || x + sv[id] > W || y + sv[id] > H)
            quitf(_wa, "Artwork %d at (%lld,%lld) with side %lld is out of wall [0,%lld]x[0,%lld]",
                  id, x, y, sv[id], W, H);

        used[id] = true;
        px[id] = x;
        py[id] = y;
        placed_ids.push_back(id);
    }

    // Check EOF
    if (!ouf.seekEof())
        quitf(_wa, "Extra output after the last placed artwork");

    // Check non-overlapping interiors
    for (int ii = 0; ii < (int)placed_ids.size(); ii++) {
        int i = placed_ids[ii];
        for (int jj_idx = ii + 1; jj_idx < (int)placed_ids.size(); jj_idx++) {
            int jj = placed_ids[jj_idx];
            long long xi1 = px[i],  xi2 = px[i]  + sv[i];
            long long yi1 = py[i],  yi2 = py[i]  + sv[i];
            long long xj1 = px[jj], xj2 = px[jj] + sv[jj];
            long long yj1 = py[jj], yj2 = py[jj] + sv[jj];
            // Interiors overlap iff strictly overlapping on both axes
            if (xi1 < xj2 && xj1 < xi2 && yi1 < yj2 && yj1 < yi2)
                quitf(_wa, "Artworks %d and %d have overlapping interiors", i, jj);
        }
    }

    // ── Compute participant objective U ─────────────────────────────────────
    double U = 0.0;
    for (int id : placed_ids)
        U += computeEngagement(id, px[id], py[id]);

    // ── Scoring ─────────────────────────────────────────────────────────────
    // Per problem statement:
    //   if B > 0: per-test score = 100 * clamp(U/B, 0, 2)   (max 200)
    //   if B = 0: score = 100 if U = 0, else 0
    //
    // config.yaml declares subtask score = 200.
    // The Frontier-CS judge computes: final_score = 200 * ratio
    //
    // Mapping:
    //   B > 0:  ratio = clamp(U/B, 0, 2) / 2   in [0, 1]
    //     U = 0    -> ratio = 0.0  -> score =   0
    //     U = B    -> ratio = 0.5  -> score = 100  (matches baseline)
    //     U = 2*B  -> ratio = 1.0  -> score = 200  (maximum)
    //   B = 0, U = 0: ratio = 0.5  -> score = 100
    //   B = 0, U > 0: ratio = 0.0  -> score =   0

    double ratio;
    if (B > 1e-9) {
        double r_val = U / B;
        if (r_val < 0.0) r_val = 0.0;
        if (r_val > 2.0) r_val = 2.0;
        ratio = r_val / 2.0;  // normalise to [0, 1]
    } else {
        // B == 0
        ratio = (U < 1e-9) ? 0.5 : 0.0;
    }

    quitp(ratio, "Ratio: %.9f (U=%.6f, B=%.6f, placed=%d)", ratio, U, B, T);
    return 0;
}