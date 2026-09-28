#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Compute occupied cells for a trace given run-lengths, starting direction, and shifts.
vector<pair<long long,long long>> computeCells(
    const vector<int>& runs, bool startUp, long long dx, long long dy)
{
    vector<pair<long long,long long>> cells;
    long long cx = 0, cy = 0;
    bool goUp = startUp;
    for (int run : runs) {
        for (int s = 0; s < run; s++) {
            if (goUp) {
                // up-step: cell is (cx, cy)
                cells.push_back({dx + cx, dy + cy});
                cx++; cy++;
            } else {
                // down-step: cell is (cx, cy-1)
                cells.push_back({dx + cx, dy + cy - 1});
                cx++; cy--;
            }
        }
        goUp = !goUp;
    }
    return cells;
}

// Compute local y-range for a trace given starting direction (no shifts).
pair<long long,long long> localYRange(const vector<int>& runs, bool startUp) {
    long long cx = 0, cy = 0;
    bool goUp = startUp;
    long long ymin = (long long)2e18, ymax = -(long long)2e18;
    for (int run : runs) {
        for (int s = 0; s < run; s++) {
            long long cellY;
            if (goUp) {
                cellY = cy;
                cx++; cy++;
            } else {
                cellY = cy - 1;
                cx++; cy--;
            }
            ymin = min(ymin, cellY);
            ymax = max(ymax, cellY);
        }
        goUp = !goUp;
    }
    return {ymin, ymax};
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---------- read problem input ----------
    int n = inf.readInt();
    int W = inf.readInt();

    vector<vector<int>> traces(n);
    vector<int> L(n, 0);
    for (int i = 0; i < n; i++) {
        int k = inf.readInt();
        traces[i].resize(k);
        for (int j = 0; j < k; j++) {
            traces[i][j] = inf.readInt();
            L[i] += traces[i][j];
        }
    }

    // ---------- compute baseline H_base ----------
    // Baseline: X_i=0, D_i=U, stack vertically in order.
    long long H_base = 0;
    for (int i = 0; i < n; i++) {
        auto [ylo, yhi] = localYRange(traces[i], true /*startUp*/);
        H_base += (yhi - ylo + 1);
    }

    // ---------- read participant output ----------
    vector<long long> Xi(n), Yi(n);
    vector<char> Di(n);

    for (int i = 0; i < n; i++) {
        // Read Xi
        if (ouf.seekEof()) {
            quitf(_wa, "Trace %d: unexpected end of output while reading X_i", i+1);
        }
        Xi[i] = ouf.readLong();

        // Read Yi
        if (ouf.seekEof()) {
            quitf(_wa, "Trace %d: unexpected end of output while reading Y_i", i+1);
        }
        Yi[i] = ouf.readLong();

        // Read Di
        if (ouf.seekEof()) {
            quitf(_wa, "Trace %d: unexpected end of output while reading D_i", i+1);
        }
        string ds = ouf.readToken();
        if (ds != "U" && ds != "D") {
            quitf(_wa, "Trace %d: D_i must be 'U' or 'D', got '%s'", i+1, ds.c_str());
        }
        Di[i] = ds[0];
    }

    // ---------- validate constraints ----------
    map<pair<long long,long long>, int> cellOwner;
    long long globalYmin = (long long)2e18;
    long long globalYmax = -(long long)2e18;

    for (int i = 0; i < n; i++) {
        // Constraint 1: 0 <= X_i <= W - L_i
        if (Xi[i] < 0 || Xi[i] > (long long)(W - L[i])) {
            quitf(_wa, "Trace %d: X_i=%lld out of range [0, %d]", i+1, Xi[i], W - L[i]);
        }
        // Constraint 2: |Y_i| <= 1e9
        if (Yi[i] < -1000000000LL || Yi[i] > 1000000000LL) {
            quitf(_wa, "Trace %d: Y_i=%lld out of range [-1e9, 1e9]", i+1, Yi[i]);
        }
        // Constraint 3: Di already validated above

        bool startUp = (Di[i] == 'U');
        auto cells = computeCells(traces[i], startUp, Xi[i], Yi[i]);

        for (auto& cell : cells) {
            long long gx = cell.first, gy = cell.second;
            // Constraint 5: 0 <= x < W
            if (gx < 0 || gx >= (long long)W) {
                quitf(_wa, "Trace %d: cell (%lld,%lld) has x out of [0,%d)", i+1, gx, gy, W);
            }
            // Constraint 4: no two traces share a cell
            auto key = make_pair(gx, gy);
            auto it = cellOwner.find(key);
            if (it != cellOwner.end()) {
                quitf(_wa, "Traces %d and %d both occupy cell (%lld,%lld)",
                      it->second, i+1, gx, gy);
            }
            cellOwner[key] = i+1;
            globalYmin = min(globalYmin, gy);
            globalYmax = max(globalYmax, gy);
        }
    }

    // ---------- compute H ----------
    if (globalYmax < globalYmin) {
        // Should not happen if n>=1 and all traces non-empty
        quitf(_wa, "No cells placed; impossible for n>=1");
    }
    long long H = globalYmax - globalYmin + 1;

    // ---------- compute score ratio ----------
    // score = 1000 * clamp(H_base / H, 0, 2)
    // We map [0,2] -> [0,1] for quitp: score_ratio = clamp(H_base/H, 0, 2) / 2
    double ratio_val = (double)H_base / (double)H;
    if (ratio_val < 0.0) ratio_val = 0.0;
    if (ratio_val > 2.0) ratio_val = 2.0;
    double score_ratio = ratio_val / 2.0; // in [0, 1]

    quitp(score_ratio,
          "OK H=%lld H_base=%lld score=%.2f | Ratio: %.9f",
          H, H_base, 1000.0 * ratio_val, score_ratio);

    return 0;
}