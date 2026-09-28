#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input ----
    int N = inf.readInt();
    int K = inf.readInt();
    int C = inf.readInt();
    int M = inf.readInt();

    vector<int> rx1(M), ry1(M), rx2(M), ry2(M);
    vector<long long> rw(M);
    long long W = 0;

    for (int j = 0; j < M; j++) {
        rx1[j] = inf.readInt();
        ry1[j] = inf.readInt();
        rx2[j] = inf.readInt();
        ry2[j] = inf.readInt();
        rw[j] = inf.readLong();
        W += rw[j];
    }

    // ---- Helper: compute objective from a set of queries ----
    // queries[i] = list of (x,y) points (1-indexed)
    auto computeObjective = [&](const vector<vector<pair<int,int>>>& queries) -> long long {
        int numQ = (int)queries.size();

        // Build prefix sums for each query: psum[i][x][y] = # points in query i with x'<=x, y'<=y
        // N<=200, so use (N+1)x(N+1) arrays
        vector<vector<vector<int>>> psum(numQ,
            vector<vector<int>>(N + 2, vector<int>(N + 2, 0)));

        for (int i = 0; i < numQ; i++) {
            // mark points
            for (auto& p : queries[i]) {
                psum[i][p.first][p.second]++;
            }
            // build 2D prefix sum
            for (int x = 1; x <= N; x++)
                for (int y = 1; y <= N; y++)
                    psum[i][x][y] += psum[i][x-1][y] + psum[i][x][y-1] - psum[i][x-1][y-1];
        }

        // For each rectangle j, compute response vector
        // Group by response vector, take max weight per group
        map<vector<int>, long long> groups;

        for (int j = 0; j < M; j++) {
            vector<int> vec(numQ);
            for (int i = 0; i < numQ; i++) {
                // count of query i points inside rectangle j
                int a = rx1[j], b = ry1[j], c = rx2[j], d = ry2[j];
                int cnt = psum[i][c][d]
                        - psum[i][a-1][d]
                        - psum[i][c][b-1]
                        + psum[i][a-1][b-1];
                vec[i] = cnt;
            }
            auto it = groups.find(vec);
            if (it == groups.end()) {
                groups[vec] = rw[j];
            } else {
                it->second = max(it->second, rw[j]);
            }
        }

        long long T = 0;
        for (auto& kv : groups) T += kv.second;
        return T;
    };

    // ---- Compute baseline ----
    // Grid points ordered lexicographically: (1,1),(1,2),...,(1,N),(2,1),...
    // p_1, p_2, ..., p_{N^2}
    // c_i = floor(C/K) + [i <= C mod K]   (1-indexed i)
    // Baseline query i: first c_i points of p_i, p_{i+K}, p_{i+2K}, ...
    auto computeBaseline = [&]() -> long long {
        int base = C / K;
        int rem  = C % K;
        // Build the full ordered list of grid points
        // p[idx] where idx=0..N*N-1 => (x,y) with x from 1..N, y from 1..N
        // ordered as (1,1),(1,2),...,(1,N),(2,1),...
        // p_i (1-indexed) = p[i-1]
        auto getPoint = [&](int idx) -> pair<int,int> {
            // idx is 0-based
            int x = idx / N + 1;
            int y = idx % N + 1;
            return {x, y};
        };
        int total = N * N;

        vector<vector<pair<int,int>>> bqueries(K);
        for (int i = 0; i < K; i++) {
            int ci = base + (i < rem ? 1 : 0);
            // query i (0-indexed): points at positions i, i+K, i+2K, ... (0-indexed in full list)
            int count = 0;
            for (int pos = i; pos < total && count < ci; pos += K, count++) {
                bqueries[i].push_back(getPoint(pos));
            }
        }
        return computeObjective(bqueries);
    };

    long long T_base = computeBaseline();

    // ---- Read and validate participant output ----
    bool feasible = true;
    string infeasible_reason = "";
    vector<vector<pair<int,int>>> pqueries(K);
    int total_pts = 0;

    for (int i = 0; i < K; i++) {
        if (!feasible) {
            // still need to try to read the line, but we've already failed
            // Just try to skip; but we mark infeasible anyway
            // We'll just break out of reading
            break;
        }

        // read t_i
        if (ouf.eof()) {
            feasible = false;
            infeasible_reason = "unexpected EOF at query line";
            break;
        }

        int t = ouf.readInt();
        if (t < 0) {
            feasible = false;
            infeasible_reason = "negative t_i";
            break;
        }

        set<pair<int,int>> seen;
        bool line_ok = true;

        for (int p = 0; p < t; p++) {
            if (ouf.eof()) {
                feasible = false;
                infeasible_reason = "unexpected EOF reading points";
                line_ok = false;
                break;
            }
            int x = ouf.readInt();
            int y = ouf.readInt();
            if (x < 1 || x > N || y < 1 || y > N) {
                feasible = false;
                infeasible_reason = "point coordinates out of range";
                line_ok = false;
                break;
            }
            auto pt = make_pair(x, y);
            if (seen.count(pt)) {
                feasible = false;
                infeasible_reason = "duplicate point in query";
                line_ok = false;
                break;
            }
            seen.insert(pt);
            pqueries[i].push_back(pt);
        }

        if (!line_ok) break;

        total_pts += t;
        if (total_pts > C) {
            feasible = false;
            infeasible_reason = "total points exceed budget C";
            break;
        }
    }

    // Check for extra tokens after K queries (only if feasible so far)
    if (feasible && !ouf.eof()) {
        // Try to read one more token to check for trailing garbage
        // ouf.readEof() will fail if there's content; let's just attempt
        // We check by trying to skip whitespace and see if we're at EOF
        // testlib's eof() should work here
        // Actually, attempt reading an extra char
        // The safe way: try readChar and see
        // But we want to be lenient about trailing newlines; let's check properly
        // ouf.eof() already checks this
        // Some trailing whitespace is allowed; strict check:
        // We skip whitespace manually
        while (!ouf.eof()) {
            char ch = ouf.readChar();
            if (ch != ' ' && ch != '\n' && ch != '\r' && ch != '\t') {
                feasible = false;
                infeasible_reason = "extra tokens after K queries";
                break;
            }
        }
    }

    long long T_you = 0;
    if (feasible) {
        T_you = computeObjective(pqueries);
    }
    // If infeasible, T_you = 0

    // ---- Compute score ----
    double score_ratio;

    if (T_base == W) {
        // Baseline already perfect
        if (T_you == W) {
            score_ratio = 1.0;
        } else {
            // score = 1000000 * T_you / W  ... wait, re-read:
            // "If T_base == W, then the score for the test is 1,000,000"
            // This means the test is trivially solved by baseline, and any solution
            // scores 1,000,000 if feasible? Or only if T_you == W?
            // Re-reading: "If T_base = W, then the score for the test is 1,000,000"
            // This means the per-test score is always 1,000,000 for this test.
            // Regardless of T_you? That seems odd. Let's interpret: the test itself
            // scores 1,000,000 (the baseline gets full score, and presumably any
            // valid submission too, since W-T_base=0 makes the other formula degenerate).
            // We'll award 1,000,000 to everyone on such a test.
            score_ratio = 1.0;
        }
    } else {
        // W > T_base > 0 (T_base < W since T_base <= W always)
        long long floor_score;
        if (T_you <= T_base) {
            // score = 500000 * T_you / T_base
            double sc = 500000.0 * (double)T_you / (double)T_base;
            floor_score = (long long)sc; // floor
        } else {
            // score = 500000 + 500000 * (T_you - T_base) / (W - T_base)
            double sc = 500000.0 + 500000.0 * (double)(T_you - T_base) / (double)(W - T_base);
            floor_score = (long long)sc; // floor
        }
        score_ratio = (double)floor_score / 1000000.0;
    }

    // Clamp to [0, 1]
    if (score_ratio < 0.0) score_ratio = 0.0;
    if (score_ratio > 1.0) score_ratio = 1.0;

    if (!feasible) {
        quitp(0.0, "Infeasible: %s | T_you=0 T_base=%lld W=%lld | Ratio: 0",
              infeasible_reason.c_str(), T_base, W);
    } else {
        quitp(score_ratio, "T_you=%lld T_base=%lld W=%lld | Ratio: %.9f",
              T_you, T_base, W, score_ratio);
    }

    return 0;
}