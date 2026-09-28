#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static bool isAllLowercaseStr(const string &s) {
    for (char c : s)
        if (c < 'a' || c > 'z') return false;
    return true;
}

int main(int argc, char *argv[]) {
    registerTestlibCmd(argc, argv);

    // ── read problem input ──────────────────────────────────────────────────
    int N = inf.readInt();
    int K = inf.readInt();
    int L = inf.readInt();
    int R = inf.readInt();
    inf.readLine(); // consume rest of first line

    vector<long long> w(N);
    vector<array<long long, 26>> freq(N);

    for (int i = 0; i < N; i++) {
        string wline = inf.readLine();
        w[i] = (long long)stoll(wline);

        string s = inf.readLine();
        freq[i].fill(0);
        for (char c : s)
            if (c >= 'a' && c <= 'z')
                freq[i][c - 'a']++;
    }

    // ── compute baseline B ──────────────────────────────────────────────────
    // 26 strips "a".."z"; each poster uses up to R strips with largest positive freq
    long long B = 0;
    for (int i = 0; i < N; i++) {
        vector<pair<long long, int>> cands;
        for (int c = 0; c < 26; c++)
            if (freq[i][c] > 0)
                cands.push_back({freq[i][c], c});

        // sort descending by frequency, ascending by letter on tie
        sort(cands.begin(), cands.end(), [](const pair<long long, int> &a,
                                            const pair<long long, int> &b) {
            if (a.first != b.first) return a.first > b.first;
            return a.second < b.second;
        });

        array<long long, 26> g;
        g.fill(0);
        int use = min((int)cands.size(), R);
        for (int j = 0; j < use; j++)
            g[cands[j].second] = cands[j].first;

        long long err = 0;
        for (int c = 0; c < 26; c++)
            err += llabs(freq[i][c] - g[c]);
        B += w[i] * err;
    }

    // ── read and validate participant output ────────────────────────────────
    int M = ouf.readInt();
    if (M < 0 || M > K)
        quitf(_wa, "M=%d out of range [0,%d]", M, K);

    vector<array<long long, 26>> stripFreq(M);
    for (int t = 0; t < M; t++) {
        string strip = ouf.readToken();
        if ((int)strip.size() < 1 || (int)strip.size() > L)
            quitf(_wa, "Strip %d has length %d, must be in [1,%d]",
                  t + 1, (int)strip.size(), L);
        if (!isAllLowercaseStr(strip))
            quitf(_wa, "Strip %d contains non-lowercase characters", t + 1);
        stripFreq[t].fill(0);
        for (char c : strip)
            stripFreq[t][c - 'a']++;
    }

    long long O = 0;
    for (int i = 0; i < N; i++) {
        int d = ouf.readInt();
        if (d < 0 || d > R)
            quitf(_wa, "Poster %d: d=%d out of range [0,%d]", i + 1, d, R);

        array<long long, 26> g;
        g.fill(0);
        set<int> usedIds;
        for (int j = 0; j < d; j++) {
            int id = ouf.readInt();
            long long mult = ouf.readLong();
            if (id < 1 || id > M)
                quitf(_wa, "Poster %d: strip id %d out of range [1,%d]",
                      i + 1, id, M);
            if (usedIds.count(id))
                quitf(_wa, "Poster %d: duplicate strip id %d", i + 1, id);
            usedIds.insert(id);
            if (mult < 1)
                quitf(_wa, "Poster %d: multiplicity %lld < 1", i + 1, mult);
            for (int c = 0; c < 26; c++)
                g[c] += mult * stripFreq[id - 1][c];
        }

        long long err = 0;
        for (int c = 0; c < 26; c++)
            err += llabs(freq[i][c] - g[c]);
        O += w[i] * err;
    }

    // ensure no trailing tokens
    if (!ouf.seekEof())
        quitf(_wa, "Extra data after all poster assignments");

    // ── compute score ─────────────────────────────────────────────────────
    // Per statement: raw_score = 10^6 * min(5, (B+1)/(O+1))
    // Normalized ratio for quitp (must be in [0,1]):
    //   ratio = min(5, (B+1)/(O+1)) / 5
    // So ratio=1.0 when the solution is >=5x better than baseline,
    //    ratio=0.2 when solution matches baseline exactly (O==B),
    //    ratio<0.2 when solution is worse than baseline.
    double raw_ratio = (double)(B + 1) / (double)(O + 1);
    double clamped   = min(5.0, raw_ratio);   // in (0, 5]
    // Normalize to [0,1] for quitp
    double score_ratio = clamped / 5.0;
    if (score_ratio < 0.0) score_ratio = 0.0;
    if (score_ratio > 1.0) score_ratio = 1.0;

    // raw_score_1e6 = 10^6 * clamped  (human-readable; not used by judge)
    double raw_score_1e6 = 1000000.0 * clamped;

    quitp(score_ratio,
          "B=%lld O=%lld raw_ratio=%.6f raw_score=%.0f Ratio: %.9f",
          (long long)B, (long long)O, raw_ratio, raw_score_1e6, score_ratio);
}