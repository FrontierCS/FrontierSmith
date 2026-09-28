#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef __int128 lll;

static double to_double_128(lll x) {
    if (x == (lll)0) return 0.0;
    if (x < 0) return -(double)(lll)(-x);
    ull lo = (ull)(x & (lll)0xFFFFFFFFFFFFFFFFULL);
    ull hi = (ull)((x >> 64) & (lll)0xFFFFFFFFFFFFFFFFULL);
    return (double)hi * 18446744073709551616.0 + (double)lo;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    // sum of per-test ratios in [0,1]; final ratio = sum / T
    double sum_ratio = 0.0;
    int num_cases = 0;

    for (int t = 0; t < T; t++) {
        num_cases++;

        // Read problem parameters from inf
        int k = inf.readInt();
        int B = inf.readInt();
        ll F = inf.readLong();
        ll G_coeff = inf.readLong();

        vector<ll> c(B);
        for (int b = 0; b < B; b++) {
            c[b] = inf.readLong();
        }

        vector<ll> n_mask(k), w(k);
        for (int i = 0; i < k; i++) {
            n_mask[i] = inf.readLong();
            w[i] = inf.readLong();
        }

        // Trait value function
        auto V = [&](ll x) -> ll {
            ll val = 0;
            for (int b = 0; b < B; b++) {
                if ((x >> b) & 1LL) val += c[b];
            }
            return val;
        };

        // Baseline cost: k avatars each with exact person mask, dissatisfaction = 0
        lll B_base = (lll)0;
        for (int i = 0; i < k; i++) {
            ll vi = V(n_mask[i]);
            B_base += (lll)F + (lll)G_coeff * (lll)vi * (lll)vi;
        }

        // B-bit mask for feasibility check
        ll full_mask;
        if (B >= 63) {
            full_mask = -1LL;
        } else {
            full_mask = (1LL << B) - 1LL;
        }

        // Read participant output for this test case
        int r;
        if (ouf.seekEof()) {
            quitf(_wa, "Test case %d: unexpected end of output while reading r", t + 1);
        }
        r = ouf.readInt();

        if (r < 1 || r > k) {
            quitf(_wa, "Test case %d: r=%d is out of range [1, %d]", t + 1, r, k);
        }

        // Read avatar masks
        vector<ll> m(r);
        for (int j = 0; j < r; j++) {
            if (ouf.seekEof()) {
                quitf(_wa, "Test case %d: unexpected end of output while reading avatar mask %d", t + 1, j + 1);
            }
            m[j] = ouf.readLong();
            if (m[j] < 0 || m[j] > full_mask) {
                quitf(_wa, "Test case %d: avatar %d mask=%lld is out of range [0, 2^B-1]", t + 1, j + 1, m[j]);
            }
        }

        // Read assignments
        vector<int> a(k);
        for (int i = 0; i < k; i++) {
            if (ouf.seekEof()) {
                quitf(_wa, "Test case %d: unexpected end of output while reading assignment for person %d", t + 1, i + 1);
            }
            a[i] = ouf.readInt();
            if (a[i] < 1 || a[i] > r) {
                quitf(_wa, "Test case %d: assignment a[%d]=%d is out of range [1, %d]", t + 1, i + 1, a[i], r);
            }
        }

        // Feasibility: avatar must be a submask of person's mask
        for (int i = 0; i < k; i++) {
            ll av = m[a[i] - 1];
            ll pv = n_mask[i];
            if ((av & (~pv)) & full_mask) {
                quitf(_wa,
                      "Test case %d: avatar %d (mask=%lld) cannot imitate person %d (mask=%lld): "
                      "avatar has bits not in person",
                      t + 1, a[i], av, i + 1, pv);
            }
        }

        // Compute participant solution cost C
        lll C_cost = (lll)0;

        // Deployment costs
        for (int j = 0; j < r; j++) {
            ll vj = V(m[j]);
            C_cost += (lll)F + (lll)G_coeff * (lll)vj * (lll)vj;
        }

        // Dissatisfaction costs
        for (int i = 0; i < k; i++) {
            ll vi_n = V(n_mask[i]);
            ll vi_m = V(m[a[i] - 1]);
            // vi_n >= vi_m since m[a[i]-1] is a submask of n_mask[i]
            C_cost += (lll)w[i] * (lll)(vi_n - vi_m);
        }

        // Per-test ratio = B_base / (B_base + C), in [0,1]
        // Statement score = 10^6 * ratio, but quitp uses [0,1]
        double ratio;
        if (B_base == (lll)0 && C_cost == (lll)0) {
            ratio = 1.0;
        } else if (C_cost <= (lll)0) {
            // C <= 0 means solution is at least as good as perfect: cap at 1
            ratio = 1.0;
        } else {
            double bb = to_double_128(B_base);
            double cc = to_double_128(C_cost);
            if (bb + cc <= 0.0) {
                ratio = 1.0;
            } else {
                ratio = bb / (bb + cc);
            }
            if (ratio > 1.0) ratio = 1.0;
            if (ratio < 0.0) ratio = 0.0;
        }

        sum_ratio += ratio;
    }

    // Check for trailing garbage in participant output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after all test cases");
    }

    // Final score is the arithmetic mean of per-test ratios, in [0,1]
    // (The statement reports scores scaled by 10^6, but quitp expects [0,1])
    double final_ratio = (num_cases > 0) ? (sum_ratio / (double)num_cases) : 0.0;
    if (final_ratio > 1.0) final_ratio = 1.0;
    if (final_ratio < 0.0) final_ratio = 0.0;

    // The message MUST contain "Ratio: <value>" for the judge to parse
    quitp(final_ratio, "Ratio: %f (mean over %d test cases, statement score ~%.2f)",
          final_ratio, num_cases, final_ratio * 1e6);
}