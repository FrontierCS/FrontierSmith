#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Check if pattern p matches string s exactly (DP)
bool matchExact(const string& p, const string& s) {
    int m = (int)p.size(), n = (int)s.size();
    // dp[i][j]: p[0..i-1] matches s[0..j-1]
    vector<vector<bool>> dp(m + 1, vector<bool>(n + 1, false));
    dp[0][0] = true;
    for (int i = 1; i <= m; i++) {
        if (p[i-1] == '*') dp[i][0] = dp[i-1][0];
    }
    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (p[i-1] == '*') {
                dp[i][j] = dp[i-1][j] || dp[i][j-1];
            } else if (p[i-1] == '-') {
                dp[i][j] = dp[i-1][j-1];
            } else {
                dp[i][j] = dp[i-1][j-1] && (p[i-1] == s[j-1]);
            }
        }
    }
    return dp[m][n];
}

// Check if pattern p matches any contiguous substring of x (including empty)
// Uses O(|p| * (|x|+1)) DP:
//   dp[i][j] = true means p[0..i-1] matches some substring of x that ends at position j
//   We allow the match to start at any position by treating dp[0][j] = true for all j.
bool isCovered(const string& p, const string& x) {
    int m = (int)p.size(), n = (int)x.size();
    // dp[i][j]: p[0..i-1] matches some substring x[start..j-1] for some start<=j
    // Boundary: dp[0][j] = true for all j (empty pattern prefix matches empty substring ending at j)
    vector<vector<bool>> dp(m + 1, vector<bool>(n + 1, false));
    for (int j = 0; j <= n; j++) dp[0][j] = true;

    for (int i = 1; i <= m; i++) {
        if (p[i-1] == '*') {
            // '*' can match empty: dp[i][j] = dp[i-1][j]
            // '*' can extend by one char: dp[i][j] = dp[i][j-1] (if j>=1)
            for (int j = 0; j <= n; j++) {
                dp[i][j] = dp[i-1][j];
                if (j >= 1) dp[i][j] = dp[i][j] || dp[i][j-1];
            }
        } else if (p[i-1] == '-') {
            for (int j = 1; j <= n; j++) {
                dp[i][j] = dp[i-1][j-1]; // match exactly one lowercase (x[j-1])
            }
        } else {
            char c = p[i-1];
            for (int j = 1; j <= n; j++) {
                dp[i][j] = dp[i-1][j-1] && (x[j-1] == c);
            }
        }
    }

    // Check if dp[m][j] is true for any j
    for (int j = 0; j <= n; j++) {
        if (dp[m][j]) return true;
    }
    return false;
}

// Compute canonical witness for pattern p: replace '-' with 'a', remove '*'
string canonicalWitness(const string& p) {
    string res;
    for (char c : p) {
        if (c == '*') continue;
        else if (c == '-') res += 'a';
        else res += c;
    }
    return res;
}

// Compute Value(x) = sum(w_i * covered_i(x)) - P * |x|
long long computeValue(const string& x, const vector<string>& patterns,
                       const vector<long long>& weights, long long P) {
    long long val = -(long long)P * (long long)x.size();
    for (int i = 0; i < (int)patterns.size(); i++) {
        if (isCovered(patterns[i], x)) {
            val += weights[i];
        }
    }
    return val;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read input
    int N = inf.readInt();
    long long L = inf.readLong();
    long long P = inf.readLong();
    inf.readEoln();

    vector<string> patterns(N);
    vector<long long> weights(N);
    for (int i = 0; i < N; i++) {
        patterns[i] = inf.readToken();
        weights[i] = inf.readLong();
        inf.readEoln();
    }

    // Compute U = sum(w_i)
    long long U = 0;
    for (int i = 0; i < N; i++) U += weights[i];

    // Compute baseline B
    // Candidate 1: empty string
    long long B = 0LL; // Value("") = 0 (no cost, but patterns with '*' might match empty...)
    {
        // Actually compute Value("") properly
        string empty_str = "";
        B = computeValue(empty_str, patterns, weights, P);
    }

    // Candidate 2: canonical witnesses
    for (int i = 0; i < N; i++) {
        string cw = canonicalWitness(patterns[i]);
        if ((long long)cw.size() <= L) {
            long long val = computeValue(cw, patterns, weights, P);
            B = max(B, val);
        }
    }

    // Read participant output
    long long len = ouf.readLong();
    string x;
    if (len == 0) {
        // second line must be empty
        x = "";
        // try to read the second line (could be empty line or EOF)
        // We use readLine which returns empty string for empty line
        string line2 = ouf.readLine();
        if (!line2.empty()) {
            quitf(_wa, "len=0 but second line is not empty: got '%s'", line2.c_str());
        }
    } else {
        x = ouf.readToken();
        ouf.readEoln();
    }

    // Check EOF
    if (!ouf.seekEof()) {
        quitf(_wa, "Unexpected trailing output after the two required lines.");
    }

    // Validate feasibility
    if (len < 0 || len > L) {
        quitf(_wa, "Infeasible: len=%lld is not in [0, %lld].", len, L);
    }
    if ((long long)x.size() != len) {
        quitf(_wa, "Infeasible: declared len=%lld but string has length %d.", len, (int)x.size());
    }
    for (char c : x) {
        if (c < 'a' || c > 'z') {
            quitf(_wa, "Infeasible: string contains non-lowercase character '%c'.", c);
        }
    }

    // Compute V = Value(x)
    long long V = computeValue(x, patterns, weights, P);

    // Compute score ratio
    double score_ratio;
    if (U == B) {
        // Special case
        score_ratio = (V == U) ? 1.0 : 0.0;
    } else {
        double num = (double)(V - B);
        double den = (double)(U - B);
        score_ratio = num / den;
        if (score_ratio < 0.0) score_ratio = 0.0;
        if (score_ratio > 1.0) score_ratio = 1.0;
    }

    quitp(score_ratio,
          "V=%lld, B=%lld, U=%lld, len=%lld. Ratio: %.6f",
          V, B, U, len, score_ratio);

    return 0;
}