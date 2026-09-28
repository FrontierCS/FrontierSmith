#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read problem input from inf ---
    long long n, T, S, G, R;
    n = inf.readLong();
    T = inf.readLong();
    S = inf.readLong();
    G = inf.readLong();
    R = inf.readLong();

    vector<long long> tv(n+1), bv(n+1), ev(n+1), gv(n+1), av(n+1);
    for (int i = 1; i <= n; i++) {
        tv[i] = inf.readLong();
        bv[i] = inf.readLong();
        ev[i] = inf.readLong();
        gv[i] = inf.readLong();
        av[i] = inf.readLong();
    }

    // --- Read participant output from ouf ---
    // First line: m
    long long m = ouf.readLong();
    if (m < 0 || m > n) {
        quitf(_wa, "m=%lld is out of range [0, %lld]", m, n);
    }

    // Second line: m indices (may be empty/omitted if m==0)
    vector<long long> playlist;
    playlist.reserve((size_t)m);
    for (long long j = 0; j < m; j++) {
        long long idx = ouf.readLong();
        playlist.push_back(idx);
    }
    // Do NOT call ouf.readEof() — solutions may emit trailing newlines/spaces.

    // --- Validate feasibility ---
    {
        set<long long> seen;
        long long total_dur = 0;
        for (long long j = 0; j < m; j++) {
            long long idx = playlist[j];
            if (idx < 1 || idx > n) {
                quitf(_wa, "Song index %lld is out of range [1, %lld]", idx, n);
            }
            if (seen.count(idx)) {
                quitf(_wa, "Song index %lld appears more than once", idx);
            }
            seen.insert(idx);
            total_dur += tv[idx];
            if (total_dur > T) {
                quitf(_wa, "Total duration %lld exceeds budget %lld", total_dur, T);
            }
        }
    }

    // --- Compute objective for a given playlist ---
    auto computeObjective = [&](vector<long long>& pl) -> long long {
        long long sz = (long long)pl.size();
        if (sz == 0) return 0LL;

        // Base appeal
        long long base_val = 0;
        for (long long j = 0; j < sz; j++) {
            base_val += tv[pl[j]] * bv[pl[j]];
        }

        // Smooth transitions
        long long smooth_val = 0;
        for (long long j = 0; j + 1 < sz; j++) {
            long long u = pl[j], v = pl[j+1];
            long long diff = ev[u] - ev[v];
            if (diff < 0) diff = -diff;
            long long factor = S - diff;
            if (factor > 0) {
                long long mn = (tv[u] < tv[v]) ? tv[u] : tv[v];
                smooth_val += mn * factor;
            }
        }

        // Genre diversity bonus
        set<long long> distinct_genres;
        for (long long j = 0; j < sz; j++) distinct_genres.insert(gv[pl[j]]);
        long long genre_bonus = G * (long long)distinct_genres.size();

        // Artist repetition penalty
        map<long long, long long> artist_cnt;
        for (long long j = 0; j < sz; j++) artist_cnt[av[pl[j]]]++;
        long long penalty = 0;
        for (auto& kv : artist_cnt) {
            long long c = kv.second;
            penalty += c * (c - 1) / 2;
        }
        penalty *= R;

        return base_val + smooth_val + genre_bonus - penalty;
    };

    long long U = computeObjective(playlist);

    // --- Compute baseline B ---
    // Sort by: decreasing b_i, increasing t_i, increasing index
    vector<int> order(n);
    iota(order.begin(), order.end(), 1);
    sort(order.begin(), order.end(), [&](int x, int y) {
        if (bv[x] != bv[y]) return bv[x] > bv[y];
        if (tv[x] != tv[y]) return tv[x] < tv[y];
        return x < y;
    });

    vector<long long> baseline_pl;
    long long used = 0;
    for (int idx : order) {
        if (used + tv[idx] <= T) {
            baseline_pl.push_back((long long)idx);
            used += tv[idx];
        }
    }

    // Sort chosen by: increasing e_i, decreasing b_i, increasing index
    sort(baseline_pl.begin(), baseline_pl.end(), [&](long long x, long long y) {
        if (ev[x] != ev[y]) return ev[x] < ev[y];
        if (bv[x] != bv[y]) return bv[x] > bv[y];
        return x < y;
    });

    long long B = computeObjective(baseline_pl);

    // --- Compute score ratio ---
    // score = floor(1_000_000 * clamp(U / B, 0, 2))
    // We pass ratio in [0,1] to quitp where ratio = clamp(U/B, 0, 2) / 2
    double score_ratio = 0.0;
    if (B <= 0) {
        // Degenerate baseline: if participant achieves non-negative, full marks
        score_ratio = (U >= 0) ? 1.0 : 0.0;
    } else {
        double raw = (double)U / (double)B;
        // clamp to [0, 2], then normalize to [0, 1]
        double clamped = raw < 0.0 ? 0.0 : (raw > 2.0 ? 2.0 : raw);
        score_ratio = clamped / 2.0;
    }
    // Safety clamp
    if (score_ratio < 0.0) score_ratio = 0.0;
    if (score_ratio > 1.0) score_ratio = 1.0;

    quitp(score_ratio,
          "Ratio: %.9f | U=%lld B=%lld m=%lld",
          score_ratio, U, B, m);

    return 0;
}