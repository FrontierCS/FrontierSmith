#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static int N, M, K;
static vector<vector<int>> THR, VAL, DEC;

static const int DX[4] = {-1, -1,  1,  1};
static const int DY[4] = {-1,  1, -1,  1};

// Simulate activation and return total energy.
// seeds are 0-indexed (r, c).
long long simulate(const vector<pair<int,int>>& seeds) {
    const int INF = INT_MAX;
    vector<vector<int>> t_act(N, vector<int>(M, INF));
    vector<vector<int>> cnt(N, vector<int>(M, 0));

    queue<pair<int,int>> q;
    for (auto& [r, c] : seeds) {
        if (t_act[r][c] == INF) {
            t_act[r][c] = 0;
            q.push({r, c});
        }
    }

    while (!q.empty()) {
        auto [r, c] = q.front();
        q.pop();
        int t = t_act[r][c];
        for (int d = 0; d < 4; d++) {
            int nr = r + DX[d];
            int nc = c + DY[d];
            if (nr < 0 || nr >= N || nc < 0 || nc >= M) continue;
            if ((nr + nc) % 2 != 0) continue;  // white cell
            if (t_act[nr][nc] != INF) continue; // already active
            cnt[nr][nc]++;
            if (cnt[nr][nc] >= THR[nr][nc]) {
                t_act[nr][nc] = t + 1;
                q.push({nr, nc});
            }
        }
    }

    long long total = 0;
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < M; c++) {
            if ((r + c) % 2 != 0) continue;
            if (t_act[r][c] == INF) continue;
            long long contrib = (long long)VAL[r][c] - (long long)DEC[r][c] * t_act[r][c];
            if (contrib > 0) total += contrib;
        }
    }
    return total;
}

long long computeBaseline() {
    // Collect all black cells (0-indexed)
    vector<pair<int,int>> black_cells;
    for (int r = 0; r < N; r++)
        for (int c = 0; c < M; c++)
            if ((r + c) % 2 == 0)
                black_cells.push_back({r, c});

    // Sort: larger val first, then smaller row (0-indexed), then smaller col
    sort(black_cells.begin(), black_cells.end(), [&](const pair<int,int>& a, const pair<int,int>& b) {
        if (VAL[a.first][a.second] != VAL[b.first][b.second])
            return VAL[a.first][a.second] > VAL[b.first][b.second];
        if (a.first != b.first)
            return a.first < b.first;
        return a.second < b.second;
    });

    int take = min((int)black_cells.size(), K);
    vector<pair<int,int>> base_seeds(black_cells.begin(), black_cells.begin() + take);
    return simulate(base_seeds);
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    N = inf.readInt();
    M = inf.readInt();
    K = inf.readInt();

    THR.assign(N, vector<int>(M, 0));
    VAL.assign(N, vector<int>(M, 0));
    DEC.assign(N, vector<int>(M, 0));

    for (int r = 0; r < N; r++)
        for (int c = 0; c < M; c++)
            THR[r][c] = inf.readInt();
    for (int r = 0; r < N; r++)
        for (int c = 0; c < M; c++)
            VAL[r][c] = inf.readInt();
    for (int r = 0; r < N; r++)
        for (int c = 0; c < M; c++)
            DEC[r][c] = inf.readInt();

    // Read participant output
    int s = ouf.readInt();
    if (s < 0 || s > K) {
        quitf(_wa, "s=%d is out of range [0,%d]", s, K);
    }

    set<pair<int,int>> seen;
    vector<pair<int,int>> seeds; // 0-indexed
    for (int i = 0; i < s; i++) {
        int r = ouf.readInt();
        int c = ouf.readInt();
        // Validate 1-indexed bounds
        if (r < 1 || r > N || c < 1 || c > M) {
            quitf(_wa, "Seed %d: (%d,%d) is out of board bounds [1..%d]x[1..%d]",
                  i + 1, r, c, N, M);
        }
        int r0 = r - 1, c0 = c - 1;
        // Must be a black cell
        if ((r0 + c0) % 2 != 0) {
            quitf(_wa, "Seed %d: (%d,%d) is a white cell", i + 1, r, c);
        }
        if (seen.count({r0, c0})) {
            quitf(_wa, "Seed %d: (%d,%d) is duplicated", i + 1, r, c);
        }
        seen.insert({r0, c0});
        seeds.push_back({r0, c0});
    }

    // Assert no trailing content
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra content in participant output after the seed list");
    }

    // Compute V_sub
    long long V_sub = simulate(seeds);

    // Compute V_base
    long long V_base = computeBaseline();

    // score = 10^6 * min(5, V_sub / max(1, V_base))
    // Normalized to [0,1]: min(5, V_sub / max(1, V_base)) / 5
    double denom = (double)max(1LL, V_base);
    double raw_ratio = (double)V_sub / denom;
    double capped = min(5.0, raw_ratio);
    if (capped < 0.0) capped = 0.0;
    double score_ratio = capped / 5.0;  // normalize to [0, 1]

    quitp(score_ratio,
          "V_sub=%lld V_base=%lld Ratio: %.9f",
          V_sub, V_base, score_ratio);

    return 0;
}