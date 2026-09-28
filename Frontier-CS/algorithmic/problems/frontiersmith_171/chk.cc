#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read problem input ──────────────────────────────────────────────────
    int n       = inf.readInt();
    int m       = inf.readInt();
    int K       = inf.readInt();
    long long B = inf.readLong();

    vector<long long> w(n + 1);
    long long W = 0;
    for (int i = 1; i <= n; i++) {
        w[i] = inf.readLong();
        W   += w[i];
    }

    vector<int>       ru(m + 1), rv(m + 1);
    vector<long long> rr(m + 1), rp(m + 1);
    for (int e = 1; e <= m; e++) {
        ru[e] = inf.readInt();
        rv[e] = inf.readInt();
        rr[e] = inf.readLong();
        rp[e] = inf.readLong();
    }

    // ── Read participant output ─────────────────────────────────────────────
    int q = ouf.readInt();
    if (q < 0 || q > K)
        quitf(_wa, "q=%d is outside [0, K=%d]", q, K);

    vector<bool> covered(n + 1, false);
    long long total_capital = 0;
    long long total_L       = 0;

    for (int ci = 1; ci <= q; ci++) {

        // Header line: s A L z
        int       s = ouf.readInt();
        long long A = ouf.readLong();
        long long L = ouf.readLong();
        long long z = ouf.readLong();

        if (s < 1 || s > n)
            quitf(_wa, "Caravan %d: start city s=%d out of [1,%d]", ci, s, n);
        if (A < 0)
            quitf(_wa, "Caravan %d: initial capital A=%lld < 0", ci, A);
        if (L < 1)
            quitf(_wa, "Caravan %d: L=%lld < 1", ci, L);
        if (z < 0 || z >= L)
            quitf(_wa, "Caravan %d: z=%lld not in [0, L-1=%lld]", ci, z, L - 1);

        total_L += L;
        if (total_L > 400000LL)
            quitf(_wa, "Total walk length %lld exceeds 400000", total_L);

        total_capital += A;
        if (total_capital > B)
            quitf(_wa, "Sum of initial capitals %lld exceeds B=%lld", total_capital, B);

        // Road list
        vector<int> roads((int)L);
        for (int i = 0; i < (int)L; i++) {
            roads[i] = ouf.readInt();
            if (roads[i] < 1 || roads[i] > m)
                quitf(_wa, "Caravan %d: road id %d out of [1,%d]", ci, roads[i], m);
        }

        // ── Simulate ──────────────────────────────────────────────────────
        long long capital    = A;
        int       cur_city   = s;
        long long C_cycle    = A;
        int       cycle_start = s;

        covered[s] = true;

        for (int t = 0; t < (int)L; t++) {
            int e = roads[t];

            // Connectivity check
            if (ru[e] != cur_city)
                quitf(_wa, "Caravan %d: road %d starts at city %d but current city is %d (step %d)",
                      ci, e, ru[e], cur_city, t + 1);

            // Capital requirement
            if (capital < rr[e])
                quitf(_wa, "Caravan %d: capital %lld < required %lld for road %d at step %d",
                      ci, capital, rr[e], e, t + 1);

            capital += rp[e];

            // Capital must stay non-negative
            if (capital < 0)
                quitf(_wa, "Caravan %d: capital became %lld (negative) after step %d",
                      ci, capital, t + 1);

            cur_city = rv[e];
            covered[cur_city] = true;

            // Record cycle start state right after prefix
            if (t == (int)z - 1) {
                C_cycle     = capital;
                cycle_start = cur_city;
            }
        }

        // If z == 0, cycle start is city s with capital A
        if (z == 0) {
            cycle_start = s;
            C_cycle     = A;
        }

        // Cycle must return to its start city
        if (cur_city != cycle_start)
            quitf(_wa, "Caravan %d: cycle does not return to its start city "
                       "(expected %d, got %d after %lld roads)",
                  ci, cycle_start, cur_city, L);

        // Cycle must not lose capital
        if (capital < C_cycle)
            quitf(_wa, "Caravan %d: cycle capital decreased from %lld to %lld",
                  ci, C_cycle, capital);
    }

    // ── EOF check: reject any trailing non-whitespace tokens ───────────────
    if (!ouf.seekEof())
        quitf(_wa, "extra output after the last caravan description");

    // ── Compute objective ───────────────────────────────────────────────────
    long long O = 0;
    for (int i = 1; i <= n; i++)
        if (covered[i]) O += w[i];

    // Score ratio in [0,1]: ratio = O / W  (O_base = 0 per problem statement,
    // so score = 100 * O / W, which equals 100 * ratio).
    // quitp expects a value in [0,1]; the judge multiplies by 100 for the
    // per-file score, matching the statement formula score = 100 * O / W.
    double ratio = (W > 0) ? (double)O / (double)W : 1.0;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
          "Covered=%lld TotalW=%lld q=%d TotalL=%lld TotalCapital=%lld "
          "Ratio: %f",
          O, W, q, total_L, total_capital, ratio);

    return 0;
}