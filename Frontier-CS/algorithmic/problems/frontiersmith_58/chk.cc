#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input from inf ----
    int M, N, L;
    M = inf.readInt(); N = inf.readInt(); L = inf.readInt();
    inf.readEoln();

    // Compute balances
    vector<long long> bal(M, 0);
    for (int i = 0; i < N; i++) {
        int a = inf.readInt(), b = inf.readInt();
        long long p = inf.readLong();
        inf.readEoln();
        bal[a] += p;
        bal[b] -= p;
    }

    // Meetup data
    vector<int> cap(L), open_cost(L);
    vector<vector<int>> travel(L, vector<int>(M));
    for (int l = 0; l < L; l++) {
        cap[l] = inf.readInt();
        open_cost[l] = inf.readInt();
        inf.readEoln();
        for (int i = 0; i < M; i++) {
            travel[l][i] = inf.readInt();
        }
        inf.readEoln();
    }

    // S matrix (fixed fee)
    vector<vector<long long>> S(M, vector<long long>(M));
    for (int u = 0; u < M; u++) {
        for (int v = 0; v < M; v++) {
            S[u][v] = inf.readLong();
        }
        inf.readEoln();
    }

    // R matrix (per-euro fee)
    vector<vector<long long>> R(M, vector<long long>(M));
    for (int u = 0; u < M; u++) {
        for (int v = 0; v < M; v++) {
            R[u][v] = inf.readLong();
        }
        inf.readEoln();
    }

    // ---- Compute baseline cost B ----
    // No meetups, standard two-pointer settlement
    {
        // Build debtors and creditors sorted by person index (increasing)
        vector<pair<int,long long>> debtors, creditors;
        for (int i = 0; i < M; i++) {
            if (bal[i] < 0) debtors.push_back({i, -bal[i]});
            else if (bal[i] > 0) creditors.push_back({i, bal[i]});
        }
        // already in index order since we iterated i=0..M-1
    }

    long long B = 0;
    {
        vector<pair<int,long long>> debtors, creditors;
        for (int i = 0; i < M; i++) {
            if (bal[i] < 0) debtors.push_back({i, -bal[i]});
            else if (bal[i] > 0) creditors.push_back({i, bal[i]});
        }
        int di = 0, ci = 0;
        while (di < (int)debtors.size() && ci < (int)creditors.size()) {
            int u = debtors[di].first;   // debtor pays
            int v = creditors[ci].first; // creditor receives
            long long amt = min(debtors[di].second, creditors[ci].second);
            // Transfer: u -> v for amt (debtor sends to creditor)
            B += S[u][v] + R[u][v] * amt;
            debtors[di].second -= amt;
            creditors[ci].second -= amt;
            if (debtors[di].second == 0) di++;
            if (creditors[ci].second == 0) ci++;
        }
    }

    // ---- Read participant output from ouf ----
    // Line 1: assignments A_0 ... A_{M-1}
    vector<int> A(M);
    for (int i = 0; i < M; i++) {
        A[i] = ouf.readInt();
    }
    ouf.readEoln();

    // Validate assignments
    for (int i = 0; i < M; i++) {
        if (A[i] != -1) {
            if (A[i] < 0 || A[i] >= L) {
                quitf(_wa, "Person %d has invalid meetup index %d (L=%d)", i, A[i], L);
            }
            if (travel[A[i]][i] == -1) {
                quitf(_wa, "Person %d cannot attend meetup %d (travel cost is -1)", i, A[i]);
            }
        }
    }

    // Check capacities
    vector<int> meetup_count(L, 0);
    for (int i = 0; i < M; i++) {
        if (A[i] != -1) {
            meetup_count[A[i]]++;
        }
    }
    for (int l = 0; l < L; l++) {
        if (meetup_count[l] > cap[l]) {
            quitf(_wa, "Meetup %d over capacity: %d > %d", l, meetup_count[l], cap[l]);
        }
    }

    // Line 2: K
    int K = ouf.readInt();
    ouf.readEoln();
    if (K < 0) {
        quitf(_wa, "K=%d is negative", K);
    }
    if (K > 10000000) {
        quitf(_wa, "K=%d is too large", K);
    }

    // Read K transfers
    struct Transfer { int u, v; long long x; };
    vector<Transfer> transfers(K);

    // net flow for each person from online transfers
    vector<long long> online_net(M, 0);

    long long C = 0;

    // Add meetup costs
    for (int l = 0; l < L; l++) {
        if (meetup_count[l] > 0) {
            C += open_cost[l];
            for (int i = 0; i < M; i++) {
                if (A[i] == l) {
                    C += travel[l][i];
                }
            }
        }
    }

    for (int k = 0; k < K; k++) {
        int u = ouf.readInt();
        int v = ouf.readInt();
        long long x = ouf.readLong();
        ouf.readEoln();

        if (u < 0 || u >= M) quitf(_wa, "Transfer %d: u=%d out of range", k, u);
        if (v < 0 || v >= M) quitf(_wa, "Transfer %d: v=%d out of range", k, v);
        if (u == v) quitf(_wa, "Transfer %d: u==v==%d", k, u);
        if (x < 1) quitf(_wa, "Transfer %d: x=%lld < 1", k, (long long)x);

        online_net[v] += x;
        online_net[u] -= x;

        C += S[u][v] + R[u][v] * x;

        transfers[k] = {u, v, x};
    }

    // Check EOF
    ouf.readEof();

    // ---- Check balance condition ----
    // For each meetup group
    for (int l = 0; l < L; l++) {
        if (meetup_count[l] == 0) continue;
        long long group_bal = 0;
        long long group_online = 0;
        for (int i = 0; i < M; i++) {
            if (A[i] == l) {
                group_bal += bal[i];
                group_online += online_net[i];
            }
        }
        if (group_bal + group_online != 0) {
            quitf(_wa, "Meetup group %d not balanced: bal=%lld, online_net=%lld, sum=%lld",
                  l, (long long)group_bal, (long long)group_online, (long long)(group_bal + group_online));
        }
    }

    // For each solo person
    for (int i = 0; i < M; i++) {
        if (A[i] == -1) {
            if (bal[i] + online_net[i] != 0) {
                quitf(_wa, "Person %d (solo) not balanced: bal=%lld, online_net=%lld, sum=%lld",
                      i, (long long)bal[i], (long long)online_net[i], (long long)(bal[i] + online_net[i]));
            }
        }
    }

    // ---- Compute score ----
    // ratio = min(1.0, (B+1)/(C+1))
    double ratio = (double)(B + 1) / (double)(C + 1);
    if (ratio > 1.0) ratio = 1.0;
    if (ratio < 0.0) ratio = 0.0;

    quitp(ratio, "Feasible. C=%lld, B=%lld, Ratio: %.9f", (long long)C, (long long)B, ratio);

    return 0;
}