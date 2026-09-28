#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Simulate and return objective value
long long simulate(
    int N, int M,
    const string& A, const string& B,
    const string& C, const string& D,
    const vector<string>& T,
    const vector<vector<int>>& V,
    const vector<vector<int>>& P,
    const vector<pair<int,int>>& blockers,
    const vector<pair<char,int>>& activations)
{
    // 0-indexed internally
    vector<vector<char>> canvas(N, vector<char>(M, 'W'));
    long long obj = 0;

    for (auto& bl : blockers) {
        int x = bl.first - 1, y = bl.second - 1;
        canvas[x][y] = 'B';
        obj -= (long long)P[x][y];
    }

    for (auto& act : activations) {
        char side = act.first;
        int idx   = act.second;
        if (side == 'L') {
            int i = idx - 1;
            for (int j = 0; j < M; j++) {
                if (canvas[i][j] != 'W') break;
                canvas[i][j] = 'L';
            }
        } else if (side == 'R') {
            int i = idx - 1;
            for (int j = M - 1; j >= 0; j--) {
                if (canvas[i][j] != 'W') break;
                canvas[i][j] = 'R';
            }
        } else if (side == 'U') {
            int j = idx - 1;
            for (int i = 0; i < N; i++) {
                if (canvas[i][j] != 'W') break;
                canvas[i][j] = 'U';
            }
        } else { // 'D'
            int j = idx - 1;
            for (int i = N - 1; i >= 0; i--) {
                if (canvas[i][j] != 'W') break;
                canvas[i][j] = 'D';
            }
        }
    }

    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            if ((char)canvas[i][j] == T[i][j])
                obj += (long long)V[i][j];

    return obj;
}

// Baseline: do-nothing
long long baseline_nothing(
    int N, int M,
    const vector<string>& T,
    const vector<vector<int>>& V)
{
    long long obj = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            if (T[i][j] == 'W')
                obj += (long long)V[i][j];
    return obj;
}

// Baseline: fixed order (L1..LN, R1..RN, U1..UM, D1..DM), no blockers
long long baseline_fixed(
    int N, int M,
    const string& A, const string& B,
    const string& C, const string& D,
    const vector<string>& T,
    const vector<vector<int>>& V,
    const vector<vector<int>>& P)
{
    vector<pair<char,int>> order;
    for (int i = 1; i <= N; i++) if (A[i-1] == '1') order.push_back({'L', i});
    for (int i = 1; i <= N; i++) if (B[i-1] == '1') order.push_back({'R', i});
    for (int j = 1; j <= M; j++) if (C[j-1] == '1') order.push_back({'U', j});
    for (int j = 1; j <= M; j++) if (D[j-1] == '1') order.push_back({'D', j});
    return simulate(N, M, A, B, C, D, T, V, P, {}, order);
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read problem input
    int N = inf.readInt();
    int M = inf.readInt();
    int K = inf.readInt();
    inf.readEoln();

    string A = inf.readToken();
    inf.readEoln();
    string B = inf.readToken();
    inf.readEoln();
    string C = inf.readToken();
    inf.readEoln();
    string D = inf.readToken();
    inf.readEoln();

    vector<string> T(N);
    for (int i = 0; i < N; i++) {
        T[i] = inf.readToken();
        inf.readEoln();
    }

    vector<vector<int>> V(N, vector<int>(M));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            V[i][j] = inf.readInt();

    vector<vector<int>> P(N, vector<int>(M));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            P[i][j] = inf.readInt();

    // Compute baselines
    long long obj_nothing = baseline_nothing(N, M, T, V);
    long long obj_fixed   = baseline_fixed(N, M, A, B, C, D, T, V, P);
    long long base_val    = max(1LL, max(obj_nothing, obj_fixed));

    // Read participant output
    int b = ouf.readInt();
    int p = ouf.readInt();

    if (b < 0 || b > K)
        quitf(_wa, "b=%d is out of range [0, K=%d]", b, K);
    if (p < 0)
        quitf(_wa, "p=%d is negative", p);

    // Max possible activations: 2*(N+M) distinct assistants
    int max_acts = 2 * N + 2 * M;
    if (p > max_acts)
        quitf(_wa, "p=%d exceeds maximum possible assistants %d", p, max_acts);

    // Read blockers
    set<pair<int,int>> blocker_set;
    vector<pair<int,int>> blockers(b);
    for (int k = 0; k < b; k++) {
        int x = ouf.readInt();
        int y = ouf.readInt();
        if (x < 1 || x > N)
            quitf(_wa, "Blocker #%d: x=%d out of range 1..%d", k+1, x, N);
        if (y < 1 || y > M)
            quitf(_wa, "Blocker #%d: y=%d out of range 1..%d", k+1, y, M);
        if (blocker_set.count({x, y}))
            quitf(_wa, "Blocker #%d: duplicate position (%d, %d)", k+1, x, y);
        blocker_set.insert({x, y});
        blockers[k] = {x, y};
    }

    // Read activations
    set<pair<char,int>> used_assistants;
    vector<pair<char,int>> activations(p);
    for (int k = 0; k < p; k++) {
        string s = ouf.readToken();
        if (s != "L" && s != "R" && s != "U" && s != "D")
            quitf(_wa, "Activation #%d: invalid side '%s'", k+1, s.c_str());
        char side = s[0];

        int id = ouf.readInt();
        if (side == 'L' || side == 'R') {
            if (id < 1 || id > N)
                quitf(_wa, "Activation #%d: row %d out of range 1..%d", k+1, id, N);
            if (side == 'L' && A[id-1] != '1')
                quitf(_wa, "Activation #%d: no L assistant at row %d", k+1, id);
            if (side == 'R' && B[id-1] != '1')
                quitf(_wa, "Activation #%d: no R assistant at row %d", k+1, id);
        } else {
            if (id < 1 || id > M)
                quitf(_wa, "Activation #%d: col %d out of range 1..%d", k+1, id, M);
            if (side == 'U' && C[id-1] != '1')
                quitf(_wa, "Activation #%d: no U assistant at col %d", k+1, id);
            if (side == 'D' && D[id-1] != '1')
                quitf(_wa, "Activation #%d: no D assistant at col %d", k+1, id);
        }

        pair<char,int> key = {side, id};
        if (used_assistants.count(key))
            quitf(_wa, "Activation #%d: assistant %c %d activated more than once",
                  k+1, side, id);
        used_assistants.insert(key);
        activations[k] = key;
    }

    // Ensure no trailing tokens
    if (!ouf.seekEof())
        quitf(_wa, "Extra content found after expected output");

    // Simulate participant's plan
    long long obj = simulate(N, M, A, B, C, D, T, V, P, blockers, activations);

    // Compute score ratio matching the statement formula:
    //   Score_t = floor(10^6 * min(10, max(0, Obj_t) / Base_t))
    // The platform computes floor(subtask_score * ratio).
    // subtask_score = 10^7 in config.yaml, so:
    //   floor(10^7 * ratio) = floor(10^6 * min(10, max(0, Obj/Base)))
    // iff ratio = min(10, max(0, Obj/Base)) / 10
    double raw_ratio = (double)obj / (double)base_val;
    double clamped   = min(10.0, max(0.0, raw_ratio));
    double ratio     = clamped / 10.0;  // in [0, 1]

    // Tag "Ratio: <value>" is required by the judge
    quitp(ratio,
          "Obj: %lld, Base: %lld (nothing=%lld, fixed=%lld), Ratio: %.6f",
          obj, base_val, obj_nothing, obj_fixed, ratio);

    return 0;
}