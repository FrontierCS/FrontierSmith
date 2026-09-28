#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static bool allLower(const string& s) {
    for (char c : s) if (c < 'a' || c > 'z') return false;
    return true;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    // Per-test scores: S_i = 10^6 * min(2, B_i / C_i), in [0, 2e6].
    // We store sum of (S_i / 1e6) = sum of min(2, B_i/C_i) in [0, 2*T],
    // then the final quitp ratio = sum / (2*T), mapping [0, 2e6 avg] -> [0, 1].
    double sum_score = 0.0;

    for (int tc = 0; tc < T; tc++) {
        int q = inf.readInt();
        vector<string> targets(q);
        int max_len = 0;
        for (int i = 0; i < q; i++) {
            targets[i] = inf.readToken();
            max_len = max(max_len, (int)targets[i].size());
        }

        // Compute baseline B
        double B = 0.0;
        for (int i = 0; i < q; i++) {
            int n = (int)targets[i].size();
            int k = (n + 7) / 8;
            B += (double)(n + 2 * k - 1);
        }

        // ---- Read participant output ----
        bool feasible = true;
        string fail_reason;

        // Read m
        long long m_val = 0;
        int m = 0;
        {
            string tok = ouf.readToken();
            bool ok = !tok.empty();
            for (char c : tok) if (!isdigit((unsigned char)c)) { ok = false; break; }
            if (!ok) {
                feasible = false;
                fail_reason = "expected integer m for test case " + to_string(tc+1) + ", got '" + tok + "'";
                quitf(_wa, "%s", fail_reason.c_str());
            }
            m_val = 0;
            for (char c : tok) m_val = m_val * 10 + (c - '0');
            if (m_val < 1 || m_val > 5000) {
                feasible = false;
                fail_reason = "m=" + to_string(m_val) + " out of range [1,5000] for test case " + to_string(tc+1);
            }
            m = (int)m_val;
        }

        // Read m commands
        vector<string> values; // 1-indexed: values[0] unused, values[i] = value of var i
        values.push_back(""); // placeholder index 0
        long long cost = 0;

        if (feasible) {
            cost += m; // each command costs 1
            for (int ci = 0; ci < m && feasible; ci++) {
                string cmd = ouf.readToken();
                if (cmd == "L") {
                    string s = ouf.readToken();
                    if (s.empty() || s.size() > 8 || !allLower(s)) {
                        feasible = false;
                        fail_reason = "invalid literal string '" + s + "' at command " + to_string(ci+1);
                        break;
                    }
                    if ((int)s.size() > max_len) {
                        feasible = false;
                        fail_reason = "literal '" + s + "' length " + to_string(s.size()) +
                                      " exceeds max_len " + to_string(max_len);
                        break;
                    }
                    cost += (long long)s.size();
                    values.push_back(s);
                } else if (cmd == "C") {
                    string ta = ouf.readToken();
                    string tb = ouf.readToken();
                    bool aok = !ta.empty(), bok = !tb.empty();
                    for (char c : ta) if (!isdigit((unsigned char)c)) { aok = false; break; }
                    for (char c : tb) if (!isdigit((unsigned char)c)) { bok = false; break; }
                    if (!aok || !bok) {
                        feasible = false;
                        fail_reason = "invalid C operands '" + ta + "' '" + tb + "' at command " + to_string(ci+1);
                        break;
                    }
                    long long av = 0, bv = 0;
                    for (char c : ta) av = av * 10 + (c - '0');
                    for (char c : tb) bv = bv * 10 + (c - '0');
                    int cur_idx = ci + 1; // variable being created is ci+1 (1-indexed)
                    if (av < 1 || av >= cur_idx) {
                        feasible = false;
                        fail_reason = "C operand a=" + to_string(av) + " invalid at command " + to_string(ci+1);
                        break;
                    }
                    if (bv < 1 || bv >= cur_idx) {
                        feasible = false;
                        fail_reason = "C operand b=" + to_string(bv) + " invalid at command " + to_string(ci+1);
                        break;
                    }
                    string concat = values[(int)av] + values[(int)bv];
                    if ((int)concat.size() > max_len) {
                        feasible = false;
                        fail_reason = "concatenated string length " + to_string(concat.size()) +
                                      " exceeds max_len " + to_string(max_len) + " at command " + to_string(ci+1);
                        break;
                    }
                    values.push_back(concat);
                } else {
                    feasible = false;
                    fail_reason = "unknown command '" + cmd + "' at command " + to_string(ci+1);
                    break;
                }
            }
        }

        // Read result indices
        vector<int> result(q, 0);
        if (feasible) {
            for (int i = 0; i < q && feasible; i++) {
                string tok = ouf.readToken();
                bool ok = !tok.empty();
                for (char c : tok) if (!isdigit((unsigned char)c)) { ok = false; break; }
                if (!ok) {
                    feasible = false;
                    fail_reason = "expected result index for target " + to_string(i+1) + ", got '" + tok + "'";
                    break;
                }
                long long rv = 0;
                for (char c : tok) rv = rv * 10 + (c - '0');
                if (rv < 1 || rv > (long long)m) {
                    feasible = false;
                    fail_reason = "result index r_" + to_string(i+1) + "=" + to_string(rv) +
                                  " out of range [1," + to_string(m) + "]";
                    break;
                }
                int r = (int)rv;
                if (values[r] != targets[i]) {
                    feasible = false;
                    fail_reason = "target " + to_string(i+1) + ": variable " + to_string(r) +
                                  " has value '" + values[r] + "' but expected '" + targets[i] + "'";
                    break;
                }
                result[i] = r;
            }
        }

        if (!feasible) {
            // Score 0 for this test case (adds 0 to sum_score)
            // But we must not crash or leave unread tokens from this test case
            // Since we can't reliably skip, just quit with WA
            quitf(_wa, "Infeasible solution for test case %d: %s", tc + 1, fail_reason.c_str());
        } else {
            // S_i = 10^6 * min(2, B/C), normalised to [0,2] here
            double s_norm = min(2.0, B / (double)cost);
            sum_score += s_norm;
        }
    }

    // Check no extra output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after all test case results");
    }

    // Final ratio in [0,1]:
    // mean of S_i/10^6 is sum_score/T in [0,2]; divide by 2 to get [0,1]
    double ratio = sum_score / (2.0 * (double)T);
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio, "Ratio: %.9f (sum_norm=%.6f, T=%d)", ratio, sum_score, T);
    return 0;
}