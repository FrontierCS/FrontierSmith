#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// BFS to find rooms reachable from room 0 (0-indexed) via broken doors
static vector<bool> computeAccessible(int n, const vector<pair<int,int>>& ep, const vector<bool>& broken) {
    int m = (int)ep.size();
    vector<vector<int>> adj(n);
    for (int j = 0; j < m; j++) {
        if (broken[j]) {
            adj[ep[j].first].push_back(ep[j].second);
            adj[ep[j].second].push_back(ep[j].first);
        }
    }
    vector<bool> acc(n, false);
    queue<int> q;
    acc[0] = true;
    q.push(0);
    while (!q.empty()) {
        int cur = q.front(); q.pop();
        for (int nb : adj[cur]) {
            if (!acc[nb]) { acc[nb] = true; q.push(nb); }
        }
    }
    return acc;
}

// Simulate a strategy given an attack-provider function.
// getAttacks returns vector of (0-based door index, damage) pairs.
// Sets valid=false on any feasibility violation.
static long long simulate(
    int n, int m, int T, long long P,
    const vector<long long>& val,
    const vector<pair<int,int>>& ep,
    const vector<long long>& initDur,
    const vector<long long>& repairAmt,
    function<vector<pair<int,int>>(const vector<bool>&, const vector<long long>&, const vector<bool>&, int, bool&, string&)> getAttacks,
    bool& valid, string& errMsg
) {
    vector<long long> cur(initDur);
    vector<bool> broken(m, false);
    vector<bool> collected(n, false);
    long long total = 0;
    valid = true;
    errMsg = "";

    // Collect artifacts in initially accessible rooms (only room 0 at start, no doors broken)
    {
        auto acc = computeAccessible(n, ep, broken);
        for (int i = 0; i < n; i++) {
            if (acc[i] && !collected[i]) {
                collected[i] = true;
                total += val[i];
            }
        }
    }

    for (int day = 0; day < T; day++) {
        auto acc = computeAccessible(n, ep, broken);

        string localErr;
        bool localValid = true;
        auto attacks = getAttacks(acc, cur, broken, day, localValid, localErr);

        if (!localValid) {
            valid = false;
            errMsg = localErr;
            return 0;
        }

        // Validate attacks
        long long totalDmg = 0;
        set<int> attacked;
        for (auto& [j, d] : attacks) {
            if (j < 0 || j >= m) {
                valid = false;
                errMsg = "door index out of range on day " + to_string(day + 1);
                return 0;
            }
            if (d <= 0) {
                valid = false;
                errMsg = "damage must be positive on day " + to_string(day + 1);
                return 0;
            }
            if (attacked.count(j)) {
                valid = false;
                errMsg = "duplicate door attacked on day " + to_string(day + 1);
                return 0;
            }
            attacked.insert(j);
            if (broken[j]) {
                valid = false;
                errMsg = "attacked a broken door on day " + to_string(day + 1);
                return 0;
            }
            if (!acc[ep[j].first] && !acc[ep[j].second]) {
                valid = false;
                errMsg = "attacked door with no accessible endpoint on day " + to_string(day + 1);
                return 0;
            }
            totalDmg += d;
        }
        if (totalDmg > P) {
            valid = false;
            errMsg = "exceeded budget on day " + to_string(day + 1);
            return 0;
        }

        // Apply damage
        for (auto& [j, d] : attacks) {
            cur[j] = max(0LL, cur[j] - (long long)d);
            if (cur[j] == 0) {
                broken[j] = true;
            }
        }

        // Collect newly accessible rooms
        auto accAfter = computeAccessible(n, ep, broken);
        for (int i = 0; i < n; i++) {
            if (accAfter[i] && !collected[i]) {
                collected[i] = true;
                total += val[i];
            }
        }

        // Night repair: every unbroken door with h > 0 repairs
        for (int j = 0; j < m; j++) {
            if (!broken[j] && cur[j] > 0) {
                cur[j] = min(initDur[j], cur[j] + repairAmt[j]);
            }
        }
    }

    return total;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read input
    int n = inf.readInt();
    int m = inf.readInt();
    int T = inf.readInt();
    long long P = inf.readLong();

    vector<long long> val(n);
    for (int i = 0; i < n; i++) val[i] = inf.readLong();

    vector<pair<int,int>> ep(m);
    vector<long long> initDur(m), repairAmt(m);
    for (int j = 0; j < m; j++) {
        int u = inf.readInt() - 1;
        int w = inf.readInt() - 1;
        ep[j] = {u, w};
        initDur[j] = inf.readLong();
        repairAmt[j] = inf.readLong();
    }

    // Read and validate participant output
    // For each day, read the attacks from ouf
    // We'll parse ouf day by day inside the simulate call

    // We need to parse ouf lazily; store all attacks per day
    vector<vector<pair<int,int>>> participantAttacks(T);
    bool parseOk = true;
    string parseErr = "";

    for (int day = 0; day < T && parseOk; day++) {
        int k = ouf.readInt();
        if (k < 0) {
            parseOk = false;
            parseErr = "negative k on day " + to_string(day + 1);
            break;
        }
        for (int i = 0; i < k; i++) {
            int e = ouf.readInt();
            int d = ouf.readInt();
            participantAttacks[day].push_back({e - 1, d}); // convert to 0-based
        }
    }

    if (!parseOk) {
        quitf(_wa, "%s", parseErr.c_str());
    }

    // Check for trailing content
    // (we do this after simulation to get V first)

    // Simulate participant
    bool pValid = true;
    string pErr = "";
    long long V = simulate(n, m, T, P, val, ep, initDur, repairAmt,
        [&](const vector<bool>& acc, const vector<long long>& cur, const vector<bool>& broken,
            int day, bool& vld, string& err) -> vector<pair<int,int>> {
            (void)acc; (void)cur; (void)broken;
            vld = true;
            (void)err;
            return participantAttacks[day];
        },
        pValid, pErr);

    if (!pValid) {
        quitf(_wa, "Invalid output: %s", pErr.c_str());
    }

    // Check EOF
    if (!ouf.seekEof()) {
        quitf(_wa, "Unexpected trailing content in output");
    }

    // Simulate baseline
    bool bValid = true;
    string bErr = "";
    long long B = simulate(n, m, T, P, val, ep, initDur, repairAmt,
        [&](const vector<bool>& acc, const vector<long long>& cur, const vector<bool>& broken,
            int day, bool& vld, string& err) -> vector<pair<int,int>> {
            (void)day; (void)err;
            vld = true;

            // Collect attackable doors: unbroken, at least one endpoint accessible
            vector<pair<long long,int>> cands;
            for (int j = 0; j < m; j++) {
                if (!broken[j] && (acc[ep[j].first] || acc[ep[j].second])) {
                    cands.push_back({cur[j], j});
                }
            }
            // Sort by cur durability ASC, then door index ASC (0-based index)
            sort(cands.begin(), cands.end(), [](const pair<long long,int>& a, const pair<long long,int>& b) {
                if (a.first != b.first) return a.first < b.first;
                return a.second < b.second;
            });

            vector<pair<int,int>> attacks;
            long long rem = P;
            for (auto& [cd, j] : cands) {
                if (rem <= 0) break;
                if (cd <= rem) {
                    attacks.push_back({j, (int)cd});
                    rem -= cd;
                } else {
                    attacks.push_back({j, (int)rem});
                    rem = 0;
                    break;
                }
            }
            return attacks;
        },
        bValid, bErr);

    if (!bValid) {
        // Baseline should always be valid; if not, just use ratio = 0.5
        B = 0;
    }

    // Compute ratio per statement formula:
    // score = 1,000,000 * min(2, (V+1)/(B+1))
    // max score = 2,000,000; subtask score = 200 in config
    // quitp ratio in [0,1]: ratio = score / 2,000,000
    //   = (1,000,000 * min(2, (V+1)/(B+1))) / 2,000,000
    //   = min(1.0, (V+1.0) / (2.0*(B+1.0)))
    //
    // So: baseline match (V=B) -> ratio = 1/(2) = 0.5 -> 0.5*200 = 100 subtask points = 1,000,000 per statement
    //     double baseline (V=2B) -> ratio = 1.0 -> 1.0*200 = 200 subtask points = 2,000,000 per statement
    //     worse than baseline -> ratio < 0.5

    double dV = (double)V;
    double dB = (double)B;
    double ratio;

    double denom = 2.0 * (dB + 1.0);
    if (denom <= 0.0) {
        ratio = 0.5;
    } else {
        ratio = (dV + 1.0) / denom;
        if (ratio > 1.0) ratio = 1.0;
        if (ratio < 0.0) ratio = 0.0;
    }

    quitp(ratio, "V=%lld B=%lld Ratio: %.9f", V, B, ratio);
    return 0;
}