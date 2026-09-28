#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Simulate a command sequence and return total weighted evacuation time.
// Positions are 0-indexed internally.
long long simulate(
    int H, int W,
    const vector<string>& grid,
    const vector<tuple<int,int,long long>>& robots_in,
    const set<pair<int,int>>& exits,
    int T,
    const string& cmds
) {
    int N = (int)robots_in.size();
    vector<int> rx(N), ry(N);
    vector<long long> rw(N);
    vector<bool> exited(N, false);

    for (int i = 0; i < N; i++) {
        rx[i] = get<0>(robots_in[i]);
        ry[i] = get<1>(robots_in[i]);
        rw[i] = get<2>(robots_in[i]);
    }

    long long total = 0;
    int L = (int)cmds.size();

    for (int step = 1; step <= L; step++) {
        char cmd = cmds[step - 1];
        int dr = 0, dc = 0;
        if      (cmd == 'U') dr = -1;
        else if (cmd == 'D') dr = +1;
        else if (cmd == 'L') dc = -1;
        else if (cmd == 'R') dc = +1;

        for (int i = 0; i < N; i++) {
            if (exited[i]) continue;
            int nr = rx[i] + dr;
            int nc = ry[i] + dc;
            if (nr >= 0 && nr < H && nc >= 0 && nc < W && grid[nr][nc] != '#') {
                rx[i] = nr;
                ry[i] = nc;
            }
        }

        for (int i = 0; i < N; i++) {
            if (exited[i]) continue;
            if (exits.count({rx[i], ry[i]})) {
                exited[i] = true;
                total += rw[i] * (long long)step;
            }
        }
    }

    // Penalty for non-exited robots
    for (int i = 0; i < N; i++) {
        if (!exited[i]) {
            total += rw[i] * (long long)(T + 1);
        }
    }

    return total;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read the problem input from inf ----
    int H, W, N, M, T;
    H = inf.readInt();
    W = inf.readInt();
    N = inf.readInt();
    M = inf.readInt();
    T = inf.readInt();
    inf.readEoln();

    vector<string> grid(H);
    for (int i = 0; i < H; i++) {
        grid[i] = inf.readToken();
        inf.readEoln();
    }

    vector<tuple<int,int,long long>> robots(N);
    for (int i = 0; i < N; i++) {
        int r, c;
        long long w;
        r = inf.readInt();
        c = inf.readInt();
        w = inf.readLong();
        inf.readEoln();
        robots[i] = make_tuple(r - 1, c - 1, w);
    }

    set<pair<int,int>> exits;
    for (int j = 0; j < M; j++) {
        int e, f;
        e = inf.readInt();
        f = inf.readInt();
        inf.readEoln();
        exits.insert({e - 1, f - 1});
    }

    // ---- Read participant output from ouf ----
    int L = ouf.readInt();
    if (L < 0 || L > T) {
        quitf(_wa, "L=%d is not in [0, %d]", L, T);
    }

    string S = "";
    if (L == 0) {
        // Must read (and ignore) the empty second line if present;
        // we just need to confirm nothing non-whitespace follows L
        // readEof allows trailing newlines/spaces
        ouf.readEoln();
        // Now check EOF (allow trailing blank lines)
        if (!ouf.seekEof()) {
            // Try reading remaining as token; if non-empty, it's extra data
            string extra = ouf.readToken();
            if (!extra.empty()) {
                quitf(_wa, "L=0 but extra non-empty token found: '%s'", extra.c_str());
            }
        }
    } else {
        // Read the command string
        if (ouf.seekEof()) {
            quitf(_wa, "Expected command string of length %d but got EOF", L);
        }
        S = ouf.readToken();

        // Validate length
        if ((int)S.size() != L) {
            quitf(_wa, "L=%d but |S|=%d", L, (int)S.size());
        }

        // Validate characters
        for (int i = 0; i < L; i++) {
            if (S[i] != 'U' && S[i] != 'D' && S[i] != 'L' && S[i] != 'R') {
                quitf(_wa, "Invalid character '%c' at position %d in command string", S[i], i + 1);
            }
        }

        // Strict end-of-file check: no trailing tokens allowed
        if (!ouf.seekEof()) {
            quitf(_wa, "Trailing data after command string");
        }
    }

    // ---- Compute participant objective A ----
    long long A = simulate(H, W, grid, robots, exits, T, S);

    // ---- Compute baseline objective B: RDLU repeating for T steps ----
    string baseline;
    baseline.reserve(T);
    const char pat[4] = {'R', 'D', 'L', 'U'};
    for (int i = 0; i < T; i++) {
        baseline += pat[i % 4];
    }
    long long B = simulate(H, W, grid, robots, exits, T, baseline);

    // ---- Compute score using the exact stated formula ----
    // score = floor(1,000,000 * min(10, B / A))
    // Maximum score per test is 10,000,000; we express as ratio in [0,1]
    // ratio = score / 10,000,000
    long long display_score;
    double ratio;

    if (A <= 0) {
        // Perfect: all robots exited at step 0 or no robots
        display_score = 10000000LL;
        ratio = 1.0;
    } else {
        double ba = (double)B / (double)A;
        double clamped = (ba > 10.0) ? 10.0 : (ba < 0.0 ? 0.0 : ba);
        // Apply floor as stated
        display_score = (long long)floor(1000000.0 * clamped);
        if (display_score > 10000000LL) display_score = 10000000LL;
        if (display_score < 0LL) display_score = 0LL;
        // ratio for quitp: display_score / 10,000,000
        ratio = (double)display_score / 10000000.0;
    }

    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio, "OK. A=%lld, B=%lld, display_score=%lld, Ratio: %.9f",
          A, B, display_score, ratio);

    return 0;
}