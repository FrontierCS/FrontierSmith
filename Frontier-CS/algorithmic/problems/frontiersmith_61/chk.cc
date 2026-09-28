#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Compute expected streak score given hit probabilities P[0..n-1]
double computeScore(const vector<double>& P) {
    int n = (int)P.size();
    double E = 0.0, R = 0.0;
    for (int i = 0; i < n; i++) {
        double pi = P[i];
        E = E + pi * (2.0 * R + 1.0);
        R = pi * (R + 1.0);
    }
    return E;
}

// Apply a set of chosen modules (0-based indices) to baseMiss using difference arrays
// Returns hit probabilities P[0..n-1]
vector<double> computeP(int n,
                        const vector<double>& baseMiss,
                        const vector<int>& ls,
                        const vector<int>& rs,
                        const vector<double>& fs,
                        const vector<int>& chosen) {
    // For each position, we need product of f_j for chosen modules covering it.
    // Use log-sum difference array trick: sum log(f_j) over [l,r], then exponentiate.
    // But f_j can be 0, so handle carefully.
    // Instead, mark positions that are zeroed out separately.
    
    // We'll do a simple O(sum of interval lengths) approach but cap at a reasonable size.
    // For the checker baseline we use a smarter approach below.
    
    vector<double> logMiss(n, 0.0); // log of miss factor
    vector<bool> zeroOut(n, false);
    
    for (int jidx : chosen) {
        int l = ls[jidx], r = rs[jidx];
        double f = fs[jidx];
        if (f <= 0.0) {
            for (int i = l; i <= r; i++) zeroOut[i] = true;
        } else {
            double lf = log(f);
            for (int i = l; i <= r; i++) logMiss[i] += lf;
        }
    }
    
    vector<double> Pout(n);
    for (int i = 0; i < n; i++) {
        if (zeroOut[i]) {
            Pout[i] = 1.0; // miss prob = 0
        } else {
            double missFactor = exp(logMiss[i]);
            Pout[i] = 1.0 - baseMiss[i] * missFactor;
        }
    }
    return Pout;
}

