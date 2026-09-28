#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static int M, N, R, C, D, S, P;
static vector<string> grid;   // 0-indexed
static vector<vector<int>> val;

bool inGrid(int r, int c) {
    return r >= 0 && r < M && c >= 0 && c < N;
}
bool isTown(int r, int c) {
    return inGrid(r, c) && grid[r][c] == '.';
}

// BFS from source cells through towns only, up to distance D. Returns secured flags.
vector<bool> bfsSecure(const vector<pair<int,int>>& sources) {
    vector<bool> secured(M * N, false);
    vector<vector<int>> dist(M, vector<int>(N, INT_MAX));
    deque<pair<int,int>> q;
    for (auto& p : sources) {
        int sr = p.first, sc = p.second;
        if (isTown(sr, sc) && dist[sr][sc] == INT_MAX) {
            dist[sr][sc] = 0;
            q.push_back({sr, sc});
        }
    }
    int dr4[] = {-1, 1, 0, 0};
    int dc4[] = {0, 0, -1, 1};
    while (!q.empty()) {
        int r = q.front().first, c = q.front().second;
        q.pop_front();
        secured[r * N + c] = true;
        if (dist[r][c] < D) {
            for (int d = 0; d < 4; d++) {
                int nr = r + dr4[d], nc = c + dc4[d];
                if (isTown(nr, nc) && dist[nr][nc] == INT_MAX) {
                    dist[nr][nc] = dist[r][c] + 1;
                    q.push_back({nr, nc});
                }
            }
        }
    }
    return secured;
}

// Check if (r2,c2) is reachable from (r1,c1) by exactly one legal leap (1-indexed input)
bool isLegalLeap(int r1, int c1, int r2, int c2) {
    if (r2 <= r1) return false;
    int drow = r2 - r1;
    int dcol = abs(c2 - c1);
    if (drow == R && dcol == C) return true;
    if (drow == C && dcol == R) return true;
    return false;
}

// Precompute per-town BFS coverage (0-indexed) for baseline
vector<vector<pair<int,int>>> townReach; // townReach[r*N+c] = list of towns reachable within D

void precomputeReach() {
    townReach.resize(M * N);
    int dr4[] = {-1, 1, 0, 0};
    int dc4[] = {0, 0, -1, 1};
    for (int sr = 0; sr < M; sr++) {
        for (int sc = 0; sc < N; sc++) {
            if (!isTown(sr, sc)) continue;
            int idx = sr * N + sc;
            vector<vector<int>> dist(M, vector<int>(N, INT_MAX));
            deque<pair<int,int>> q;
            dist[sr][sc] = 0;
            q.push_back({sr, sc});
            while (!q.empty()) {
                int r = q.front().first, c = q.front().second;
                q.pop_front();
                townReach[idx].push_back({r, c});
                if (dist[r][c] < D) {
                    for (int d = 0; d < 4; d++) {
                        int nr = r + dr4[d], nc = c + dc4[d];
                        if (isTown(nr, nc) && dist[nr][nc] == INT_MAX) {
                            dist[nr][nc] = dist[r][c] + 1;
                            q.push_back({nr, nc});
                        }
                    }
                }
            }
        }
    }
}

