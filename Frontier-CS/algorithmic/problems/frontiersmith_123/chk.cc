#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read input ----
    int n = inf.readInt();
    int m = inf.readInt();
    string s = inf.readToken();

    struct Request {
        int l, r;       // 0-based
        long long w;
        string t;
    };
    vector<Request> reqs(m);
    for (int j = 0; j < m; j++) {
        reqs[j].l = inf.readInt() - 1;
        reqs[j].r = inf.readInt() - 1;
        reqs[j].w = (long long)inf.readLong();
        reqs[j].t = inf.readToken();
    }

    // ---- Read participant output ----
    if (ouf.seekEof()) {
        quitf(_wa, "Output is empty, expected a string of length %d", n);
    }
    string u = ouf.readToken();

    // Check there is no extra non-whitespace content after the answer.
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra content found in output after the answer string");
    }

    // ---- Feasibility checks ----
    if ((int)u.size() != n) {
        quitf(_wa, "Output length %d does not match expected length %d",
              (int)u.size(), n);
    }
    for (int i = 0; i < n; i++) {
        if (u[i] < 'a' || u[i] > 'z') {
            quitf(_wa,
                  "Position %d contains invalid character '%c' (not a lowercase letter)",
                  i + 1, u[i]);
        }
        if (s[i] != '?' && s[i] != u[i]) {
            quitf(_wa,
                  "Position %d: fixed letter '%c' in s was changed to '%c'",
                  i + 1, s[i], u[i]);
        }
    }

    // ---- Helper: check anagram of str[al..ar] vs t ----
    auto checkAnagram = [](const string& str, int al, int ar,
                           const string& t) -> bool {
        int cnt[26] = {};
        for (int i = al; i <= ar; i++) cnt[(unsigned char)str[i] - 'a']++;
        for (char c : t)               cnt[(unsigned char)c    - 'a']--;
        for (int i = 0; i < 26; i++) if (cnt[i] != 0) return false;
        return true;
    };

    // ---- Compute baseline B (replace every '?' with 'a') ----
    string baseline = s;
    for (int i = 0; i < n; i++) {
        if (baseline[i] == '?') baseline[i] = 'a';
    }

    long long B = 0;
    for (int j = 0; j < m; j++) {
        if (!checkAnagram(baseline, reqs[j].l, reqs[j].r, reqs[j].t)) {
            B += reqs[j].w;
        }
    }

    // ---- Compute C for participant output ----
    long long C = 0;
    for (int j = 0; j < m; j++) {
        if (!checkAnagram(u, reqs[j].l, reqs[j].r, reqs[j].t)) {
            C += reqs[j].w;
        }
    }

    // ---- Compute score per the statement formula ----
    // Statement: score = 1,000,000 * max(0, (B-C)/B)  [B > 0]
    //            score = 1,000,000                     [B = 0, feasible]
    //
    // quitp(x, ...) expects x in [0, 1,000,000] (integer points out of 1,000,000).
    // The judge also parses the "Ratio:" tag which must be in [0,1].

    double ratio;   // [0,1] for the Ratio: tag
    double score;   // [0,1000000] passed to quitp

    if (B == 0) {
        // Any feasible output gets full marks when baseline is already zero.
        ratio = 1.0;
        score = 1000000.0;
        quitp(score,
              "Ratio: %.10f | B=0 so full score; C=%lld",
              ratio, C);
    }

    double raw = (double)(B - C) / (double)B;
    if (raw < 0.0) raw = 0.0;
    if (raw > 1.0) raw = 1.0;
    ratio = raw;
    score = 1000000.0 * ratio;

    quitp(score,
          "Ratio: %.10f | B=%lld C=%lld improvement=%lld score=%.6f",
          ratio, B, C, B - C, score);
}