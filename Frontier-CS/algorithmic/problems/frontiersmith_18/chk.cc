#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read problem input
    int M = inf.readInt();
    int L = inf.readInt();

    vector<string> S(M);
    vector<long long> W(M);
    for (int i = 0; i < M; i++) {
        S[i] = inf.readToken();
        W[i] = inf.readLong();
    }

    // Read participant output: M group labels
    vector<int> g(M);
    for (int i = 0; i < M; i++) {
        g[i] = ouf.readInt(1, M, ("g_" + to_string(i + 1)).c_str());
    }

    // Enforce end-of-stream: no trailing garbage allowed
    if (!ouf.seekEof()) {
        quitf(_wa, "Unexpected trailing content after the %d group labels", M);
    }

    // ceil(log2(d)) for d >= 2
    auto ceil_log2 = [](int d) -> int {
        if (d <= 1) return 0;
        int bits = 1;
        while ((1 << bits) < d) bits++;
        return bits;
    };

    // Group indices by label
    unordered_map<int, vector<int>> groups;
    groups.reserve(M * 2);
    for (int i = 0; i < M; i++) {
        groups[g[i]].push_back(i);
    }

    long long total_cost = 0;

    for (auto& kv : groups) {
        const vector<int>& indices = kv.second;

        // Compute total weight of group
        long long W_x = 0;
        for (int i : indices) W_x += W[i];

        long long desc_sum = 0;
        long long data_sum = 0;

        for (int j = 0; j < L; j++) {
            // Collect distinct hex digits at position j using a bitmask
            uint32_t mask = 0;
            for (int i : indices) {
                char c = S[i][j];
                int val;
                if (c >= '0' && c <= '9') val = c - '0';
                else if (c >= 'A' && c <= 'F') val = 10 + (c - 'A');
                else if (c >= 'a' && c <= 'f') val = 10 + (c - 'a');
                else {
                    quitf(_wa, "Invalid hex character '%c' in ID %d at position %d", c, i + 1, j + 1);
                }
                mask |= (1u << val);
            }
            int d = __builtin_popcount(mask);
            if (d == 1) {
                desc_sum += 4;
                // data cost = 0
            } else {
                desc_sum += 16;
                data_sum += ceil_log2(d);
            }
        }

        long long group_cost = 64LL + desc_sum + W_x * data_sum;
        total_cost += group_cost;
    }

    if (total_cost <= 0) {
        quitf(_wa, "Total cost is non-positive: %lld", total_cost);
    }

    // Baseline cost: every ID in its own singleton group
    // C_base = M * (64 + 4*L)
    long long C_base = (long long)M * (64LL + 4LL * L);

    // Problem score formula: min(10^9, floor(10^6 * C_base / total_cost))
    // Used only for the message; ratio computation is done separately.
    long long problem_score = 0;
    {
        __int128 num = (__int128)1000000LL * C_base;
        __int128 sc = num / total_cost;
        if (sc > 1000000000LL) sc = 1000000000LL;
        problem_score = (long long)sc;
    }

    // Normalize ratio to [0, 1]:
    //   ratio = min(1.0, C_base / total_cost)
    //
    // This maps:
    //   - Baseline quality (C == C_base):   ratio = 1.0  (full credit)
    //   - Better than baseline (C < C_base): ratio > 1.0 → clamped to 1.0
    //   - Worse than baseline (C > C_base):  ratio = C_base/C < 1.0
    //   - Total garbage (C >> C_base):       ratio → 0.0
    //
    // This gives continuous spread across all quality levels, avoiding
    // the earlier clustering around 0.001.
    double ratio = (double)C_base / (double)total_cost;
    if (ratio > 1.0) ratio = 1.0;
    if (ratio < 0.0) ratio = 0.0;

    quitp(ratio,
          "Ratio: %.9f (solution_cost=%lld, baseline_cost=%lld, problem_score=%lld)",
          ratio, total_cost, C_base, problem_score);

    return 0;
}