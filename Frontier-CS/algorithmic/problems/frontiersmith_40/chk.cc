#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Returns true if pattern is a subsequence of text
bool checkSubseq(const string &pattern, const string &text) {
    int pi = 0;
    for (int i = 0; i < (int)text.size() && pi < (int)pattern.size(); i++) {
        if (text[i] == pattern[pi]) pi++;
    }
    return pi == (int)pattern.size();
}

int main(int argc, char *argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    double totalScore = 0.0;
    int numCases = 0;

    for (int tc = 0; tc < T; tc++) {
        // ---- Read problem input for this test case ----
        int N = inf.readInt();
        int K = inf.readInt();
        long long P = inf.readLong();
        long long Q = inf.readLong();
        long long A = inf.readLong();
        string s = inf.readToken();

        vector<long long> w(N);
        vector<string> t(N);
        for (int i = 0; i < N; i++) {
            w[i] = inf.readLong();
            t[i] = inf.readToken();
        }

        // ---- Compute baseline cost ----
        set<char> sChars(s.begin(), s.end());
        long long d = (long long)sChars.size();
        long long cBase = d * (P + Q);
        for (int i = 0; i < N; i++) {
            bool allInS = true;
            for (char c : t[i]) {
                if (sChars.find(c) == sChars.end()) { allInS = false; break; }
            }
            if (allInS) {
                long long fulfillCost = A * (long long)t[i].size();
                cBase += min(w[i], fulfillCost);
            } else {
                cBase += w[i];
            }
        }

        // ---- Read participant output for this test case ----
        // We must always read all output tokens for this test case, even if
        // infeasible, so subsequent test cases can be parsed correctly.
        // feasible=false => caseScore=0, but we keep parsing.

        bool feasible = true;
        string infeasReason;

        int M = ouf.readInt();
        if (M < 0 || M > K) {
            feasible = false;
            infeasReason = "M=" + to_string(M) + " violates 0<=M<=" + to_string(K);
        }

        // Read M template strings (always read them so we can continue)
        // If M is negative or absurdly large we can't parse further — hard fail
        if (M < 0) {
            quitf(_wa, "Test case %d: M=%d is negative, cannot continue parsing", tc+1, M);
        }
        if (M > 100000) {
            // Protect against pathological M
            quitf(_wa, "Test case %d: M=%d is absurdly large", tc+1, M);
        }

        vector<string> templates(M);
        for (int j = 0; j < M; j++) {
            templates[j] = ouf.readToken();
            if (feasible) {
                if (templates[j].empty()) {
                    feasible = false;
                    infeasReason = "template " + to_string(j+1) + " is empty";
                } else if (!checkSubseq(templates[j], s)) {
                    feasible = false;
                    infeasReason = "template " + to_string(j+1) + " (\"" +
                                   templates[j].substr(0, 20) + "\") is not a subsequence of s";
                }
            }
        }

        // Read N order lines
        long long cSub = 0;
        // Add template creation cost
        for (int j = 0; j < M; j++) {
            cSub += P + Q * (long long)templates[j].size();
        }

        for (int i = 0; i < N; i++) {
            int firstVal = ouf.readInt();
            if (firstVal == 0) {
                // Skipped order
                cSub += w[i];
            } else {
                int r = firstVal;
                if (r < 1) {
                    // This is infeasible but still parseable: r tokens to read = 0?
                    // r<1 means invalid. Since r<=0 and r!=0 means r<0, we can't
                    // read a negative number of indices. Hard fail.
                    quitf(_wa, "Test case %d, order %d: r=%d is invalid (must be >=1 or 0 to skip)",
                          tc+1, i+1, r);
                }
                // Read r template indices
                string assembled;
                bool orderFeasible = true;
                for (int j = 0; j < r; j++) {
                    int idx = ouf.readInt();
                    if (idx < 1 || idx > M) {
                        if (feasible || orderFeasible) {
                            feasible = false;
                            orderFeasible = false;
                            infeasReason = "order " + to_string(i+1) + ": template index " +
                                           to_string(idx) + " out of range [1," + to_string(M) + "]";
                        }
                        // assembled stays partial/wrong but we keep reading
                    } else {
                        assembled += templates[idx - 1];
                    }
                }
                if (feasible && orderFeasible) {
                    if (assembled != t[i]) {
                        feasible = false;
                        infeasReason = "order " + to_string(i+1) + ": assembled string does not match target";
                    } else {
                        cSub += A * (long long)r;
                    }
                } else if (!orderFeasible) {
                    // feasible already false; cSub contribution from this order: don't add
                    // (doesn't matter since whole case scores 0)
                } else {
                    // feasible was already false before this order
                    // still add assembly cost for score calculation purposes?
                    // Doesn't matter since whole case is infeasible => score 0
                    cSub += A * (long long)r;
                }
            }
        }

        // ---- Score this test case ----
        double caseScore;
        if (!feasible) {
            caseScore = 0.0;
            // Optionally log infeasibility but do NOT quit
        } else if (cSub <= 0) {
            // cSub==0 means free solution: ratio = cBase/0 would be infinity, cap at 2
            caseScore = 2.0e6;
        } else {
            double ratio = (double)cBase / (double)cSub;
            if (ratio > 2.0) ratio = 2.0;
            if (ratio < 0.0) ratio = 0.0;
            caseScore = 1.0e6 * ratio;
        }

        totalScore += caseScore;
        numCases++;
    }

    // Check no trailing garbage in participant output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra data in participant output after all test cases");
    }

    // Overall score = arithmetic mean of score_case over all test cases
    // Each case score is in [0, 2e6]; map mean to [0,1] by dividing by 2e6
    double meanScore = (numCases > 0) ? (totalScore / (double)numCases) : 0.0;
    double finalRatio = meanScore / 2.0e6;
    if (finalRatio > 1.0) finalRatio = 1.0;
    if (finalRatio < 0.0) finalRatio = 0.0;

    quitp(finalRatio, "Ratio: %.9f (mean score per case: %.2f / 2000000)", finalRatio, meanScore);
}