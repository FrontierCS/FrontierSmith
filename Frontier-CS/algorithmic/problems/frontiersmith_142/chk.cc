#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Compute total harmonic interference for a set of racks.
// rack_members[r] = list of A-values in that rack (0-indexed).
long long computeInterference(const vector<vector<int>>& rack_members) {
    long long total = 0;
    for (const auto& members : rack_members) {
        if (members.size() < 2) continue;
        int maxv = *max_element(members.begin(), members.end());
        vector<int> freq(maxv + 1, 0);
        for (int v : members) freq[v]++;
        long long rack_conflict = 0;
        for (int v = 1; v <= maxv; v++) {
            if (freq[v] == 0) continue;
            // pairs of equal values: x divides x
            rack_conflict += (long long)freq[v] * (freq[v] - 1) / 2;
            // v divides each proper multiple
            for (int m = 2 * v; m <= maxv; m += v) {
                rack_conflict += (long long)freq[v] * freq[m];
            }
        }
        total += rack_conflict;
    }
    return total;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- read problem input ----
    int N = inf.readInt();
    int K = inf.readInt();
    int rack_size = N / K;

    vector<int> A(N);
    for (int i = 0; i < N; i++) {
        A[i] = inf.readInt();
    }

    // ---- read participant output: exactly N integers ----
    vector<int> R(N);
    for (int i = 0; i < N; i++) {
        R[i] = ouf.readInt(1, K, ("R_" + to_string(i + 1)).c_str());
    }

    // Strict end-of-file check: no trailing tokens allowed beyond whitespace
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output found after the %d rack assignments", N);
    }

    // ---- feasibility: each rack must have exactly rack_size generators ----
    vector<int> rack_count(K + 1, 0);
    for (int i = 0; i < N; i++) rack_count[R[i]]++;
    for (int r = 1; r <= K; r++) {
        if (rack_count[r] != rack_size) {
            quitf(_wa, "Rack %d has %d generators but expected %d",
                  r, rack_count[r], rack_size);
        }
    }

    // ---- build rack members for participant solution ----
    vector<vector<int>> rack_members(K);
    for (int i = 0; i < N; i++) {
        rack_members[R[i] - 1].push_back(A[i]);
    }

    // ---- compute participant interference X ----
    long long X = computeInterference(rack_members);

    // ---- compute baseline interference B ----
    // Baseline: generator i (0-indexed) -> rack (i % K) (0-indexed)
    vector<vector<int>> baseline_members(K);
    for (int i = 0; i < N; i++) {
        baseline_members[i % K].push_back(A[i]);
    }
    long long B = computeInterference(baseline_members);

    // ---- score formula from the problem statement ----
    // stated score = 10^6 * min(5, (B+1)/(X+1))
    // max stated score = 5 * 10^6  (when ratio_raw = 5.0)
    //
    // The judge config sets subtask score = 5 000 000.
    // We pass ratio = min(5,(B+1)/(X+1)) / 5  in [0,1] to quitp.
    // The judge then computes: ratio * 5 000 000 = 10^6 * min(5,(B+1)/(X+1)).
    //
    // Matching baseline gives ratio_raw=1 => ratio=0.2 => score = 1 000 000. Correct.
    // Perfect (X=0) with B>0 can give up to ratio=1.0 => score = 5 000 000.

    double ratio_raw = (double)(B + 1) / (double)(X + 1);
    if (ratio_raw > 5.0) ratio_raw = 5.0;
    if (ratio_raw < 0.0) ratio_raw = 0.0;

    double ratio = ratio_raw / 5.0;  // normalized to [0, 1]

    // "Ratio: <value>" tag is parsed by the judge to determine the per-file score.
    quitp(ratio, "Ratio: %.9f X=%lld B=%lld score=%.0f",
          ratio, X, B, ratio * 5000000.0);
    return 0;
}