long long computeBaseline() {
    precomputeReach();

    vector<bool> secured(M * N, false);
    long long totalVal = 0;
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (isTown(i, j)) totalVal += val[i][j];

    bool chosen(false);
    // We'll track which towns are "chosen" singletons
    vector<bool> chosenSingleton(M * N, false);
    long long numSingletons = 0;

    while (true) {
        int bestGain = 0; // must be strictly positive (> 0) after subtracting S
        int bestR = -1, bestC = -1;

        for (int r = 0; r < M; r++) {
            for (int c = 0; c < N; c++) {
                if (!isTown(r, c)) continue;
                if (chosenSingleton[r * N + c]) continue;
                // Compute gain
                int g = 0;
                for (auto& p : townReach[r * N + c]) {
                    int tr = p.first, tc = p.second;
                    if (!secured[tr * N + tc]) g += val[tr][tc];
                }
                int netGain = g - S;
                if (netGain > bestGain ||
                    (netGain == bestGain && bestGain > 0 && bestR != -1 &&
                     (r < bestR || (r == bestR && c < bestC)))) {
                    bestGain = netGain;
                    bestR = r;
                    bestC = c;
                }
            }
        }

        if (bestR == -1 || bestGain <= 0) break;

        // Choose this singleton
        chosenSingleton[bestR * N + bestC] = true;
        numSingletons++;
        // Mark secured
        for (auto& p : townReach[bestR * N + bestC]) {
            secured[p.first * N + p.second] = true;
        }
    }

    // Compute unsecured value
    long long U = 0;
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (isTown(i, j) && !secured[i * N + j])
                U += val[i][j];

    return U + (long long)S * numSingletons;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read input
    M = inf.readInt();
    N = inf.readInt();
    R = inf.readInt();
    C = inf.readInt();
    D = inf.readInt();
    S = inf.readInt();
    P = inf.readInt();

    grid.resize(M);
    for (int i = 0; i < M; i++) {
        grid[i] = inf.readToken();
    }

    val.assign(M, vector<int>(N, 0));
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            val[i][j] = inf.readInt();

    // Read participant output
    int K = ouf.readInt();
    if (K < 0) {
        quitf(_wa, "K=%d is negative", K);
    }

    // townUsed: 0-indexed
    vector<vector<bool>> townUsed(M, vector<bool>(N, false));
    vector<vector<pair<int,int>>> columns(K);

    for (int k = 0; k < K; k++) {
        int L = ouf.readInt();
        if (L < 1) {
            quitf(_wa, "Column %d has L=%d < 1", k + 1, L);
        }
        columns[k].resize(L);
        for (int t = 0; t < L; t++) {
            int r = ouf.readInt();
            int ci = ouf.readInt();
            // Validate 1-indexed
            if (r < 1 || r > M || ci < 1 || ci > N) {
                quitf(_wa,
                      "Column %d, town %d: (%d,%d) is outside the grid [1..%d][1..%d]",
                      k + 1, t + 1, r, ci, M, N);
            }
            int ri = r - 1, cj = ci - 1; // convert to 0-indexed
            if (!isTown(ri, cj)) {
                quitf(_wa,
                      "Column %d, town %d: (%d,%d) is a cliff, not a town",
                      k + 1, t + 1, r, ci);
            }
            if (townUsed[ri][cj]) {
                quitf(_wa,
                      "Column %d, town %d: (%d,%d) is already used in a previous column or earlier in this column",
                      k + 1, t + 1, r, ci);
            }
            townUsed[ri][cj] = true;
            columns[k][t] = {r, ci}; // keep 1-indexed for leap checks
        }
        // Check legal leaps: consecutive pairs
        for (int t = 0; t + 1 < L; t++) {
            int r1 = columns[k][t].first,  c1 = columns[k][t].second;
            int r2 = columns[k][t+1].first, c2 = columns[k][t+1].second;
            if (!isLegalLeap(r1, c1, r2, c2)) {
                quitf(_wa,
                      "Column %d: leap from (%d,%d) to (%d,%d) is not a legal leap with R=%d, C=%d",
                      k + 1, r1, c1, r2, c2, R, C);
            }
        }
    }

    // Check end of participant output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra data found after all columns");
    }

    // Compute participant's cost
    vector<pair<int,int>> allVisited;
    long long Ltotal = 0;
    for (int k = 0; k < K; k++) {
        int L = (int)columns[k].size();
        Ltotal += (L - 1);
        for (int t = 0; t < L; t++) {
            allVisited.push_back({columns[k][t].first - 1, columns[k][t].second - 1});
        }
    }

    vector<bool> securedYou = bfsSecure(allVisited);
    long long U_you = 0;
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (isTown(i, j) && !securedYou[i * N + j])
                U_you += val[i][j];

    long long cost_you = U_you + (long long)S * K + (long long)P * Ltotal;

    // Compute baseline cost
    long long cost_base = computeBaseline();

    // Compute score ratio continuously in [0,1]
    // Problem formula: score = 1,000,000 * min(2, C_base / C_you)
    // Map to [0,1]: score_ratio = min(1.0, min(2.0, C_base/C_you) / 2.0)
    // This gives 0.5 when you match baseline, 1.0 when you are 2x better
    double score_ratio;
    if (cost_you <= 0) {
        // cost_you can't be 0 if there are towns (values >= 1), but be safe
        score_ratio = 1.0;
    } else {
        double raw = (double)cost_base / (double)cost_you;
        if (raw > 2.0) raw = 2.0;
        score_ratio = raw / 2.0;
    }
    if (score_ratio < 0.0) score_ratio = 0.0;
    if (score_ratio > 1.0) score_ratio = 1.0;

    quitp(score_ratio,
          "K=%d Ltotal=%lld cost_you=%lld cost_base=%lld Ratio: %.9f",
          K, Ltotal, cost_you, cost_base, score_ratio);
    return 0;
}