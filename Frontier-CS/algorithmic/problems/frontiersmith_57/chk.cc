#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static int hammingDist(const string &u, const string &v) {
    int h = 0;
    for (int k = 0; k < (int)u.size(); k++) h += (u[k] != v[k]);
    return h;
}

// rho(X, Y) = min over all single swaps on X and on Y of Hamming distance
static int robustGap(const string &X, const string &Y) {
    int n = (int)X.size();
    int best = n;

    int base_ham = hammingDist(X, Y);

    for (int i = 0; i < n && best > 0; i++) {
        for (int j = i; j < n && best > 0; j++) {
            // Compute Ham(swap(X,i,j), Y) incrementally
            int bh = base_ham;
            if (i != j) {
                bh -= (X[i] != Y[i]);
                bh -= (X[j] != Y[j]);
                bh += (X[j] != Y[i]);
                bh += (X[i] != Y[j]);
            }
            // Now try all swaps on Y
            // No swap on Y: cost = bh
            if (bh < best) best = bh;
            if (best == 0) return 0;

            // xp(k): X after swap(i,j)
            auto xp = [&](int k) -> char {
                if (k == i) return X[j];
                if (k == j) return X[i];
                return X[k];
            };

            for (int p = 0; p < n && best > 0; p++) {
                for (int q = p + 1; q < n && best > 0; q++) {
                    // Ham(xp, swap(Y,p,q)) = bh - (xp(p)!=Y[p]) - (xp(q)!=Y[q])
                    //                           + (xp(p)!=Y[q]) + (xp(q)!=Y[p])
                    int val = bh
                        - (xp(p) != Y[p])
                        - (xp(q) != Y[q])
                        + (xp(p) != Y[q])
                        + (xp(q) != Y[p]);
                    if (val < best) best = val;
                }
            }
        }
    }
    return best;
}

static long long cycleObj(const vector<string> &perm) {
    int M = (int)perm.size();
    long long total = 0;
    for (int k = 0; k < M; k++) {
        total += robustGap(perm[k], perm[(k + 1) % M]);
    }
    return total;
}

int main(int argc, char *argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    long long sumScore = 0;

    for (int t = 0; t < T; t++) {
        string A = inf.readToken();
        int M    = inf.readInt();
        int n    = (int)A.size();

        // Build sorted reference for permutation check
        string sortedA = A;
        sort(sortedA.begin(), sortedA.end());

        // Build baseline: first M distinct permutations in lex order
        vector<string> baseline;
        {
            string cur = sortedA;
            do {
                baseline.push_back(cur);
                if ((int)baseline.size() == M) break;
            } while (next_permutation(cur.begin(), cur.end()));
        }
        // Should always have exactly M by problem guarantee
        if ((int)baseline.size() < M) {
            // shouldn't happen per problem statement, but be safe
            quitf(_fail, "Test %d: could not enumerate %d baseline permutations", t + 1, M);
        }

        long long Base = cycleObj(baseline);

        // Read participant output: M tokens for this test case
        vector<string> codes(M);
        for (int k = 0; k < M; k++) {
            if (ouf.eof()) {
                quitf(_wa, "Test %d: expected %d codes but output ended early at code %d",
                      t + 1, M, k + 1);
            }
            codes[k] = ouf.readToken();
        }

        // Validate: length and character multiset
        bool feasible = true;
        string failReason;
        for (int k = 0; k < M && feasible; k++) {
            if ((int)codes[k].size() != n) {
                feasible = false;
                failReason = format("Test %d, code %d: wrong length %d (expected %d)",
                                    t + 1, k + 1, (int)codes[k].size(), n);
            } else {
                string sc = codes[k];
                sort(sc.begin(), sc.end());
                if (sc != sortedA) {
                    feasible = false;
                    failReason = format("Test %d, code %d: not a permutation of A='%s'",
                                        t + 1, k + 1, A.c_str());
                }
            }
        }
        // Validate distinctness
        if (feasible) {
            set<string> seen(codes.begin(), codes.end());
            if ((int)seen.size() != M) {
                feasible = false;
                failReason = format("Test %d: codes are not pairwise distinct", t + 1);
            }
        }

        long long perTestScore;
        if (!feasible) {
            // Obj = 0 per problem statement, score uses Obj=0
            long long Obj = 0;
            double ratio = (double)(1LL + Obj) / (double)(1LL + Base);
            if (ratio > 2.0) ratio = 2.0;
            if (ratio < 0.0) ratio = 0.0;
            perTestScore = (long long)(1000000.0 * ratio);
            // Per problem: infeasible => Obj=0, so score is floor(1e6*(1+0)/(1+Base))
            // (not forced to 0; the formula still applies with Obj=0)
        } else {
            long long Obj = cycleObj(codes);
            double ratio = (double)(1LL + Obj) / (double)(1LL + Base);
            if (ratio > 2.0) ratio = 2.0;
            if (ratio < 0.0) ratio = 0.0;
            perTestScore = (long long)(1000000.0 * ratio);
        }

        sumScore += perTestScore;
    }

    // Consume any remaining whitespace/newlines in ouf without failing on them.
    // We intentionally do NOT call ouf.readEof() strictly because trailing
    // newlines are normal in competitive programming output.

    // Compute mean score (floor division)
    long long meanScore = sumScore / (long long)T;

    // Normalise to [0, 1] for quitp
    double finalRatio = (double)meanScore / 2000000.0;
    if (finalRatio < 0.0) finalRatio = 0.0;
    if (finalRatio > 1.0) finalRatio = 1.0;

    quitp(finalRatio, "Ratio: %.6f (mean per-test score %lld / 2000000)",
          finalRatio, meanScore);

    return 0;
}