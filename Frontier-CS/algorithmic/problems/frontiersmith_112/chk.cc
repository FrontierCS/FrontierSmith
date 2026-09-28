#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static bool rpsBeats(char u, char t) {
    return (u=='r' && t=='s') || (u=='s' && t=='p') || (u=='p' && t=='r');
}
static bool rpsDraws(char u, char t) { return u == t; }

static long long winPoints(char u, long long R, long long S, long long P) {
    if (u=='r') return R;
    if (u=='s') return S;
    return P;
}

static char beatHand(char t) {
    if (t=='r') return 'p';
    if (t=='s') return 'r';
    return 's';
}

static char roundOutcome(char u, char t) {
    if (rpsBeats(u, t)) return 'W';
    if (rpsDraws(u, t)) return 'D';
    return 'L';
}

struct Condition {
    int type;
    int i, j;
    char x; // for type 1: hand char; for type 2: outcome char W/D/L
};

struct Contract {
    long long W;
    vector<Condition> conds;
};

static bool checkCond(const Condition& c, const string& U, const string& T) {
    int n = (int)U.size();
    int i0 = c.i - 1;
    if (i0 < 0 || i0 >= n) return false;
    if (c.type == 1) {
        return U[i0] == c.x;
    } else if (c.type == 2) {
        return roundOutcome(U[i0], T[i0]) == c.x;
    } else if (c.type == 3) {
        int j0 = c.j - 1;
        if (j0 < 0 || j0 >= n) return false;
        return U[i0] == U[j0];
    } else { // type 4
        int j0 = c.j - 1;
        if (j0 < 0 || j0 >= n) return false;
        return U[i0] != U[j0];
    }
}

static long long computeObjective(const string& U, const string& T,
                                   long long R, long long S, long long P,
                                   const vector<Contract>& contracts) {
    int n = (int)U.size();
    long long base = 0;
    for (int i = 0; i < n; i++) {
        if (rpsBeats(U[i], T[i])) {
            base += winPoints(U[i], R, S, P);
        }
    }
    long long bonus = 0;
    for (const auto& cont : contracts) {
        bool sat = true;
        for (const auto& cond : cont.conds) {
            if (!checkCond(cond, U, T)) { sat = false; break; }
        }
        if (sat) bonus += cont.W;
    }
    return base + bonus;
}

// Compute the baseline solution as described in the problem statement
static string computeBaseline(int N, int K, const string& T) {
    // lex order: r < p < s
    string U0(N, ' ');
    for (int i = 0; i < N; i++) {
        char w = beatHand(T[i]);
        if (i < K || w != U0[i - K]) {
            U0[i] = w;
        } else {
            // Among the two hands different from U0[i-K], choose lex smallest
            // r < p < s
            char forbidden = U0[i - K];
            for (char h : {'r', 'p', 's'}) {
                if (h != forbidden) {
                    U0[i] = h;
                    break;
                }
            }
        }
    }
    return U0;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read input
    int N = inf.readInt();
    int K = inf.readInt();
    int M = inf.readInt();

    long long R = inf.readLong();
    long long S = inf.readLong();
    long long P = inf.readLong();

    string T = inf.readToken();
    if ((int)T.size() != N) {
        quitf(_fail, "Judge input error: |T|=%d, expected %d", (int)T.size(), N);
    }

    vector<Contract> contracts(M);
    for (int m = 0; m < M; m++) {
        contracts[m].W = inf.readLong();
        int C = inf.readInt();
        contracts[m].conds.resize(C);
        for (int c = 0; c < C; c++) {
            contracts[m].conds[c].type = inf.readInt();
            contracts[m].conds[c].i = inf.readInt();
            int tp = contracts[m].conds[c].type;
            if (tp == 1) {
                string sx = inf.readToken();
                contracts[m].conds[c].x = sx[0]; // r, p, or s
            } else if (tp == 2) {
                string sy = inf.readToken();
                contracts[m].conds[c].x = sy[0]; // W, D, or L
            } else {
                // type 3 or 4: i j
                contracts[m].conds[c].j = inf.readInt();
            }
        }
    }

    // Read participant output
    string U = ouf.readToken();

    // Check length
    if ((int)U.size() != N) {
        quitf(_wa, "Output length %d, expected %d", (int)U.size(), N);
    }

    // Check valid characters
    for (int i = 0; i < N; i++) {
        if (U[i] != 'r' && U[i] != 'p' && U[i] != 's') {
            quitf(_wa, "Invalid character '%c' at position %d", U[i], i + 1);
        }
    }

    // Check anti-spam constraint: for every i > K (1-based), U[i] != U[i-K]
    // 0-based: for i in [K, N-1], U[i] != U[i-K]
    for (int i = K; i < N; i++) {
        if (U[i] == U[i - K]) {
            quitf(_wa, "Anti-spam violation at round %d: same hand as round %d",
                  i + 1, i + 1 - K);
        }
    }

    // Strict EOF check: no trailing tokens allowed
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output after the answer string");
    }

    // Compute participant objective
    long long O = computeObjective(U, T, R, S, P, contracts);

    // Compute baseline objective
    string U0 = computeBaseline(N, K, T);
    long long B = computeObjective(U0, T, R, S, P, contracts);

    // score = floor(1,000,000 * clamp(O / (2*B), 0, 1))
    // ratio = clamp(O / (2*B), 0, 1)
    double ratio;
    if (B <= 0) {
        // Baseline is 0; any feasible solution gets full score
        ratio = 1.0;
    } else {
        double denom = 2.0 * (double)B;
        ratio = (double)O / denom;
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
    }

    long long scoreInt = (long long)(1000000.0 * ratio);

    // The judge reads the "Ratio: <value>" tag from this message.
    quitp(ratio, "Ratio: %g | score=%lld | O=%lld | B=%lld", ratio, scoreInt, O, B);
}