#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef unsigned long long ull;

// modular exponentiation
ll powmod(ll base, ll exp, ll mod) {
    ll result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = (__int128)result * base % mod;
        base = (__int128)base * base % mod;
        exp >>= 1;
    }
    return result;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input ----
    ll d = inf.readLong();
    ll p = inf.readLong();

    int q = inf.readInt();
    int n = inf.readInt();
    int L = inf.readInt();
    ll beta = inf.readLong();

    // data records
    vector<ll> w(n);
    vector<vector<ll>> x(n, vector<ll>(q));
    vector<ll> t(n);

    for (int i = 0; i < n; i++) {
        w[i] = inf.readLong();
        for (int j = 0; j < q; j++) x[i][j] = inf.readLong();
        t[i] = inf.readLong();
    }

    // ---- Compute baseline B ----
    // Best objective with at most 1 computation instruction.
    // Zero-computation: return initial cell value (no penalty)
    //   cells 1..q have x[i][j-1], cells q+1..5000 have 1
    // One-computation (penalty beta):
    //   ^ a c ; f c  -> result = x[i][a-1]^d  (for a in 1..q) or 1^d=1 (non-input)
    //   + a b c ; f c -> result = (va + vb) % p

    ll W = 0;
    for (int i = 0; i < n; i++) W += w[i];

    // Helper: compute weight sum where output[i] == t[i]
    // output[i] is a vector of length n
    auto scoreVec = [&](const vector<ll>& out) -> ll {
        ll s = 0;
        for (int i = 0; i < n; i++)
            if (out[i] == t[i]) s += w[i];
        return s;
    };

    ll B = LLONG_MIN;

    // 0-computation candidates
    // Input cells 1..q
    for (int j = 0; j < q; j++) {
        vector<ll> out(n);
        for (int i = 0; i < n; i++) out[i] = x[i][j];
        ll sc = scoreVec(out); // penalty 0
        B = max(B, sc);
    }
    // Non-input cell (value 1)
    {
        vector<ll> out(n, 1LL % p);
        ll sc = scoreVec(out);
        B = max(B, sc);
    }

    // 1-computation: ^ a c ; f c
    // a in 1..q: result = x[i][a-1]^d mod p
    for (int j = 0; j < q; j++) {
        vector<ll> out(n);
        for (int i = 0; i < n; i++) out[i] = powmod(x[i][j], d, p);
        ll sc = scoreVec(out) - beta;
        B = max(B, sc);
    }
    // a non-input: result = 1^d = 1
    {
        vector<ll> out(n, powmod(1LL, d, p));
        ll sc = scoreVec(out) - beta;
        B = max(B, sc);
    }

    // 1-computation: + a b c ; f c
    // Possible value pairs (va, vb) where each is either x[i][j] or 1 (non-input)
    // distinct "slot" types: input j (0..q-1), or constant-1 (slot q)
    // We enumerate pairs of slot types
    int slots = q + 1; // 0..q-1 are input slots, q is the "1" slot
    for (int sa = 0; sa < slots; sa++) {
        for (int sb = sa; sb < slots; sb++) {
            vector<ll> out(n);
            for (int i = 0; i < n; i++) {
                ll va = (sa < q) ? x[i][sa] : 1LL;
                ll vb = (sb < q) ? x[i][sb] : 1LL;
                out[i] = (va + vb) % p;
            }
            ll sc = scoreVec(out) - beta;
            B = max(B, sc);
        }
    }

    // ---- Parse participant's program ----
    struct Instr {
        char type; // '+', '^', 'f'
        int a, b, c;
    };

    vector<Instr> prog;
    bool feasible = true;
    string feasReason;
    int computeCount = 0;

    // Read all lines from ouf
    vector<string> lines;
    while (!ouf.seekEof()) {
        string line = ouf.readLine();
        // trim
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
        if (!line.empty()) lines.push_back(line);
    }

    if (lines.empty()) {
        quitf(_wa, "Empty program");
    }

    for (int li = 0; li < (int)lines.size(); li++) {
        const string& line = lines[li];
        istringstream ss(line);
        string op;
        ss >> op;
        if (op == "+") {
            int a, b, c;
            if (!(ss >> a >> b >> c)) {
                feasible = false;
                feasReason = "Invalid + instruction: " + line;
                break;
            }
            if (a < 1 || a > 5000 || b < 1 || b > 5000 || c < 1 || c > 5000) {
                feasible = false;
                feasReason = "Cell index out of range in: " + line;
                break;
            }
            if (li == (int)lines.size() - 1) {
                feasible = false;
                feasReason = "Last instruction must be f";
                break;
            }
            computeCount++;
            prog.push_back({'+', a, b, c});
        } else if (op == "^") {
            int a, c;
            if (!(ss >> a >> c)) {
                feasible = false;
                feasReason = "Invalid ^ instruction: " + line;
                break;
            }
            if (a < 1 || a > 5000 || c < 1 || c > 5000) {
                feasible = false;
                feasReason = "Cell index out of range in: " + line;
                break;
            }
            if (li == (int)lines.size() - 1) {
                feasible = false;
                feasReason = "Last instruction must be f";
                break;
            }
            computeCount++;
            prog.push_back({'^', a, 0, c});
        } else if (op == "f") {
            int c;
            if (!(ss >> c)) {
                feasible = false;
                feasReason = "Invalid f instruction: " + line;
                break;
            }
            if (c < 1 || c > 5000) {
                feasible = false;
                feasReason = "Cell index out of range in f: " + line;
                break;
            }
            if (li != (int)lines.size() - 1) {
                feasible = false;
                feasReason = "f instruction is not last";
                break;
            }
            prog.push_back({'f', 0, 0, c});
        } else {
            feasible = false;
            feasReason = "Unknown instruction: " + line;
            break;
        }
    }

    if (feasible) {
        // Check exactly one f instruction at end
        int fcount = 0;
        for (auto& ins : prog) if (ins.type == 'f') fcount++;
        if (fcount != 1) {
            feasible = false;
            feasReason = "Must have exactly one f instruction";
        }
    }

    if (feasible && (prog.empty() || prog.back().type != 'f')) {
        feasible = false;
        feasReason = "Last instruction must be f";
    }

    if (feasible && computeCount > L) {
        feasible = false;
        feasReason = "Too many computation instructions: " + to_string(computeCount) + " > " + to_string(L);
    }

    if (!feasible) {
        // Score is 0
        quitp(0.0, "Infeasible: %s | Ratio: 0", feasReason.c_str());
    }

    // ---- Simulate program on each record ----
    ll m = computeCount;
    ll C = 0;

    vector<ll> cells(5001);
    for (int i = 0; i < n; i++) {
        // Initialize
        for (int j = 1; j <= q; j++) cells[j] = x[i][j-1];
        for (int j = q+1; j <= 5000; j++) cells[j] = 1;

        ll output_val = 0;
        for (auto& ins : prog) {
            if (ins.type == '+') {
                cells[ins.c] = (cells[ins.a] + cells[ins.b]) % p;
            } else if (ins.type == '^') {
                cells[ins.c] = powmod(cells[ins.a], d, p);
            } else { // 'f'
                output_val = cells[ins.c];
            }
        }
        if (output_val == t[i]) C += w[i];
    }

    ll O = C - beta * m;

    // ---- Compute score ----
    // W = sum of all weights
    // if W == B: score = 10^6 if O == W else 0
    // if W > B: score = 10^6 * max(0, (O-B)/(W-B))

    double ratio = 0.0;
    if (W == B) {
        ratio = (O == W) ? 1.0 : 0.0;
    } else {
        // W > B always (since B <= W)
        double num = (double)(O - B);
        double den = (double)(W - B);
        ratio = num / den;
        if (ratio < 0.0) ratio = 0.0;
        if (ratio > 1.0) ratio = 1.0;
    }

    ll per_test_score = (ll)(ratio * 1000000.0);

    quitp(ratio, "C=%lld m=%lld O=%lld B=%lld W=%lld score=%lld | Ratio: %.9f",
          C, m, O, B, W, per_test_score, ratio);

    return 0;
}