#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int T = inf.readInt();

    double total_score = 0.0;

    for (int t = 0; t < T; t++) {
        // Read all test-case data from inf
        int H = inf.readInt();
        int W = inf.readInt();
        int C = inf.readInt();
        int K = inf.readInt();

        vector<long long> p(C + 1);
        long long B = 0;
        for (int c = 1; c <= C; c++) {
            p[c] = inf.readLong();
            B += p[c];
        }

        vector<vector<int>> grid(H + 1, vector<int>(W + 1));
        for (int i = 1; i <= H; i++)
            for (int j = 1; j <= W; j++)
                grid[i][j] = inf.readInt();

        vector<pair<int,int>> marked(K);
        for (int k = 0; k < K; k++) {
            marked[k].first  = inf.readInt();
            marked[k].second = inf.readInt();
        }

        // ---- Read participant output for this test case ----
        // We track feasibility; if infeasible, score_t = 0 and continue.
        bool infeasible = false;
        string infeasible_reason;

        // Read M
        int M = ouf.readInt();
        if (M <= 0 || M > C) {
            infeasible = true;
            infeasible_reason = "Test case " + to_string(t+1) +
                ": M=" + to_string(M) +
                " out of valid range [1," + to_string(C) + "]";
            // Try to skip M integers if M is in a sane range; if not, skip nothing.
            // We must still consume the tokens so later test cases parse correctly.
            // Only attempt to read if M is in a plausible range.
            if (M > 0 && M <= H * W) {
                for (int i = 0; i < M; i++) ouf.readInt();
            }
            // If M <= 0 or M > H*W we have no safe way to skip; just record infeasible.
            total_score += 0.0;
            continue;
        }

        // Read the M kept color IDs
        set<int> kept_set;
        long long X = 0;
        bool read_ok = true;
        for (int i = 0; i < M; i++) {
            int col = ouf.readInt();
            if (col < 1 || col > C) {
                if (!infeasible) {
                    infeasible = true;
                    infeasible_reason = "Test case " + to_string(t+1) +
                        ": color id " + to_string(col) +
                        " out of range [1," + to_string(C) + "]";
                }
                read_ok = false;
                // continue reading to consume all M tokens
            } else if (kept_set.count(col)) {
                if (!infeasible) {
                    infeasible = true;
                    infeasible_reason = "Test case " + to_string(t+1) +
                        ": color " + to_string(col) + " appears more than once";
                }
            } else {
                kept_set.insert(col);
                X += p[col];
            }
        }

        if (infeasible) {
            total_score += 0.0;
            continue;
        }

        // Check every marked cell's color is kept
        for (int k = 0; k < K; k++) {
            int r = marked[k].first, c = marked[k].second;
            int col = grid[r][c];
            if (!kept_set.count(col)) {
                infeasible = true;
                infeasible_reason = "Test case " + to_string(t+1) +
                    ": marked cell (" + to_string(r) + "," + to_string(c) +
                    ") has color " + to_string(col) + " which is not kept";
                break;
            }
        }

        if (infeasible) {
            total_score += 0.0;
            continue;
        }

        // Collect remaining cells and check connectivity via BFS
        long long total_cells = 0;
        int seed_r = -1, seed_c = -1;
        for (int i = 1; i <= H; i++) {
            for (int j = 1; j <= W; j++) {
                if (kept_set.count(grid[i][j])) {
                    total_cells++;
                    if (seed_r == -1) {
                        seed_r = i;
                        seed_c = j;
                    }
                }
            }
        }

        if (total_cells == 0) {
            // M > 0 but no cells? Shouldn't happen given valid input, but guard anyway.
            infeasible = true;
            infeasible_reason = "Test case " + to_string(t+1) + ": no remaining cells";
            total_score += 0.0;
            continue;
        }

        // BFS
        int dx[] = {0, 0, 1, -1};
        int dy[] = {1, -1, 0, 0};
        vector<vector<bool>> visited(H + 1, vector<bool>(W + 1, false));
        queue<pair<int,int>> bfsq;
        bfsq.push({seed_r, seed_c});
        visited[seed_r][seed_c] = true;
        long long reachable = 1;
        while (!bfsq.empty()) {
            auto [x, y] = bfsq.front();
            bfsq.pop();
            for (int d = 0; d < 4; d++) {
                int nx = x + dx[d], ny = y + dy[d];
                if (nx >= 1 && nx <= H && ny >= 1 && ny <= W
                    && !visited[nx][ny]
                    && kept_set.count(grid[nx][ny])) {
                    visited[nx][ny] = true;
                    reachable++;
                    bfsq.push({nx, ny});
                }
            }
        }

        if (reachable != total_cells) {
            infeasible = true;
            infeasible_reason = "Test case " + to_string(t+1) +
                ": remaining cells are NOT 4-connected (reachable=" +
                to_string(reachable) + ", total=" + to_string(total_cells) + ")";
            total_score += 0.0;
            continue;
        }

        // Feasible: compute score_t = min(100, 20 * B / X)
        double score_t = min(100.0, 20.0 * (double)B / (double)X);
        total_score += score_t;
    }

    // Check no trailing garbage in participant output
    if (!ouf.seekEof())
        quitf(_wa, "Participant output has extra data after all test cases");

    double mean_score = total_score / (double)T;  // in [0, 100]
    double ratio = mean_score / 100.0;             // in [0, 1]
    ratio = max(0.0, min(1.0, ratio));

    quitp(ratio, "Ratio: %.6f (mean score: %.4f / 100 over %d test case(s))",
          ratio, mean_score, T);
}