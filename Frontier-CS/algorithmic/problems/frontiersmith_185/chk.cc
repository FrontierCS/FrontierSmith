#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

bool isBalanced(const string &s, int l, int r) {
    // 1-indexed, inclusive
    int len = r - l + 1;
    if (len % 2 != 0) return false;
    int balance = 0;
    for (int i = l - 1; i < r; i++) {
        if (s[i] == '(') balance++;
        else balance--;
        if (balance < 0) return false;
    }
    return balance == 0;
}

long long computeReward(const string &s, int m,
                        const vector<int> &L,
                        const vector<int> &R,
                        const vector<long long> &W) {
    long long reward = 0;
    for (int j = 0; j < m; j++) {
        if (isBalanced(s, L[j], R[j])) {
            reward += W[j];
        }
    }
    return reward;
}

int main(int argc, char *argv[]) {
    registerTestlibCmd(argc, argv);

    // Read problem input from inf
    int n = inf.readInt();
    int m = inf.readInt();
    string T = inf.readToken();

    if ((int)T.size() != n)
        quitf(_fail, "Judge error: template length %d != n=%d", (int)T.size(), n);

    vector<int> L(m), R(m);
    vector<long long> W(m);
    long long U = 0;
    for (int j = 0; j < m; j++) {
        L[j] = inf.readInt();
        R[j] = inf.readInt();
        W[j] = inf.readLong();
        U += W[j];
    }

    // Read participant output
    string S = ouf.readToken();

    // Assert no trailing non-whitespace content
    ouf.readEof();

    // Feasibility check: length
    if ((int)S.size() != n)
        quitf(_wa, "Output length %d does not match n=%d", (int)S.size(), n);

    // Feasibility check: characters and template constraints
    for (int i = 0; i < n; i++) {
        if (S[i] != '(' && S[i] != ')')
            quitf(_wa, "Invalid character '%c' at position %d (1-indexed)", S[i], i + 1);
        if (T[i] == '(' && S[i] != '(')
            quitf(_wa, "Position %d must be '(' (fixed in template) but got '%c'", i + 1, S[i]);
        if (T[i] == ')' && S[i] != ')')
            quitf(_wa, "Position %d must be ')' (fixed in template) but got '%c'", i + 1, S[i]);
    }

    // Build baseline B: replace 1st '?' with '(', 2nd with ')', alternating globally
    string B = T;
    {
        int qcount = 0;
        for (int i = 0; i < n; i++) {
            if (B[i] == '?') {
                B[i] = (qcount % 2 == 0) ? '(' : ')';
                qcount++;
            }
        }
    }

    // Compute rewards
    long long R_val = computeReward(S, m, L, R, W);
    long long R0    = computeReward(B, m, L, R, W);

    // Compute per-test integer score in [0, 1,000,000] exactly per the statement:
    //   if U == R0: score = 1,000,000 if R == U, else 0
    //   else: score = floor(1,000,000 * max(0, R - R0) / (U - R0))
    long long int_score;
    if (U == R0) {
        int_score = (R_val == U) ? 1000000LL : 0LL;
    } else {
        long long num = R_val - R0;
        if (num < 0LL) num = 0LL;
        long long den = U - R0; // > 0 here
        // floor(1,000,000 * num / den)
        int_score = (1000000LL * num) / den;
        if (int_score < 0LL) int_score = 0LL;
        if (int_score > 1000000LL) int_score = 1000000LL;
    }

    // Convert integer score to [0,1] ratio for quitp
    double ratio = (double)int_score / 1000000.0;

    // The message MUST contain the tag "Ratio: <value>" for the judge to parse
    quitp(ratio,
          "Ratio: %.9f | score=%lld R=%lld R0=%lld U=%lld",
          ratio, int_score, R_val, R0, U);

    return 0;
}