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
        if (xs[i] < 1 || xs[i] > W) quitf(_wa, "x[%d]=%lld must be positive", i, xs[i]);
    }
    for (int j = 0; j < (int)ky; j++) {
        ys[j] = ouf.readLong();
        if (ys[j] < 1 || ys[j] > H) quitf(_wa, "y[%d]=%lld must be positive", j, ys[j]);
    }

    if (!ouf.seekEof()) quitf(_wa, "extra output after strip lengths");

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

    // Evaluate each uniform pattern against ALL order types it produces,
    // including rectangles involving the remainder strips.
    map<pair<ll,ll>, int> order;
    for (int i = 0; i < n; ++i) order[{wi[i], hi[i]}] = i;
    auto patternObjective = [&](ll w, ll h) -> lll {
        ll ax = W/w, rx = W%w, ay = H/h, ry = H%h;
        ll sx = ax + (rx != 0), sy = ay + (ry != 0);
        if (sx-1 > Vmax || sy-1 > Hmax) return -L-1;
        vector<pair<ll,ll>> vx={{w,ax}}, vy={{h,ay}};
        if(rx) vx.push_back({rx,1});
        if(ry) vy.push_back({ry,1});
        lll profit = -(lll)S*W*H - (lll)(sx-1)*CV - (lll)(sy-1)*CH;
        for(auto [ww,cw]:vx) for(auto [hh,ch]:vy) {
            auto it=order.find({ww,hh});
            if(it==order.end()) continue;
            int i=it->second;
            lll sold=min((lll)di[i],(lll)cw*ch);
            profit+=sold*((lll)ri[i]+(lll)S*wi[i]*hi[i]);
        }
        return profit;
    };
    lll BASE = patternObjective(W,H);
    for(int i=0;i<n;++i) BASE=max(BASE,patternObjective(wi[i],hi[i]));
    // Subtract in 128-bit integer arithmetic BEFORE conversion to avoid
    // catastrophic cancellation with very large scrap penalties.
    lll gain = OBJ - BASE;
    long double ratio = gain > 0 ? (long double)gain / (long double)(OBJ+L+1) : 0.0L;
    quitp((double)ratio, "OBJ=%s BASE=%s L=%s Ratio: %.12f",
          lll_to_string(OBJ).c_str(), lll_to_string(BASE).c_str(),
          lll_to_string(L).c_str(), (double)ratio);
}
