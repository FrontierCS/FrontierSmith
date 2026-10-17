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

    double ratio = max(0.0L, (long double)(C_base - total_cost) /
                                  ((long double)C_base + 1.0L));
    quitp(ratio, "C=%lld B=%lld Ratio: %.12f", total_cost, C_base, ratio);
}
