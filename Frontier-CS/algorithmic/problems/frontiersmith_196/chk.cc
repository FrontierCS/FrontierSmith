#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read problem input ──────────────────────────────────────────────────
    int N = inf.readInt();
    int M = inf.readInt();
    long long B = inf.readLong();

    struct Schedule { long long v, w; };
    struct Route    { long long L; vector<Schedule> schedules; };
    struct Option   { long long c, l, r; };
    struct Train    { int route; vector<Option> options; };

    vector<Route> routes(N);
    for (int i = 0; i < N; i++) {
        routes[i].L = inf.readLong();
        int H = inf.readInt();
        routes[i].schedules.resize(H);
        for (int k = 0; k < H; k++) {
            routes[i].schedules[k].v = inf.readLong();
            routes[i].schedules[k].w = inf.readLong();
        }
    }

    vector<Train> trains(M);
    for (int j = 0; j < M; j++) {
        trains[j].route = inf.readInt() - 1; // convert to 0-indexed
        int D = inf.readInt();
        trains[j].options.resize(D);
        for (int t = 0; t < D; t++) {
            trains[j].options[t].c = inf.readLong();
            trains[j].options[t].l = inf.readLong();
            trains[j].options[t].r = inf.readLong();
        }
    }

    // ── Read participant output ─────────────────────────────────────────────
    vector<int> chosen(M);
    for (int j = 0; j < M; j++) {
        chosen[j] = ouf.readInt();
    }
    // Ensure no trailing tokens
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra tokens found in output after reading %d option indices", M);
    }

    // ── Feasibility checks ──────────────────────────────────────────────────
    for (int j = 0; j < M; j++) {
        int D = (int)trains[j].options.size();
        if (chosen[j] < 1 || chosen[j] > D) {
            quitf(_wa,
                "Train %d: chosen option %d is out of valid range [1, %d]",
                j + 1, chosen[j], D);
        }
    }

    // Check budget (use saturating addition to avoid overflow)
    long long total_cost = 0;
    for (int j = 0; j < M; j++) {
        long long c = trains[j].options[chosen[j] - 1].c;
        if (total_cost > B - c) { // i.e. total_cost + c > B, overflow-safe
            quitf(_wa,
                "Total cost of chosen options exceeds budget B=%lld",
                B);
        }
        total_cost += c;
    }

    // ── Helper: count multiples of v in [lo, hi] ───────────────────────────
    auto countMultiples = [](long long lo, long long hi, long long v) -> long long {
        if (lo > hi || v <= 0) return 0LL;
        return hi / v - (lo - 1) / v;
    };

    // ── Helper: compute objective for an assignment ────────────────────────
    auto computeValue = [&](const vector<int>& asgn) -> long long {
        long long val = 0;
        for (int i = 0; i < N; i++) {
            // Collect intervals for route i
            vector<pair<long long, long long>> segs;
            for (int j = 0; j < M; j++) {
                if (trains[j].route == i) {
                    const Option& opt = trains[j].options[asgn[j] - 1];
                    segs.push_back({opt.l, opt.r});
                }
            }
            // Merge intervals
            sort(segs.begin(), segs.end());
            vector<pair<long long, long long>> merged;
            for (auto& s : segs) {
                if (!merged.empty() && s.first <= merged.back().second + 1) {
                    merged.back().second = max(merged.back().second, s.second);
                } else {
                    merged.push_back(s);
                }
            }
            long long L = routes[i].L;
            for (const Schedule& sc : routes[i].schedules) {
                long long total_lamps = L / sc.v;
                long long hidden = 0;
                for (auto& m : merged) {
                    long long lo = m.first;
                    long long hi = min(m.second, L);
                    hidden += countMultiples(lo, hi, sc.v);
                }
                val += sc.w * (total_lamps - hidden);
            }
        }
        return val;
    };

    // ── Compute VAL, BASE, MAXFREE ─────────────────────────────────────────
    long long VAL = computeValue(chosen);

    vector<int> baseline(M, 1);
    long long BASE = computeValue(baseline);

    long long MAXFREE = 0;
    for (int i = 0; i < N; i++) {
        long long L = routes[i].L;
        for (const Schedule& sc : routes[i].schedules) {
            MAXFREE += sc.w * (L / sc.v);
        }
    }

    // ── Scoring ────────────────────────────────────────────────────────────
    if (MAXFREE == BASE) {
        // Full score regardless of VAL
        quitp(1.0,
            "MAXFREE equals BASE (no room for improvement). "
            "VAL=%lld BASE=%lld MAXFREE=%lld Ratio: 1.0",
            VAL, BASE, MAXFREE);
    }

    double ratio = (double)(VAL - BASE) / (double)(MAXFREE - BASE);
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
        "VAL=%lld BASE=%lld MAXFREE=%lld Ratio: %.9f",
        VAL, BASE, MAXFREE, ratio);

    return 0;
}