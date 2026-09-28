#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// ---------- energy helpers ----------

long long laneEnergy(const string &lane) {
    int n = (int)lane.size();
    if (n == 0) return 0;
    long long e = 0;
    // Odd-length palindromes
    for (int c = 0; c < n; c++) {
        for (int r = 0; c - r >= 0 && c + r < n; r++) {
            if (lane[c - r] == lane[c + r]) {
                long long L = 2 * r + 1;
                e += L * L;
            } else break;
        }
    }
    // Even-length palindromes
    for (int c = 0; c < n - 1; c++) {
        for (int r = 1; c - r + 1 >= 0 && c + r < n; r++) {
            if (lane[c - r + 1] == lane[c + r]) {
                long long L = 2 * r;
                e += L * L;
            } else break;
        }
    }
    return e;
}

long long computeEnergy(int H, int W, const vector<string> &grid) {
    long long total = 0;
    // Horizontal lanes
    for (int i = 0; i < H; i++) {
        string seg;
        for (int j = 0; j <= W; j++) {
            if (j < W && grid[i][j] != '#') {
                seg += grid[i][j];
            } else {
                if (!seg.empty()) { total += laneEnergy(seg); seg.clear(); }
            }
        }
    }
    // Vertical lanes
    for (int j = 0; j < W; j++) {
        string seg;
        for (int i = 0; i <= H; i++) {
            if (i < H && grid[i][j] != '#') {
                seg += grid[i][j];
            } else {
                if (!seg.empty()) { total += laneEnergy(seg); seg.clear(); }
            }
        }
    }
    return total;
}

vector<string> buildBaseline(int H, int W,
                              const vector<string> &mask,
                              const string &s) {
    string sorted_s = s;
    sort(sorted_s.begin(), sorted_s.end());
    vector<string> grid = mask;
    int idx = 0;
    for (int i = 0; i < H; i++)
        for (int j = 0; j < W; j++)
            if (grid[i][j] == '.')
                grid[i][j] = sorted_s[idx++];
    return grid;
}

// ---------- main ----------

int main(int argc, char *argv[]) {
    registerTestlibCmd(argc, argv);

    // Read input
    int H = inf.readInt();
    int W = inf.readInt();
    vector<string> mask(H);
    for (int i = 0; i < H; i++)
        mask[i] = inf.readToken();
    string s = inf.readToken();

    // Read participant output: H tokens, each of length W
    vector<string> outgrid(H);
    string placed;

    for (int i = 0; i < H; i++) {
        // readToken skips whitespace/newlines between rows
        if (ouf.eof())
            quitf(_wa, "Unexpected EOF: expected row %d (1-indexed)", i + 1);

        outgrid[i] = ouf.readToken();

        if ((int)outgrid[i].size() != W)
            quitf(_wa,
                  "Row %d has length %d, expected %d",
                  i + 1, (int)outgrid[i].size(), W);

        for (int j = 0; j < W; j++) {
            char cm = mask[i][j];
            char co = outgrid[i][j];
            if (cm == '#') {
                if (co != '#')
                    quitf(_wa,
                          "Cell (%d,%d) must be '#' but participant wrote '%c'",
                          i + 1, j + 1, co);
            } else {
                // must be a lowercase letter
                if (co < 'a' || co > 'z')
                    quitf(_wa,
                          "Cell (%d,%d) must be a lowercase letter but participant wrote '%c'",
                          i + 1, j + 1, co);
                placed += co;
            }
        }
    }

    // Check multiset
    {
        string sp = placed, ss = s;
        sort(sp.begin(), sp.end());
        sort(ss.begin(), ss.end());
        if (sp != ss)
            quitf(_wa,
                  "Multiset of placed letters does not match s. "
                  "Placed (sorted): \"%s\", Expected (sorted): \"%s\"",
                  sp.c_str(), ss.c_str());
    }

    // Compute energies
    long long E_you  = computeEnergy(H, W, outgrid);
    vector<string> baseline = buildBaseline(H, W, mask, s);
    long long E_base = computeEnergy(H, W, baseline);

    if (E_base <= 0)
        quitf(_fail, "Baseline energy is non-positive: E_base=%lld", E_base);

    // score = 1000 * min(2, E_you / E_base)
    // map to ratio in [0,1]: ratio = min(1, E_you / (2*E_base))
    double ratio = (double)E_you / (2.0 * (double)E_base);
    if (ratio > 1.0) ratio = 1.0;
    if (ratio < 0.0) ratio = 0.0;

    quitp(ratio,
          "E_you=%lld E_base=%lld Ratio: %.6f",
          E_you, E_base, ratio);

    return 0;
}