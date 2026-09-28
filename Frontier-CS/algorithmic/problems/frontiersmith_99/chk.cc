#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// KMP: find all starting positions of pattern p in text t
static vector<int> kmpSearch(const string &t, const string &p) {
    vector<int> res;
    int n = (int)t.size(), m = (int)p.size();
    if (m == 0 || m > n) return res;
    vector<int> fail(m, 0);
    for (int i = 1; i < m; i++) {
        int j = fail[i-1];
        while (j > 0 && p[i] != p[j]) j = fail[j-1];
        if (p[i] == p[j]) j++;
        fail[i] = j;
    }
    int j = 0;
    for (int i = 0; i < n; i++) {
        while (j > 0 && t[i] != p[j]) j = fail[j-1];
        if (t[i] == p[j]) j++;
        if (j == m) {
            res.push_back(i - m + 1);
            j = fail[j-1];
        }
    }
    return res;
}

static bool isLowerAlphaStr(const string &s) {
    for (char c : s)
        if (c < 'a' || c > 'z') return false;
    return true;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt(1, 200, "T");

    // We accumulate per-case scores (each in [0, 2]) and average at the end
    // to produce a ratio in [0, 1].
    // Per-case score = 10^6 * min(2, (C_base+1)/(C(D)+1))
    // We track: sum of min(2, (C_base+1)/(C(D)+1))
    // Final ratio = totalScoreSum / (2.0 * T)
    double totalScoreSum = 0.0;

    for (int tc = 0; tc < T; tc++) {
        // Read test case parameters from input file
        int M   = inf.readInt(1, 200000, "M");
        int K   = inf.readInt(1, 80,     "K");
        int B   = inf.readInt(1, 20,     "B");

        vector<string> inscriptions(M);
        long long totalLen = 0;
        for (int i = 0; i < M; i++) {
            inscriptions[i] = inf.readToken();
            totalLen += (long long)inscriptions[i].size();
        }

        // Baseline cost: number of non-'a' characters
        long long C_base = 0;
        for (int i = 0; i < M; i++)
            for (char c : inscriptions[i])
                if (c != 'a') C_base++;

        // ---- Read participant output for this test case ----
        int D = ouf.readInt(-2000000, 2000000, "D");

        // Feasibility: D must be in [0, K]
        if (D < 0 || D > K) {
            // infeasible: 0 contribution to score sum; keep reading to stay in sync
            // We can't easily skip stencil lines if D is bogus, so hard fail.
            quitf(_wa, "Test case %d: D=%d is out of range [0,%d]", tc+1, D, K);
        }

        // Read the D stencils
        vector<string> stencils(D);
        bool feasible = true;
        string feasReason;

        set<string> seen;
        long long sumStencilLens = 0;

        for (int i = 0; i < D; i++) {
            stencils[i] = ouf.readToken();
            sumStencilLens += (long long)stencils[i].size();

            // Check lowercase only
            if (!isLowerAlphaStr(stencils[i])) {
                feasible = false;
                feasReason = "stencil contains non-lowercase character";
                break;
            }
            // Check not equal to "a"
            if (stencils[i] == "a") {
                feasible = false;
                feasReason = "stencil is exactly 'a'";
                break;
            }
            // Check nonempty (readToken already guarantees non-empty)
            // Check distinctness
            if (seen.count(stencils[i])) {
                feasible = false;
                feasReason = "duplicate stencil '" + stencils[i] + "'";
                break;
            }
            seen.insert(stencils[i]);
        }

        if (feasible) {
            // Check sum of stencil lengths <= total inscription length
            if (sumStencilLens > totalLen) {
                feasible = false;
                feasReason = "sum of stencil lengths exceeds total inscription length";
            }
        }

        if (feasible) {
            // Check each stencil occurs as a contiguous substring of at least one inscription
            for (int i = 0; i < D && feasible; i++) {
                bool found = false;
                for (int j = 0; j < M && !found; j++) {
                    if (inscriptions[j].find(stencils[i]) != string::npos)
                        found = true;
                }
                if (!found) {
                    feasible = false;
                    feasReason = "stencil '" + stencils[i] + "' does not appear in any inscription";
                }
            }
        }

        if (!feasible) {
            // Infeasible: score = 0 for this test case
            totalScoreSum += 0.0;
            continue;
        }

        // ---- Compute total cost C(D) ----

        // Build cost
        long long buildCost = 0;
        for (int i = 0; i < D; i++)
            buildCost += (long long)B + (long long)stencils[i].size();

        // Encoding cost: for each inscription compute min-cost via DP
        // Precompute for each inscription which stencils match at which positions
        long long encodingCost = 0;

        for (int si = 0; si < M; si++) {
            const string &s = inscriptions[si];
            int n = (int)s.size();

            // matches[i] = list of end positions (exclusive) reachable from i via a stencil
            vector<vector<int>> matches(n);
            for (int di = 0; di < D; di++) {
                const string &p = stencils[di];
                int plen = (int)p.size();
                vector<int> starts = kmpSearch(s, p);
                for (int st2 : starts) {
                    int endPos = st2 + plen;
                    if (endPos <= n)
                        matches[st2].push_back(endPos);
                }
            }

            const long long INF_COST = (long long)1e18;
            vector<long long> dp(n+1, INF_COST);
            dp[0] = 0;

            for (int i = 0; i < n; i++) {
                if (dp[i] == INF_COST) continue;
                // Option 1: free-a or literal
                long long charCost = (s[i] == 'a') ? 0LL : 1LL;
                if (dp[i] + charCost < dp[i+1])
                    dp[i+1] = dp[i] + charCost;
                // Option 2: stencil
                for (int endPos : matches[i]) {
                    if (dp[i] + 1LL < dp[endPos])
                        dp[endPos] = dp[i] + 1LL;
                }
            }

            encodingCost += dp[n];
        }

        long long totalCost = buildCost + encodingCost;

        // Per-case score = 10^6 * min(2, (C_base+1) / (C(D)+1))
        // We track min(2, ...) to sum later
        double perCaseScore = min(2.0, (double)(C_base + 1) / (double)(totalCost + 1));
        totalScoreSum += perCaseScore;
    }

    // Strict EOF: no trailing tokens allowed
    if (!ouf.seekEof())
        quitf(_wa, "Extra output after all test cases");

    // Final ratio in [0, 1]:
    // sum(per_case_score) / (2.0 * T)
    // where per_case_score in [0, 2], so ratio in [0, 1]
    double finalRatio = (T > 0) ? (totalScoreSum / (2.0 * (double)T)) : 0.0;
    if (finalRatio > 1.0) finalRatio = 1.0;
    if (finalRatio < 0.0) finalRatio = 0.0;

    quitp(finalRatio, "Ratio: %.9f (scoreSum=%.6f over %d cases)", finalRatio, totalScoreSum, T);

    return 0;
}