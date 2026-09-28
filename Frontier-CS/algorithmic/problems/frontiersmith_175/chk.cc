#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Trim leading zeros, keep at least one digit
string trimLeadingZeros(const string& s) {
    size_t i = 0;
    while (i + 1 < s.size() && s[i] == '0') i++;
    return s.substr(i);
}

// BFS over all display modes reachable from 'original'
set<string> getReachable(const string& original) {
    set<string> visited;
    queue<string> q;
    visited.insert(original);
    q.push(original);
    while (!q.empty()) {
        string cur = q.front(); q.pop();
        if (cur.size() <= 1) continue;

        // Left rotation
        {
            string nxt = cur.substr(1) + cur[0];
            nxt = trimLeadingZeros(nxt);
            if (!visited.count(nxt)) {
                visited.insert(nxt);
                q.push(nxt);
            }
        }
        // Right rotation
        {
            string nxt = string(1, cur.back()) + cur.substr(0, cur.size() - 1);
            nxt = trimLeadingZeros(nxt);
            if (!visited.count(nxt)) {
                visited.insert(nxt);
                q.push(nxt);
            }
        }
    }
    return visited;
}

// overlap(A,B): largest k so suffix of A of length k == prefix of B of length k
int computeOverlap(const string& A, const string& B) {
    int maxK = (int)min(A.size(), B.size());
    for (int k = maxK; k >= 1; k--) {
        if (A.compare(A.size() - k, k, B, 0, k) == 0) return k;
    }
    return 0;
}

long long bannerLength(const vector<string>& modes) {
    if (modes.empty()) return 0LL;
    long long len = (long long)modes[0].size();
    for (int i = 1; i < (int)modes.size(); i++) {
        len += (long long)modes[i].size() - computeOverlap(modes[i - 1], modes[i]);
    }
    return len;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    // Per-test-case score in [0, 1]:
    //   perTest = min(10, B/C) / 10
    //   baseline (C == B): perTest = 1/10 = 0.1
    //   10x improvement:   perTest = 10/10 = 1.0  (capped)
    // Final ratio = arithmetic mean of perTest scores over T test cases.
    // This maps the stated formula score=1,000,000*min(10,B/C) to the [0,1]
    // range by dividing by 10,000,000 (the maximum per-test score).
    double sumScore = 0.0;

    for (int t = 0; t < T; t++) {
        int n = inf.readInt();

        // Read original codes from input
        vector<string> originals(n);
        for (int i = 0; i < n; i++) {
            originals[i] = inf.readToken();
        }

        // Precompute reachable modes for each original
        vector<set<string>> reachable(n);
        for (int i = 0; i < n; i++) {
            reachable[i] = getReachable(originals[i]);
        }

        // Compute baseline banner length (original order, original strings)
        long long B = bannerLength(originals);

        // Read participant output: n lines of "index chosen_mode"
        set<int> usedIndices;
        vector<string> chosenModes(n);

        for (int i = 0; i < n; i++) {
            int idx = ouf.readInt(1, n,
                ("Test case " + to_string(t + 1) + ": index out of range at line " + to_string(i + 1)).c_str());

            if (usedIndices.count(idx)) {
                quitf(_wa, "Test case %d: index %d appears more than once", t + 1, idx);
            }
            usedIndices.insert(idx);

            string mode = ouf.readToken();

            // Validate that mode is reachable from originals[idx-1]
            if (!reachable[idx - 1].count(mode)) {
                quitf(_wa,
                      "Test case %d: '%s' is not a valid display mode for original '%s' (index %d)",
                      t + 1, mode.c_str(), originals[idx - 1].c_str(), idx);
            }

            chosenModes[i] = mode;
        }

        // Ensure all indices 1..n appeared exactly once
        for (int i = 1; i <= n; i++) {
            if (!usedIndices.count(i)) {
                quitf(_wa, "Test case %d: index %d missing from output", t + 1, i);
            }
        }

        long long C = bannerLength(chosenModes);

        // Guard against degenerate zero lengths
        if (C <= 0) C = 1;
        if (B <= 0) B = 1;

        // Per-test score: min(10, B/C) / 10, in [0, 1]
        // Baseline (C==B): ratio=1.0, perTest=0.1
        // 10x better: ratio=10.0, perTest=1.0
        double ratio = (double)B / (double)C;
        if (ratio > 10.0) ratio = 10.0;
        double perTestScore = ratio / 10.0; // in [0, 1]
        sumScore += perTestScore;
    }

    // After consuming all expected output, reject any trailing garbage
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after all test cases");
    }

    // Final score ratio = arithmetic mean of per-test scores, in [0, 1]
    // baseline: each test gets 0.1, mean = 0.1
    // best possible: each test gets 1.0, mean = 1.0
    double scoreRatio = sumScore / (double)T;
    // Clamp to [0, 1] for safety
    if (scoreRatio < 0.0) scoreRatio = 0.0;
    if (scoreRatio > 1.0) scoreRatio = 1.0;

    quitp(scoreRatio,
          "Ratio: %.9f (sum_score=%.6f over %d test cases)",
          scoreRatio, sumScore, T);
}