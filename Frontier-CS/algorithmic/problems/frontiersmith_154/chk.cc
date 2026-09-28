#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Returns {start_r, start_c, end_r, end_c} for a stitch
tuple<int,int,int,int> stitchEndpoints(int i, int j, char d, int o) {
    int sr, sc, er, ec;
    if (d == '\\') {
        if (o == 0) { sr = i-1; sc = j-1; er = i;   ec = j;   }
        else        { sr = i;   sc = j;   er = i-1;  ec = j-1; }
    } else { // '/'
        if (o == 0) { sr = i-1; sc = j;   er = i;   ec = j-1; }
        else        { sr = i;   sc = j-1; er = i-1;  ec = j;   }
    }
    return {sr, sc, er, ec};
}

long long dbCost(int r, int c, int N, int M) {
    return (long long)min({r, N-r, c, M-c});
}

long long manhattanDist(int r1, int c1, int r2, int c2) {
    return (long long)abs(r1-r2) + (long long)abs(c1-c2);
}

long long sessionCost(const vector<tuple<int,int,char,int>>& stitches, int N, int M) {
    int K = (int)stitches.size();
    if (K == 0) return 0LL;
    long long cost = 1000LL; // 500 start + 500 end overhead

    // first stitch
    auto [sr0, sc0, er0, ec0] = stitchEndpoints(
        get<0>(stitches[0]), get<1>(stitches[0]),
        get<2>(stitches[0]), get<3>(stitches[0]));
    cost += 1000LL * dbCost(sr0, sc0, N, M);

    // last stitch
    auto [srL, scL, erL, ecL] = stitchEndpoints(
        get<0>(stitches[K-1]), get<1>(stitches[K-1]),
        get<2>(stitches[K-1]), get<3>(stitches[K-1]));
    cost += 1000LL * dbCost(erL, ecL, N, M);

    // front-side diagonal costs
    cost += 1414LL * K;

    // transition costs
    int prev_er = er0, prev_ec = ec0;
    for (int x = 1; x < K; x++) {
        auto [sr, sc, er, ec] = stitchEndpoints(
            get<0>(stitches[x]), get<1>(stitches[x]),
            get<2>(stitches[x]), get<3>(stitches[x]));
        cost += 1000LL * manhattanDist(prev_er, prev_ec, sr, sc);
        prev_er = er; prev_ec = ec;
    }
    return cost;
}

long long baselineCost(int N, int M, const vector<string>& grid) {
    long long total = 0LL;
    for (char col = 'A'; col <= 'Z'; col++) {
        vector<tuple<int,int,char,int>> stitches;
        for (int i = 1; i <= N; i++) {
            for (int j = 1; j <= M; j++) {
                if (grid[i-1][j-1] == col) {
                    stitches.push_back({i, j, '\\', 0});
                    stitches.push_back({i, j, '/', 1});
                }
            }
        }
        if (!stitches.empty()) {
            total += sessionCost(stitches, N, M);
        }
    }
    return total;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int N = inf.readInt(1, 200, "N");
    int M = inf.readInt(1, 200, "M");
    inf.readEoln();

    vector<string> grid(N);
    for (int i = 0; i < N; i++) {
        grid[i] = inf.readToken();
        if ((int)grid[i].size() != M)
            quitf(_fail, "grid row %d has wrong length", i+1);
    }

    // stitch_done[i][j][0] = backslash done, [1] = slash done
    // 1-indexed; we'll use i in [1..N], j in [1..M]
    vector<vector<array<bool,2>>> stitch_done(N+1, vector<array<bool,2>>(M+1, {false, false}));

    int S = ouf.readInt(0, 2*26*N*M, "S");

    long long total_cost = 0LL;

    for (int s = 0; s < S; s++) {
        // Read color
        string colorStr = ouf.readToken();
        if (colorStr.size() != 1 || colorStr[0] < 'A' || colorStr[0] > 'Z')
            quitf(_wa, "session %d: invalid color '%s'", s+1, colorStr.c_str());
        char color = colorStr[0];

        int K = ouf.readInt(1, 2*N*M, "K");

        vector<tuple<int,int,char,int>> stitches;
        stitches.reserve(K);

        for (int k = 0; k < K; k++) {
            int i = ouf.readInt(1, N, "i");
            int j = ouf.readInt(1, M, "j");

            string dStr = ouf.readToken();
            if (dStr.size() != 1 || (dStr[0] != '\\' && dStr[0] != '/'))
                quitf(_wa, "session %d stitch %d: invalid diagonal '%s'", s+1, k+1, dStr.c_str());
            char d = dStr[0];

            int o = ouf.readInt(0, 1, "o");

            // Check cell color
            char cellColor = grid[i-1][j-1];
            if (cellColor == '.')
                quitf(_wa, "session %d stitch %d: cell (%d,%d) is empty", s+1, k+1, i, j);
            if (cellColor != color)
                quitf(_wa, "session %d stitch %d: cell (%d,%d) has color '%c' but session color is '%c'",
                      s+1, k+1, i, j, cellColor, color);

            int didx = (d == '\\') ? 0 : 1;
            if (stitch_done[i][j][didx])
                quitf(_wa, "session %d stitch %d: stitch (%d,%d,%c) used more than once",
                      s+1, k+1, i, j, d);
            stitch_done[i][j][didx] = true;

            stitches.push_back({i, j, d, o});
        }

        total_cost += sessionCost(stitches, N, M);
    }

    // Check all non-empty cells have both stitches
    for (int i = 1; i <= N; i++) {
        for (int j = 1; j <= M; j++) {
            char c = grid[i-1][j-1];
            if (c != '.') {
                if (!stitch_done[i][j][0])
                    quitf(_wa, "Cell (%d,%d) missing '\\' stitch", i, j);
                if (!stitch_done[i][j][1])
                    quitf(_wa, "Cell (%d,%d) missing '/' stitch", i, j);
            } else {
                if (stitch_done[i][j][0] || stitch_done[i][j][1])
                    quitf(_wa, "Empty cell (%d,%d) has a stitch", i, j);
            }
        }
    }

    // NOTE: do NOT call ouf.readEof() — trailing newlines in participant
    // output would cause false "Expected EOF" failures.

    long long B = baselineCost(N, M, grid);
    long long O = total_cost;

    if (O <= 0) {
        quitf(_wa, "Participant cost is non-positive: %lld", O);
    }

    // score = 100 * min(2, B/O), mapped to ratio in [0,1] for quitp
    // ratio_for_quitp = min(1.0, min(2.0, B/O) / 2.0)
    double ratio = (double)B / (double)O;
    double score_ratio = min(1.0, min(2.0, ratio) / 2.0);

    quitp(score_ratio,
          "Participant cost: %lld, Baseline cost: %lld, B/O=%.6f, Ratio: %.6f",
          O, B, ratio, score_ratio);

    return 0;
}