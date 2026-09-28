#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// ── Levenshtein edit distance ──────────────────────────────────────────────
static int editDist(const string &a, const string &b) {
    int n = (int)a.size(), m = (int)b.size();
    vector<int> dp(m + 1);
    for (int j = 0; j <= m; j++) dp[j] = j;
    for (int i = 1; i <= n; i++) {
        int prev = dp[0];
        dp[0] = i;
        for (int j = 1; j <= m; j++) {
            int tmp = dp[j];
            dp[j] = min({dp[j] + 1, dp[j-1] + 1, prev + (a[i-1] != b[j-1] ? 1 : 0)});
            prev = tmp;
        }
    }
    return dp[m];
}

// ── Template matching ──────────────────────────────────────────────────────
static bool matchTemplate(const string &tmpl, const string &s, int pos,
                          map<char,char> &bindings) {
    if (pos + (int)tmpl.size() > (int)s.size()) return false;
    bindings.clear();
    for (int i = 0; i < (int)tmpl.size(); i++) {
        char tc = tmpl[i], sc = s[pos + i];
        if (tc >= '0' && tc <= '9') {
            if (tc != sc) return false;
        } else {
            auto it = bindings.find(tc);
            if (it != bindings.end()) {
                if (it->second != sc) return false;
            } else {
                bindings[tc] = sc;
            }
        }
    }
    return true;
}

// ── Apply replacement ──────────────────────────────────────────────────────
static string applyReplacement(const string &rhs, const map<char,char> &bindings) {
    if (rhs == "_") return "";
    string result;
    for (char c : rhs)
        result += (c >= '0' && c <= '9') ? c : bindings.at(c);
    return result;
}

// ── Command structure ──────────────────────────────────────────────────────
struct Command {
    string lhs, rhs;
    bool stopAfter;
};

// ── Execute rulebook on one label ──────────────────────────────────────────
static pair<string,int> executeRulebook(const string &input,
                                        const vector<Command> &cmds) {
    string x = input;
    int steps = 0;
    while (true) {
        if (steps >= 200) break;
        if ((int)x.size() > 80) break;
        bool found = false;
        for (int ci = 0; ci < (int)cmds.size(); ci++) {
            const Command &cmd = cmds[ci];
            map<char,char> bindings;
            // find leftmost match
            int matchPos = -1;
            for (int pos = 0; pos <= (int)x.size() - (int)cmd.lhs.size(); pos++) {
                if (matchTemplate(cmd.lhs, x, pos, bindings)) {
                    matchPos = pos;
                    break;
                }
            }
            if (matchPos == -1) continue;
            // apply replacement
            string rep = applyReplacement(cmd.rhs, bindings);
            x = x.substr(0, matchPos) + rep + x.substr(matchPos + cmd.lhs.size());
            steps++;
            found = true;
            if (cmd.stopAfter) goto done;
            break; // restart from first command
        }
        if (!found) break;
    }
    done:
    return {x, steps};
}

// ── Validate template characters ──────────────────────────────────────────
static bool validTemplate(const string &s) {
    for (char c : s) {
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'D')))
            return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    registerTestlibCmd(argc, argv);

    // ── Read input (inf) ──────────────────────────────────────────────
    int n = inf.readInt();
    vector<long long> w(n);
    vector<string> sv(n), tv(n);
    for (int i = 0; i < n; i++) {
        w[i] = inf.readLong();
        sv[i] = inf.readToken();
        tv[i] = inf.readToken();
    }

    // ── Compute baseline B = 1000 * sum(w_i * edit_distance(s_i, t_i)) ──
    long long B = 0;
    for (int i = 0; i < n; i++) {
        B += w[i] * (long long)editDist(sv[i], tv[i]);
    }
    B *= 1000LL;

    // ── Read participant output (ouf) ──────────────────────────────────
    int m = ouf.readInt(0, 100, "m");

    // Validate feasibility: 0 <= m <= 100
    if (m < 0 || m > 100)
        quitf(_wa, "m=%d is out of range [0,100]", m);

    vector<Command> cmds(m);
    long long D = 0; // description length

    for (int ci = 0; ci < m; ci++) {
        string token = ouf.readToken();
        D += (long long)token.size();

        // Parse L->R or L=>R
        size_t arrow2 = token.find("=>");
        size_t arrow1 = token.find("->");
        bool stopAfter;
        string lhs, rhs;

        if (arrow2 != string::npos && (arrow1 == string::npos || arrow2 < arrow1)) {
            stopAfter = true;
            lhs = token.substr(0, arrow2);
            rhs = token.substr(arrow2 + 2);
        } else if (arrow1 != string::npos) {
            stopAfter = false;
            lhs = token.substr(0, arrow1);
            rhs = token.substr(arrow1 + 2);
        } else {
            quitf(_wa, "Command %d: \"%s\" does not contain '->' or '=>'",
                  ci + 1, token.c_str());
        }

        // Validate L length [1,8]
        if (lhs.empty() || (int)lhs.size() > 8)
            quitf(_wa, "Command %d: L=\"%s\" length %d not in [1,8]",
                  ci + 1, lhs.c_str(), (int)lhs.size());

        // Validate R length [1,8] or "_"
        if (rhs != "_" && ((int)rhs.size() < 1 || (int)rhs.size() > 8))
            quitf(_wa, "Command %d: R=\"%s\" length %d not in [1,8] (or '_')",
                  ci + 1, rhs.c_str(), (int)rhs.size());

        // Validate characters in L
        if (!validTemplate(lhs))
            quitf(_wa, "Command %d: L=\"%s\" contains invalid characters",
                  ci + 1, lhs.c_str());

        // Validate characters in R and that every variable in R appears in L
        if (rhs != "_") {
            if (!validTemplate(rhs))
                quitf(_wa, "Command %d: R=\"%s\" contains invalid characters",
                      ci + 1, rhs.c_str());
            set<char> lhsVars;
            for (char c : lhs) if (c >= 'A' && c <= 'D') lhsVars.insert(c);
            for (char c : rhs) {
                if (c >= 'A' && c <= 'D' && lhsVars.find(c) == lhsVars.end())
                    quitf(_wa, "Command %d: R uses variable '%c' not present in L",
                          ci + 1, c);
            }
        }

        cmds[ci] = {lhs, rhs, stopAfter};
    }

    // ── Strict EOF check: no trailing tokens allowed ───────────────────
    if (!ouf.seekEof())
        quitf(_wa, "Trailing content found after the %d commands", m);

    // ── Execute rulebook and compute E, T ─────────────────────────────
    long long E = 0, T = 0;
    for (int i = 0; i < n; i++) {
        auto res = executeRulebook(sv[i], cmds);
        E += w[i] * (long long)editDist(res.first, tv[i]);
        T += w[i] * (long long)res.second;
    }

    long long C = 1000LL * E + T + D;

    // ── Score ──────────────────────────────────────────────────────────
    // Statement: if B=0, score=1_000_000 iff C=0, else 0
    //            else score = round(1_000_000 * max(0, (B-C)/B))
    // checker returns ratio in [0,1]; judge multiplies by 1_000_000 and rounds
    if (B == 0) {
        if (C == 0) {
            quitp(1.0, "B=0 C=0 perfect. Ratio: 1.000000");
        } else {
            quitp(0.0, "B=0 but C=%lld (E=%lld T=%lld D=%lld). Ratio: 0.000000",
                  C, E, T, D);
        }
    }

    double ratio = (double)(B - C) / (double)B;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
          "B=%lld C=%lld (E=%lld T=%lld D=%lld) Ratio: %.6f",
          B, C, E, T, D, ratio);
}