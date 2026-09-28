#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef __int128 lll;

static string lll_to_string(lll v) {
    if (v == 0) return "0";
    bool neg = (v < 0);
    if (neg) v = -v;
    string s;
    while (v > 0) { s += char('0' + (int)(v % 10)); v /= 10; }
    if (neg) s += '-';
    reverse(s.begin(), s.end());
    return s;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input ----
    ll n, W, H, Vmax, Hmax, S, CV, CH;
    n    = inf.readLong();
    W    = inf.readLong();
    H    = inf.readLong();
    Vmax = inf.readLong();
    Hmax = inf.readLong();
    S    = inf.readLong();
    CV   = inf.readLong();
    CH   = inf.readLong();

    vector<ll> wi(n), hi(n), di(n), ri(n);
    for (int i = 0; i < n; i++) {
        wi[i] = inf.readLong();
        hi[i] = inf.readLong();
        di[i] = inf.readLong();
        ri[i] = inf.readLong();
    }

    // ---- Read participant output ----
    // Read kx and ky
    ll kx, ky;
    if (ouf.seekEof()) {
        quitf(_wa, "output is empty");
    }
    kx = ouf.readLong();
    ky = ouf.readLong();

    // Feasibility checks on kx, ky
    if (kx < 1 || kx > Vmax + 1) {
        quitf(_wa, "kx=%lld is out of range [1, %lld]", kx, Vmax + 1);
    }
    if (ky < 1 || ky > Hmax + 1) {
        quitf(_wa, "ky=%lld is out of range [1, %lld]", ky, Hmax + 1);
    }

    vector<ll> xs(kx), ys(ky);
    for (int i = 0; i < (int)kx; i++) {
        xs[i] = ouf.readLong();
        if (xs[i] < 1) quitf(_wa, "x[%d]=%lld must be positive", i, xs[i]);
    }
    for (int j = 0; j < (int)ky; j++) {
        ys[j] = ouf.readLong();
        if (ys[j] < 1) quitf(_wa, "y[%d]=%lld must be positive", j, ys[j]);
    }

    // NOTE: Do NOT call ouf.readEof() here.
    // Candidate solutions often print trailing spaces/newlines which are harmless.

    // Feasibility: sums must equal W and H
    ll sumX = 0, sumY = 0;
    for (int i = 0; i < (int)kx; i++) sumX += xs[i];
    for (int j = 0; j < (int)ky; j++) sumY += ys[j];
    if (sumX != W) quitf(_wa, "Sum of x-strips=%lld != W=%lld", sumX, W);
    if (sumY != H) quitf(_wa, "Sum of y-strips=%lld != H=%lld", sumY, H);

    // ---- Compute OBJ for participant solution ----
    // Build frequency maps
    map<ll, ll> cntW_map, cntH_map;
    for (int i = 0; i < (int)kx; i++) cntW_map[xs[i]]++;
    for (int j = 0; j < (int)ky; j++) cntH_map[ys[j]]++;

    lll Revenue  = 0;
    lll SoldArea = 0;

    for (int i = 0; i < n; i++) {
        ll cw = 0, ch = 0;
        auto itW = cntW_map.find(wi[i]);
        if (itW != cntW_map.end()) cw = itW->second;
        auto itH = cntH_map.find(hi[i]);
        if (itH != cntH_map.end()) ch = itH->second;

        lll produced = (lll)cw * ch;
        lll sold     = (produced < (lll)di[i]) ? produced : (lll)di[i];

        Revenue  += sold * (lll)ri[i];
        SoldArea += sold * (lll)wi[i] * (lll)hi[i];
    }

    lll ScrapArea = (lll)W * H - SoldArea;
    lll CutsCost  = (lll)(kx - 1) * CV + (lll)(ky - 1) * CH;
    lll OBJ       = Revenue - (lll)S * ScrapArea - CutsCost;

    // ---- Compute L ----
    lll L = (lll)S * W * H + (lll)CV * Vmax + (lll)CH * Hmax;

    // ---- Compute BASE (baseline) ----
    // OBJ_0: do nothing (kx=1, ky=1, no sales)
    lll OBJ_0 = -(lll)S * W * H;
    lll BASE  = OBJ_0;

    for (int i = 0; i < n; i++) {
        if (wi[i] > W || hi[i] > H) continue;

        ll ax = W / wi[i];
        ll rx = W - ax * wi[i];
        ll sx = ax + (rx > 0 ? 1 : 0);  // number of x-strips

        ll ay = H / hi[i];
        ll ry = H - ay * hi[i];
        ll sy = ay + (ry > 0 ? 1 : 0);  // number of y-strips

        if ((sx - 1) > Vmax || (sy - 1) > Hmax) continue;

        lll sold_i = (lll)di[i] < (lll)ax * ay ? (lll)di[i] : (lll)ax * ay;
        lll OBJ_i  = sold_i * (lll)ri[i]
                     - (lll)S * ((lll)W * H - sold_i * (lll)wi[i] * (lll)hi[i])
                     - (lll)(sx - 1) * CV
                     - (lll)(sy - 1) * CH;

        if (OBJ_i > BASE) BASE = OBJ_i;
    }

    // ---- Compute score ratio ----
    // score = floor(1,000,000 * min(2, (OBJ + L + 1) / (BASE + L + 1)))
    // We map this to quitp ratio in [0,1]:
    //   ratio = clamp(OBJ+L+1, 0, BASE+L+1) / (BASE+L+1)
    // so that:
    //   OBJ == BASE  => ratio = 1.0 (full score = 1,000,000)
    //   OBJ > BASE   => ratio = 1.0 (capped; above-baseline bonus handled separately if desired)
    //   OBJ < BASE   => ratio in [0,1) proportional to how close we are
    //   OBJ == -L-1  => ratio = 0.0 (infeasible penalty)
    //
    // NOTE: OBJ >= -L for any feasible solution, so numerator >= 1 when OBJ >= -L.

    lll numerator   = OBJ  + L + (lll)1;
    lll denominator = BASE + L + (lll)1;

    double ratio = 0.0;
    if (denominator <= 0) {
        // Degenerate: shouldn't happen since BASE >= OBJ_0 = -S*W*H >= -L
        ratio = 1.0;
    } else {
        // ratio = numerator / denominator, capped to [0, 1]
        // We use long double for precision
        long double num_ld = (long double)numerator;
        long double den_ld = (long double)denominator;
        long double r = num_ld / den_ld;
        // Allow up to 2x baseline to map to ratio up to 1.0
        // (so above-baseline solutions also get ratio=1.0 max)
        if (r < 0.0L) r = 0.0L;
        if (r > 1.0L) r = 1.0L;
        ratio = (double)r;
    }

    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio, "OBJ=%s BASE=%s L=%s Ratio: %f",
          lll_to_string(OBJ).c_str(),
          lll_to_string(BASE).c_str(),
          lll_to_string(L).c_str(),
          ratio);
}