// Efficient single-module application using the already-computed base score
// For baseline: compute V_j for each module j individually
// Use difference array over log for a single module: trivial since it's one segment
double computeScoreSingleModule(int n,
                                 const vector<double>& baseMiss,
                                 int l, int r, double f) {
    // Build P directly
    vector<double> P(n);
    if (f <= 0.0) {
        for (int i = 0; i < n; i++) {
            if (i >= l && i <= r) P[i] = 1.0;
            else P[i] = 1.0 - baseMiss[i];
        }
    } else {
        for (int i = 0; i < n; i++) {
            if (i >= l && i <= r) P[i] = 1.0 - baseMiss[i] * f;
            else P[i] = 1.0 - baseMiss[i];
        }
    }
    return computeScore(P);
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input from inf ----
    int n = inf.readInt();
    int m = inf.readInt();
    long long B = inf.readLong();

    vector<double> p(n);
    for (int i = 0; i < n; i++) p[i] = inf.readDouble();

    vector<int> ls(m), rs(m);
    vector<long long> cs(m);
    vector<double> fs(m);
    for (int j = 0; j < m; j++) {
        ls[j] = inf.readInt() - 1; // 0-based
        rs[j] = inf.readInt() - 1; // 0-based
        cs[j] = inf.readLong();
        fs[j] = inf.readDouble();
    }

    // Base miss probabilities
    vector<double> baseMiss(n);
    for (int i = 0; i < n; i++) baseMiss[i] = 1.0 - p[i];

    // ---- Read participant output from ouf ----
    // First token: k
    int k = ouf.readInt(0, m, "k must be between 0 and m");

    vector<int> chosen; // 0-based module indices
    set<int> chosenSet;
    long long totalCost = 0;

    // Read k module indices (1-based in output, convert to 0-based)
    for (int t = 0; t < k; t++) {
        int idx = ouf.readInt(1, m, "module index must be between 1 and m");
        int idx0 = idx - 1;
        if (chosenSet.count(idx0)) {
            quitf(_wa, "Duplicate module index %d", idx);
        }
        chosenSet.insert(idx0);
        chosen.push_back(idx0);
        // Check cost overflow
        if (cs[idx0] > B - totalCost + totalCost) { /* handled below */ }
        totalCost += cs[idx0];
    }

    // Consume any remaining whitespace/newlines before checking feasibility
    // (testlib readInt already skips whitespace, so just proceed)

    // Check budget feasibility
    if (totalCost > B) {
        quitf(_wa, "Total cost %lld exceeds budget %lld", (long long)totalCost, (long long)B);
    }

    // ---- Compute V_sub ----
    vector<double> P_sub = computeP(n, baseMiss, ls, rs, fs, chosen);
    double V_sub = computeScore(P_sub);

    // ---- Compute V_empty ----
    vector<double> P_empty(n);
    for (int i = 0; i < n; i++) P_empty[i] = p[i];
    double V_empty = computeScore(P_empty);

    // ---- Compute baseline ----
    // For each module j, compute V_j (only module j chosen)
    // To keep this O(n*m) manageable we need to be smart.
    // For n,m up to 200000, a naive O(n) per module is 4*10^10 ops -- too slow.
    // Use the recurrence structure: only notes in [l_j, r_j] differ.
    // Split computation into three segments: [0,l-1], [l,r], [r+1,n-1]
    // Precompute prefix info for the recurrence.

    // Precompute forward recurrence on base probabilities
    // E[i], R[i] = state after processing notes 0..i-1 (0-indexed, so before note i)
    vector<double> fwdE(n + 1, 0.0), fwdR(n + 1, 0.0);
    for (int i = 0; i < n; i++) {
        double pi = p[i];
        fwdE[i + 1] = fwdE[i] + pi * (2.0 * fwdR[i] + 1.0);
        fwdR[i + 1] = pi * (fwdR[i] + 1.0);
    }

    // For the segment [l..r] with modified probabilities, we need to propagate
    // the recurrence. The recurrence is linear in (E, R, 1):
    // E_{i+1} = E_i + p_{i+1}*(2*R_i + 1) = E_i*1 + R_i*(2*p_{i+1}) + p_{i+1}
    // R_{i+1} = p_{i+1}*(R_i + 1) = R_i*p_{i+1} + p_{i+1}
    // State vector: (E, R, 1) -- 3D linear transform

    // For a segment of notes with given probs q[l..r], the transform matrix M is:
    // [E']   [1  2q_l  q_l ] [E ]
    // [R'] = [0  q_l   q_l ] [R ]
    // [1 ]   [0  0     1   ] [1 ]
    // Compose these matrices for the segment.

    // But composing n matrices per module is still O(n*m). 
    // Instead, precompute suffix info: given (E_r, R_r) after note r, 
    // what's the final (E_n, R_n)?
    // The suffix transform is linear: E_n = a*E_r + b*R_r + c, R_n = d*E_r + e*R_r + g
    // Since E doesn't feed back into R, R_n = e*R_r + g (E_r coefficient is 0)
    // E_n = a*E_r + b*R_r + c

    // Precompute suffix: from state (E, R) after note i (0-indexed, meaning we've processed 0..i-1)
    // to final state (E_n, R_n):
    // sfxA[i]*E + sfxB[i]*R + sfxC[i] = E_n
    // sfxD[i]*R + sfxF[i] = R_n  (sfxD[i]=0 trivially since E doesn't affect R... wait)
    // Actually E_n does depend on R_i but not E_i in the R recurrence.
    // The recurrence: E_{i+1} = E_i + p_{i+1}*(2*R_i+1), R_{i+1} = p_{i+1}*(R_i+1)
    // So R only depends on R (not E), but E depends on both E and R.

    // sfxRa[i] = coefficient of R_i in R_n: product of p_{i+1}*...*p_n... 
    // Let's define: given state (E, R) at position i, final E_n = a_i*E + b_i*R + c_i
    // and R_n = d_i * R + e_i

    // Base: i=n: E_n = 1*E + 0*R + 0, R_n = 1*R + 0
    // For i < n, note i+1 (0-indexed) has prob p[i]:
    // (E', R') at i+1 given (E,R) at i... wait, let me redefine.
    // State after processing notes 0..i-1 is (E_i, R_i).
    // sfx: given (E, R) = (E_i, R_i), compute (E_n, R_n).
    // E_n = sfxA[i]*E + sfxB[i]*R + sfxC[i]
    // R_n = sfxD[i]*R + sfxE2[i]   (A coefficient for R is 0)

    vector<double> sfxA(n + 1), sfxB(n + 1), sfxC(n + 1);
    vector<double> sfxD(n + 1), sfxE2(n + 1);
    sfxA[n] = 1.0; sfxB[n] = 0.0; sfxC[n] = 0.0;
    sfxD[n] = 1.0; sfxE2[n] = 0.0;

    for (int i = n - 1; i >= 0; i--) {
        double pi = p[i];
        // Process note i: E' = E + pi*(2R+1), R' = pi*(R+1)
        // Then apply suffix from i+1:
        // E_n = sfxA[i+1]*E' + sfxB[i+1]*R' + sfxC[i+1]
        //     = sfxA[i+1]*(E + pi*(2R+1)) + sfxB[i+1]*pi*(R+1) + sfxC[i+1]
        //     = sfxA[i+1]*E + (sfxA[i+1]*2*pi + sfxB[i+1]*pi)*R + (sfxA[i+1]*pi + sfxB[i+1]*pi + sfxC[i+1])
        // R_n = sfxD[i+1]*R' + sfxE2[i+1]
        //     = sfxD[i+1]*pi*(R+1) + sfxE2[i+1]
        //     = sfxD[i+1]*pi*R + (sfxD[i+1]*pi + sfxE2[i+1])
        sfxA[i] = sfxA[i + 1];
        sfxB[i] = sfxA[i + 1] * 2.0 * pi + sfxB[i + 1] * pi;
        sfxC[i] = sfxA[i + 1] * pi + sfxB[i + 1] * pi + sfxC[i + 1];
        sfxD[i] = sfxD[i + 1] * pi;
        sfxE2[i] = sfxD[i + 1] * pi + sfxE2[i + 1];
    }

    // Now for module j covering [l_j, r_j] with factor f_j:
    // The modified prob for note i in [l_j, r_j] is: p_i' = 1 - baseMiss[i]*f_j
    // We simulate the recurrence over [l_j, r_j] starting from state (fwdE[l_j], fwdR[l_j])
    // then apply suffix from r_j+1.
    // The segment [l_j, r_j] has at most n notes, which is O(n) per module -- still O(n*m) total.
    // But in practice for this checker, n*m <= 200000*200000 is too slow.
    // We need segment length sum to be manageable.
    // 
    // Since checker has 4s and this is evaluation (not solution), we'll limit:
    // If total segment work > 10^7, fall back to approximate baseline using V_empty.
    
    long long totalSegWork = 0;
    for (int j = 0; j < m; j++) totalSegWork += (rs[j] - ls[j] + 1);
    
    // Compute V_j for each module
    vector<double> Vj(m, V_empty);
    
    if (totalSegWork <= 20000000LL) {
        // Exact computation
        for (int j = 0; j < m; j++) {
            int l = ls[j], r = rs[j];
            double fj = fs[j];
            // Simulate over [l, r] with modified probs
            double E = fwdE[l], R = fwdR[l];
            for (int i = l; i <= r; i++) {
                double pi_mod = 1.0 - baseMiss[i] * fj;
                double Enew = E + pi_mod * (2.0 * R + 1.0);
                double Rnew = pi_mod * (R + 1.0);
                E = Enew; R = Rnew;
            }
            // Apply suffix from r+1
            double En = sfxA[r + 1] * E + sfxB[r + 1] * R + sfxC[r + 1];
            Vj[j] = En;
        }
    } else {
        // Fall back: use only per-note approximation for short segments
        // Or just do it anyway -- the checker has 4s and the solutions do work too
        // Let's just compute it; the checker runs once per test
        for (int j = 0; j < m; j++) {
            int l = ls[j], r = rs[j];
            double fj = fs[j];
            double E = fwdE[l], R = fwdR[l];
            for (int i = l; i <= r; i++) {
                double pi_mod = 1.0 - baseMiss[i] * fj;
                double Enew = E + pi_mod * (2.0 * R + 1.0);
                double Rnew = pi_mod * (R + 1.0);
                E = Enew; R = Rnew;
            }
            double En = sfxA[r + 1] * E + sfxB[r + 1] * R + sfxC[r + 1];
            Vj[j] = En;
        }
    }

    // Compute density for each module
    vector<double> density(m);
    for (int j = 0; j < m; j++) {
        if (cs[j] == 0) {
            density[j] = 1e18;
        } else {
            density[j] = (Vj[j] - V_empty) / (double)cs[j];
        }
    }

    // Sort modules for baseline
    vector<int> order(m);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        if (density[a] != density[b]) return density[a] > density[b];
        if (cs[a] != cs[b]) return cs[a] < cs[b];
        return a < b;
    });

    // Greedy baseline selection
    vector<int> baselineChosen;
    long long remaining = B;
    for (int jj : order) {
        if (cs[jj] <= remaining) {
            remaining -= cs[jj];
            baselineChosen.push_back(jj);
        }
    }

    // Compute V_base using efficient method
    vector<double> P_base = computeP(n, baseMiss, ls, rs, fs, baselineChosen);
    double V_base = computeScore(P_base);

    // U = n^2
    double U = (double)n * (double)n;

    // Compute per-test score ratio
    double scoreRatio;
    const double eps = 1e-9;
    if (U - V_base < eps) {
        // U == V_base (baseline already achieves maximum)
        if (fabs(V_sub - U) < eps) {
            scoreRatio = 1.0;
        } else {
            scoreRatio = 0.0;
        }
    } else {
        double raw = (V_sub - V_base) / (U - V_base);
        scoreRatio = max(0.0, min(1.0, raw));
    }

    quitp(scoreRatio,
          "Ratio: %.9f | V_sub=%.6f V_base=%.6f V_empty=%.6f U=%.6f k=%d",
          scoreRatio, V_sub, V_base, V_empty, U, k);

    return 0;
}