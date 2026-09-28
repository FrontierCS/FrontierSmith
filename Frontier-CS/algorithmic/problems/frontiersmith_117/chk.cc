#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static int R, C, L;

bool playerWon(const vector<vector<int>>& brd, int player) {
    // rows
    for (int r = 0; r < R; r++) {
        int run = 0;
        for (int c = 0; c < C; c++) {
            run = (brd[r][c] == player) ? run + 1 : 0;
            if (run >= L) return true;
        }
    }
    // columns
    for (int c = 0; c < C; c++) {
        int run = 0;
        for (int r = 0; r < R; r++) {
            run = (brd[r][c] == player) ? run + 1 : 0;
            if (run >= L) return true;
        }
    }
    // main diagonals
    for (int sr = 0; sr < R; sr++) {
        for (int sc = 0; sc < C; sc++) {
            if (sr != 0 && sc != 0) continue;
            int run = 0;
            for (int d = 0; sr + d < R && sc + d < C; d++) {
                run = (brd[sr+d][sc+d] == player) ? run + 1 : 0;
                if (run >= L) return true;
            }
        }
    }
    // anti-diagonals
    for (int sr = 0; sr < R; sr++) {
        for (int sc = 0; sc < C; sc++) {
            if (sr != 0 && sc != C-1) continue;
            int run = 0;
            for (int d = 0; sr + d < R && sc - d >= 0; d++) {
                run = (brd[sr+d][sc-d] == player) ? run + 1 : 0;
                if (run >= L) return true;
            }
        }
    }
    return false;
}

bool boardFull(const vector<vector<int>>& brd) {
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++)
            if (brd[r][c] == 0) return false;
    return true;
}

bool gameOver(const vector<vector<int>>& brd) {
    return playerWon(brd, 1) || playerWon(brd, 2) || boardFull(brd);
}

bool snapshotMatches(const vector<vector<int>>& brd, const vector<string>& snap) {
    for (int r = 0; r < R; r++) {
        for (int c = 0; c < C; c++) {
            char ch = snap[r][c];
            if (ch == '?') continue;
            int cell = brd[r][c];
            if (ch == 'X' && cell != 1) return false;
            if (ch == 'O' && cell != 2) return false;
            if (ch == '.' && cell != 0) return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read problem parameters from inf
    R = inf.readInt();
    C = inf.readInt();
    L = inf.readInt();

    int M     = inf.readInt();
    int Gmax  = inf.readInt();
    long long A = inf.readLong();
    long long B = inf.readLong();

    vector<long long> ws(M);
    vector<vector<string>> snaps(M, vector<string>(R));

    long long W = 0;
    for (int i = 0; i < M; i++) {
        ws[i] = inf.readLong();
        W += ws[i];
        for (int r = 0; r < R; r++) {
            snaps[i][r] = inf.readToken();
            if ((int)snaps[i][r].size() != C)
                quitf(_fail, "Snapshot %d row %d has wrong length", i+1, r+1);
        }
    }

    // Read participant output
    int K = ouf.readInt();
    if (K < 0 || K > Gmax)
        quitf(_wa, "K=%d is out of range [0, %d]", K, Gmax);

    vector<bool> explained(M, false);
    long long total_moves = 0;

    for (int s = 0; s < K; s++) {
        int T = ouf.readInt();
        if (T < 1)
            quitf(_wa, "Script %d has T=%d < 1", s+1, T);
        if (T > R * C)
            quitf(_wa, "Script %d has T=%d > R*C=%d", s+1, T, R*C);

        vector<pair<int,int>> moves(T);
        for (int t = 0; t < T; t++) {
            int r = ouf.readInt();
            int c = ouf.readInt();
            if (r < 1 || r > R || c < 1 || c > C)
                quitf(_wa, "Script %d move %d: (%d,%d) out of board", s+1, t+1, r, c);
            moves[t] = {r-1, c-1};
        }

        // Simulate
        vector<vector<int>> brd(R, vector<int>(C, 0));
        for (int t = 0; t < T; t++) {
            int r = moves[t].first;
            int c = moves[t].second;
            int player = (t % 2 == 0) ? 1 : 2;

            if (brd[r][c] != 0)
                quitf(_wa, "Script %d, move %d: cell (%d,%d) already occupied",
                      s+1, t+1, r+1, c+1);

            brd[r][c] = player;

            // Check snapshot matching after this move
            for (int i = 0; i < M; i++) {
                if (!explained[i] && snapshotMatches(brd, snaps[i]))
                    explained[i] = true;
            }

            bool over = gameOver(brd);
            if (t < T - 1) {
                if (over)
                    quitf(_wa, "Script %d: game ended at move %d but script has %d moves total",
                          s+1, t+1, T);
            } else {
                if (!over)
                    quitf(_wa, "Script %d: after %d moves game is not over",
                          s+1, T);
            }
        }

        total_moves += T;
    }

    // Require end-of-file
    if (!ouf.seekEof())
        quitf(_wa, "Extra output found after the last script");

    // Compute objective
    long long covered_value = 0;
    for (int i = 0; i < M; i++)
        if (explained[i]) covered_value += ws[i];

    long long objective = covered_value - A * (long long)K - B * total_moves;

    // score = 100 * clamp(objective / W, 0, 1)
    double score_ratio = 0.0;
    if (W > 0) {
        score_ratio = (double)objective / (double)W;
        if (score_ratio < 0.0) score_ratio = 0.0;
        if (score_ratio > 1.0) score_ratio = 1.0;
    }

    // quitp expects a value in [0,100] for this judge system (statement defines score in [0,100])
    // The Ratio: tag must carry the [0,1] value for the judge regex.
    quitp(score_ratio * 100.0,
          "Ratio: %.9f | covered=%lld W=%lld K=%d moves=%lld obj=%lld A=%lld B=%lld",
          score_ratio, covered_value, W, K, total_moves, objective, A, B);

    return 0;
}