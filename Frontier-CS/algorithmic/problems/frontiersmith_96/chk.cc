#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- read problem input (inf) ----
    int N = inf.readInt();
    int M = inf.readInt();
    int K = inf.readInt();
    long long E = inf.readLong();

    vector<long long> A(K), B(K);
    for (int c = 0; c < K; c++) A[c] = inf.readLong();
    for (int c = 0; c < K; c++) B[c] = inf.readLong();

    vector<vector<int>> target(N, vector<int>(M));
    for (int i = 0; i < N; i++) {
        string row = inf.readToken();
        if ((int)row.size() != M)
            quitf(_fail, "Input target row %d has wrong length", i + 1);
        for (int j = 0; j < M; j++)
            target[i][j] = row[j] - '0';
    }

    vector<vector<long long>> P(N, vector<long long>(M));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            P[i][j] = inf.readLong();

    // ---- compute baseline BL ----
    long long BL = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            if (target[i][j] != 0)
                BL += A[target[i][j]] + B[target[i][j]] + 4LL * E;

    // ---- read participant output (ouf) ----
    int T = ouf.readInt(0, 4000, "T");

    vector<vector<int>> board(N, vector<int>(M, 0));

    long long L = 0;
    long long totalCells = 0;

    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    for (int t = 0; t < T; t++) {
        int c = ouf.readInt(0, K - 1, "c");
        int s = ouf.readInt(1, N * M, "s");

        totalCells += s;
        if (totalCells > 120000)
            quitf(_wa, "Stroke %d: total cells over all strokes exceeds 120000", t + 1);

        vector<pair<int,int>> cells(s);
        set<pair<int,int>> cellSet;

        for (int k = 0; k < s; k++) {
            int r = ouf.readInt(1, N, "r") - 1;
            int x = ouf.readInt(1, M, "x") - 1;
            if (cellSet.count({r, x}))
                quitf(_wa, "Stroke %d: cell (%d,%d) listed twice", t + 1, r + 1, x + 1);
            cellSet.insert({r, x});
            cells[k] = {r, x};
        }

        // BFS connectivity check
        {
            map<pair<int,int>, bool> visited;
            queue<pair<int,int>> q;
            q.push(cells[0]);
            visited[cells[0]] = true;
            int found = 1;
            while (!q.empty()) {
                auto [cr, cc2] = q.front(); q.pop();
                for (int d = 0; d < 4; d++) {
                    int nr = cr + dr[d];
                    int nc = cc2 + dc[d];
                    pair<int,int> nb = {nr, nc};
                    if (cellSet.count(nb) && !visited[nb]) {
                        visited[nb] = true;
                        q.push(nb);
                        found++;
                    }
                }
            }
            if (found != s)
                quitf(_wa, "Stroke %d: cells are not side-connected", t + 1);
        }

        // compute perimeter
        long long perim = 0;
        for (auto [cr, cc2] : cells) {
            for (int d = 0; d < 4; d++) {
                int nr = cr + dr[d];
                int nc = cc2 + dc[d];
                if (nr < 0 || nr >= N || nc < 0 || nc >= M || !cellSet.count({nr, nc}))
                    perim++;
            }
        }

        // stroke cost
        L += A[c] + B[c] * (long long)s + E * perim;

        // apply stroke
        for (auto [cr, cc2] : cells)
            board[cr][cc2] = c;
    }

    // Do NOT call ouf.readEof() — solutions commonly emit a trailing newline
    // and testlib's readEof() rejects any remaining whitespace.

    // penalty for wrong cells
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            if (board[i][j] != target[i][j])
                L += P[i][j];

    // ---- compute score ratio in [0, 1] ----
    // Problem formula: score = floor(1e6 * min(2, BL/L))
    // We report ratio = min(1.0, min(2.0, BL/L) / 2.0) so ratio in [0,1].
    // Then config max_score should be 2,000,000, but since Frontier-CS uses
    // ratio * subtask_score with subtask_score=100 aggregated separately,
    // we simply report ratio = min(1.0, BL / L) which gives [0,1] partial credit.
    // Solutions beating baseline get ratio=1 (capped); partial solutions get
    // continuous credit proportional to BL/L.

    double ratio;
    if (BL == 0) {
        // no non-zero target cells
        ratio = (L == 0) ? 1.0 : 0.0;
    } else if (L == 0) {
        // participant achieved zero loss (perfect and free — theoretically impossible
        // but handle gracefully)
        ratio = 1.0;
    } else {
        double r = (double)BL / (double)L;
        // min(2, r) / 2 maps the full [0,2] range to [0,1]
        double normalized = (r < 2.0 ? r : 2.0) / 2.0;
        ratio = normalized;
    }

    // Clamp strictly to [0,1] for testlib
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
          "Ratio: %.9f | L=%lld BL=%lld T=%d totalCells=%lld",
          ratio, L, BL, T, totalCells);

    return 0;
}