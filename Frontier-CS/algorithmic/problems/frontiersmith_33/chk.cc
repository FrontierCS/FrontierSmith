#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Returns true if three card strings form a SET
static bool isSetTriple(const string &a, const string &b, const string &c) {
    int len = (int)a.size();
    for (int x = 0; x < len; x++) {
        int ax = a[x] - '0', bx = b[x] - '0', cx = c[x] - '0';
        bool allEqual    = (ax == bx && bx == cx);
        bool allDistinct = (ax != bx && bx != cx && ax != cx);
        if (!allEqual && !allDistinct) return false;
    }
    return true;
}

// Returns true if adding card string `card` to room would create a SET
static bool wouldCreateSet(const vector<string> &room, const string &card) {
    int sz = (int)room.size();
    for (int i = 0; i < sz; i++)
        for (int j = i + 1; j < sz; j++)
            if (isSetTriple(room[i], room[j], card)) return true;
    return false;
}

int main(int argc, char *argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input ----
    int n = inf.readInt();
    int k = inf.readInt();
    int h = inf.readInt();

    vector<int> cap(h + 1);
    for (int j = 1; j <= h; j++)
        cap[j] = inf.readInt();

    vector<long long> w(n + 1);
    vector<string>    s(n + 1);
    for (int i = 1; i <= n; i++) {
        w[i] = (long long)inf.readInt();
        s[i] = inf.readWord();
        if ((int)s[i].size() != k)
            quitf(_fail, "Input card %d has wrong length %d (expected %d)",
                  i, (int)s[i].size(), k);
    }

    // ---- Read participant output ----
    vector<int> a(n + 1, 0);
    for (int i = 1; i <= n; i++) {
        if (ouf.eof())
            quitf(_wa, "Unexpected end of output: expected %d integers, got %d", n, i - 1);
        a[i] = ouf.readInt();
    }
    // Ensure no trailing garbage
    if (!ouf.seekEof())
        quitf(_wa, "Extra tokens in output after reading %d assignments", n);

    // ---- Feasibility Check 1: a_i in [0, h] ----
    for (int i = 1; i <= n; i++) {
        if (a[i] < 0 || a[i] > h)
            quitf(_wa, "Card %d has invalid room assignment %d (must be in [0,%d])",
                  i, a[i], h);
    }

    // ---- Feasibility Check 2: room capacities ----
    vector<int> roomCount(h + 1, 0);
    for (int i = 1; i <= n; i++) {
        if (a[i] != 0) {
            roomCount[a[i]]++;
            if (roomCount[a[i]] > cap[a[i]])
                quitf(_wa, "Room %d exceeds capacity %d", a[i], cap[a[i]]);
        }
    }

    // ---- Feasibility Check 3: no SET in any room ----
    vector<vector<string>> rooms(h + 1);
    for (int i = 1; i <= n; i++) {
        if (a[i] != 0) {
            int rj = a[i];
            if (wouldCreateSet(rooms[rj], s[i]))
                quitf(_wa, "Room %d contains a SET (card %d = \"%s\" completes one)",
                      rj, i, s[i].c_str());
            rooms[rj].push_back(s[i]);
        }
    }

    // ---- Compute V ----
    long long V = 0;
    for (int i = 1; i <= n; i++)
        if (a[i] != 0) V += w[i];

    // ---- Compute U ----
    long long totalCap = 0;
    for (int j = 1; j <= h; j++) totalCap += (long long)cap[j];
    long long C = min((long long)n, totalCap);

    vector<long long> sortedW;
    sortedW.reserve(n);
    for (int i = 1; i <= n; i++) sortedW.push_back(w[i]);
    sort(sortedW.rbegin(), sortedW.rend());

    long long U = 0;
    for (long long i = 0; i < C; i++) U += sortedW[(int)i];

    // ---- Compute G (greedy baseline) ----
    // Sort by decreasing w, ties by smaller input index
    vector<int> order(n);
    iota(order.begin(), order.end(), 1);
    sort(order.begin(), order.end(), [&](int x, int y) {
        if (w[x] != w[y]) return w[x] > w[y];
        return x < y;
    });

    vector<vector<string>> greedyRoom(h + 1);
    vector<int>            greedyCount(h + 1, 0);
    long long G = 0;

    for (int idx : order) {
        for (int j = 1; j <= h; j++) {
            if (greedyCount[j] >= cap[j]) continue;
            if (wouldCreateSet(greedyRoom[j], s[idx])) continue;
            greedyRoom[j].push_back(s[idx]);
            greedyCount[j]++;
            G += w[idx];
            break;
        }
    }

    // ---- Edge case: U == 0 means nothing at all can be scored ----
    if (U == 0) {
        quitp(1.0, "V=%lld G=%lld U=%lld score=100.0000 Ratio: 1.000000", V, G, U);
    }

    // ---- Compute score per problem statement ----
    double score;
    if (U == G) {
        // Upper bound equals greedy baseline → linear from 0..U
        score = 100.0 * (double)V / (double)U;
    } else if (G == 0) {
        // Greedy got nothing → linear from 0..U
        score = 100.0 * (double)V / (double)U;
    } else if (V <= G) {
        // Below or equal to greedy baseline → score in [0,50]
        score = 50.0 * (double)V / (double)G;
    } else {
        // Above greedy baseline → score in (50,100]
        score = 50.0 + 50.0 * (double)(V - G) / (double)(U - G);
    }

    // Clamp to [0, 100]
    if (score < 0.0)   score = 0.0;
    if (score > 100.0) score = 100.0;

    double ratio = score / 100.0;
    // Clamp ratio to [0,1]
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
          "V=%lld G=%lld U=%lld score=%.6f Ratio: %.6f",
          V, G, U, score, ratio);

    return 0;
}