#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

struct Segment {
    char dir;
    long long reps;
};

// Check if customer (cr, cc) is visited by path starting at (1,1) with given segments
bool isVisited(long long cr, long long cc, const vector<Segment>& segs) {
    if (cr == 1 && cc == 1) return true;
    long long row = 1, col = 1;
    for (auto& seg : segs) {
        if (seg.dir == 'D') {
            // visits rows [row .. row+reps] at col
            if (col == cc && row <= cr && cr <= row + seg.reps) return true;
            row += seg.reps;
        } else {
            // visits cols [col .. col+reps] at row
            if (row == cr && col <= cc && cc <= col + seg.reps) return true;
            col += seg.reps;
        }
    }
    return false;
}

long long computeServed(
    const vector<vector<Segment>>& robot_paths,
    const vector<long long>& cr,
    const vector<long long>& cc,
    const vector<long long>& cw,
    int p)
{
    vector<bool> served(p, false);
    for (auto& segs : robot_paths) {
        for (int k = 0; k < p; k++) {
            if (!served[k] && isVisited(cr[k], cc[k], segs)) {
                served[k] = true;
            }
        }
    }
    long long total = 0;
    for (int k = 0; k < p; k++) if (served[k]) total += cw[k];
    return total;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    int t = inf.readInt();

    double total_score = 0.0;
    int total_cases = 0;

    for (int tc = 0; tc < t; tc++) {
        long long n = inf.readLong();
        int m = inf.readInt();
        int p = inf.readInt();

        vector<long long> budget(m);
        vector<string> scripts(m);
        for (int i = 0; i < m; i++) {
            budget[i] = inf.readLong();
            scripts[i] = inf.readToken();
        }

        vector<long long> cr(p), cc(p), cw(p);
        long long U = 0;
        for (int k = 0; k < p; k++) {
            cr[k] = inf.readLong();
            cc[k] = inf.readLong();
            cw[k] = inf.readLong();
            U += cw[k];
        }

        // Compute baseline B: no stretching (all x=0)
        vector<vector<Segment>> baseline_paths(m);
        for (int i = 0; i < m; i++) {
            for (char c : scripts[i]) {
                baseline_paths[i].push_back({c, 1LL});
            }
        }
        long long B = computeServed(baseline_paths, cr, cc, cw, p);

        // Read participant output for this test case
        bool feasible = true;
        string infeas_reason;
        vector<vector<long long>> xvals(m);

        for (int i = 0; i < m && feasible; i++) {
            int len = (int)scripts[i].size();
            xvals[i].resize(len);
            for (int j = 0; j < len; j++) {
                // Read integer in [0, 10^18]
                long long v = ouf.readLong(0LL, (long long)1e18);
                xvals[i][j] = v;
            }
        }

        // If we got here without a readLong failure, validate feasibility
        if (feasible) {
            for (int i = 0; i < m; i++) {
                int len = (int)scripts[i].size();
                long long sum_x = 0;
                long long sum_D = 0, sum_R = 0;
                int orig_D = 0, orig_R = 0;
                for (char c : scripts[i]) {
                    if (c == 'D') orig_D++;
                    else orig_R++;
                }
                for (int j = 0; j < len; j++) {
                    sum_x += xvals[i][j];
                    if (scripts[i][j] == 'D') sum_D += xvals[i][j];
                    else sum_R += xvals[i][j];
                }
                if (sum_x > budget[i]) {
                    feasible = false;
                    infeas_reason = "robot " + to_string(i+1) + " exceeds budget: sum_x=" + to_string(sum_x) + " > b=" + to_string(budget[i]);
                    break;
                }
                // Total D moves = orig_D + sum_D <= n-1
                if ((long long)orig_D + sum_D > n - 1) {
                    feasible = false;
                    infeas_reason = "robot " + to_string(i+1) + " goes out of grid vertically: total D moves = " + to_string((long long)orig_D + sum_D) + " but n-1 = " + to_string(n-1);
                    break;
                }
                // Total R moves = orig_R + sum_R <= n-1
                if ((long long)orig_R + sum_R > n - 1) {
                    feasible = false;
                    infeas_reason = "robot " + to_string(i+1) + " goes out of grid horizontally: total R moves = " + to_string((long long)orig_R + sum_R) + " but n-1 = " + to_string(n-1);
                    break;
                }
            }
        }

        double case_score = 0.0;
        if (!feasible) {
            // Infeasible: score 0 for this test case, continue
            case_score = 0.0;
            // We still need to skip remaining output for this test case if we failed mid-read
            // (already read all xvals up to the failure point, nothing to skip if loop broke early
            // but we need to try to consume remaining robots' lines -- however, since we can't
            // reliably parse further, just assign 0)
        } else {
            // Build participant paths
            vector<vector<Segment>> participant_paths(m);
            for (int i = 0; i < m; i++) {
                int len = (int)scripts[i].size();
                for (int j = 0; j < len; j++) {
                    participant_paths[i].push_back({scripts[i][j], 1LL + xvals[i][j]});
                }
            }

            long long A = computeServed(participant_paths, cr, cc, cw, p);

            if (U == B) {
                case_score = (A == U) ? 1000000.0 : 0.0;
            } else {
                double ratio = (double)(A - B) / (double)(U - B);
                if (ratio < 0.0) ratio = 0.0;
                if (ratio > 1.0) ratio = 1.0;
                case_score = 1000000.0 * ratio;
            }
        }

        total_score += case_score;
        total_cases++;
    }

    // Check no extra output
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output after all test cases");
    }

    double avg_score = (total_cases > 0) ? (total_score / total_cases) : 0.0;
    double final_ratio = avg_score / 1000000.0;
    if (final_ratio < 0.0) final_ratio = 0.0;
    if (final_ratio > 1.0) final_ratio = 1.0;

    quitp(final_ratio, "Ratio: %f (avg score: %.2f / 1000000)", final_ratio, avg_score);
    return 0;
}