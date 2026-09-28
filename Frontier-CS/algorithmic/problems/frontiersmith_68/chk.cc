#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input ----
    int K = inf.readInt();
    int Q = inf.readInt();
    long long C = inf.readLong(); // cache capacity (we read it but do NOT hard-fail on it)

    vector<long long> p(K + 1);
    for (int i = 0; i <= K; i++) p[i] = inf.readLong();

    vector<int> reqL(Q + 1), reqR(Q + 1);
    for (int i = 1; i <= Q; i++) {
        reqL[i] = inf.readInt();
        reqR[i] = inf.readInt();
    }

    // ---- Compute baseline via matrix-chain DP ----
    const long long INF64 = (long long)4e18;
    vector<vector<long long>> dp(K + 2, vector<long long>(K + 2, 0LL));
    for (int len = 2; len <= K; len++) {
        for (int l = 1; l + len - 1 <= K; l++) {
            int r = l + len - 1;
            dp[l][r] = INF64;
            for (int k = l; k < r; k++) {
                long long cost = dp[l][k] + dp[k + 1][r] + p[l - 1] * p[k] * p[r];
                if (cost < dp[l][r]) dp[l][r] = cost;
            }
        }
    }

    long long baseline = 0;
    for (int i = 1; i <= Q; i++) {
        baseline += dp[reqL[i]][reqR[i]];
    }

    // ---- Read participant output ----
    // S: number of operations
    if (ouf.eof())
        quitf(_wa, "Output is empty; expected number of operations S");

    int S = ouf.readInt(1, 1000000, "number of operations S");

    // Simulation state
    set<pair<int,int>> cached;   // non-trivial blocks currently in cache
    long long totalCost = 0;
    vector<bool> answered(Q + 1, false);

    auto isAvailable = [&](int l, int r) -> bool {
        if (l == r) return true;  // original single matrices always available
        return cached.count({l, r}) > 0;
    };

    for (int opIdx = 1; opIdx <= S; opIdx++) {
        string type = ouf.readToken("(MUL|DEL|OUT)", "operation type");

        if (type == "MUL") {
            int l = ouf.readInt(1, K, "MUL l");
            int k = ouf.readInt(1, K - 1, "MUL k");
            int r = ouf.readInt(1, K, "MUL r");

            if (l >= r)
                quitf(_wa, "Operation %d: MUL requires l < r, got l=%d r=%d", opIdx, l, r);
            if (k < l || k >= r)
                quitf(_wa, "Operation %d: MUL requires l <= k < r, got l=%d k=%d r=%d", opIdx, l, k, r);

            if (!isAvailable(l, k))
                quitf(_wa, "Operation %d: MUL block [%d,%d] not available", opIdx, l, k);
            if (!isAvailable(k + 1, r))
                quitf(_wa, "Operation %d: MUL block [%d,%d] not available", opIdx, k + 1, r);
            if (cached.count({l, r}))
                quitf(_wa, "Operation %d: MUL block [%d,%d] already cached", opIdx, l, r);

            long long cost = p[l - 1] * p[k] * p[r];
            totalCost += cost;
            cached.insert({l, r});

        } else if (type == "DEL") {
            int l = ouf.readInt(1, K, "DEL l");
            int r = ouf.readInt(1, K, "DEL r");

            if (l >= r)
                quitf(_wa, "Operation %d: DEL requires l < r (cannot delete original), got l=%d r=%d", opIdx, l, r);
            if (!cached.count({l, r}))
                quitf(_wa, "Operation %d: DEL block [%d,%d] not in cache", opIdx, l, r);

            cached.erase({l, r});

        } else if (type == "OUT") {
            int q = ouf.readInt(1, Q, "OUT request index");

            if (answered[q])
                quitf(_wa, "Operation %d: OUT request %d already answered", opIdx, q);
            if (!isAvailable(reqL[q], reqR[q]))
                quitf(_wa, "Operation %d: OUT request %d block [%d,%d] not available",
                      opIdx, q, reqL[q], reqR[q]);

            answered[q] = true;

        } else {
            quitf(_wa, "Operation %d: unknown operation type '%s'", opIdx, type.c_str());
        }
    }

    // Check all requests answered
    for (int i = 1; i <= Q; i++) {
        if (!answered[i])
            quitf(_wa, "Request %d (block [%d,%d]) was never answered", i, reqL[i], reqR[i]);
    }

    // Lenient EOF: allow trailing whitespace/newlines
    // We just try to read the next non-whitespace token; if there is one, it's extra garbage.
    // ouf in testlib: use readEof only if we want strict; here we allow trailing whitespace.
    // We skip by attempting a peek.
    {
        // Check that no non-whitespace tokens remain
        // readTokenTo with empty set means EOF or only whitespace
        string extra = "";
        // Try reading; if not EOF, that's extra content
        if (!ouf.seekEof()) {
            // There are more non-whitespace tokens
            extra = ouf.readToken();
            quitf(_wa, "Extra content after %d operations: '%s'", S, extra.c_str());
        }
    }

    // ---- Compute score ----
    // Score = 100 * min(2, baseline / totalCost), normalized to [0,1] for quitp
    // i.e., ratio = min(1.0, (baseline / totalCost) / 2.0)
    double scoreRatio;
    if (totalCost == 0) {
        // If no multiplications were performed and baseline > 0, that's suspicious,
        // but we trust the simulation: if all requests were answered with only original
        // matrices (all L==R, impossible since L<R per problem), then fine.
        // In practice totalCost==0 means baseline==0 too.
        if (baseline == 0) {
            scoreRatio = 1.0;
        } else {
            // participant used 0 cost but baseline > 0: perfect solution
            scoreRatio = 1.0;
        }
    } else {
        double raw = (double)baseline / (double)totalCost;  // typically >= 0
        // raw/2.0 maps the [0, 2] range to [0, 1]
        scoreRatio = min(1.0, max(0.0, raw / 2.0));
    }

    quitp(scoreRatio,
          "Ratio: %.9f | baseline=%lld participant_cost=%lld",
          scoreRatio, baseline, totalCost);

    return 0;
}