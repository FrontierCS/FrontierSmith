#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // Read problem input
    int H = inf.readInt();
    int W = inf.readInt();
    int K = inf.readInt();

    vector<vector<int>> target(H, vector<int>(W, 0));
    long long NZ = 0;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            target[y][x] = inf.readInt();
            if (target[y][x] != 0) NZ++;
        }
    }

    // Baseline cost: paint each non-zero cell individually (1x1, cost 11 each)
    long long B = 11LL * NZ;

    // Read participant output
    int q = ouf.readInt(0, 200000, "q");

    // Simulate on grid initialised to 0
    vector<vector<int>> grid(H, vector<int>(W, 0));
    long long C = 0;

    for (int op = 1; op <= q; op++) {
        int x1 = ouf.readInt();
        int y1 = ouf.readInt();
        int x2 = ouf.readInt();
        int y2 = ouf.readInt();
        int c  = ouf.readInt();

        // Bounds checks
        if (x1 < 0 || x2 > W || x1 >= x2) {
            quitf(_wa,
                  "Operation %d: invalid x range x1=%d x2=%d (W=%d)",
                  op, x1, x2, W);
        }
        if (y1 < 0 || y2 > H || y1 >= y2) {
            quitf(_wa,
                  "Operation %d: invalid y range y1=%d y2=%d (H=%d)",
                  op, y1, y2, H);
        }
        if (c < 0 || c > K) {
            quitf(_wa,
                  "Operation %d: colour c=%d out of range [0,%d]",
                  op, c, K);
        }

        // Monochrome check
        int baseColour = grid[y1][x1];
        bool mono = true;
        for (int y = y1; y < y2 && mono; y++) {
            for (int x = x1; x < x2 && mono; x++) {
                if (grid[y][x] != baseColour) {
                    mono = false;
                }
            }
        }
        if (!mono) {
            quitf(_wa,
                  "Operation %d: rectangle x=[%d,%d) y=[%d,%d) is not monochrome",
                  op, x1, x2, y1, y2);
        }

        // Apply paint
        for (int y = y1; y < y2; y++) {
            for (int x = x1; x < x2; x++) {
                grid[y][x] = c;
            }
        }

        long long area = (long long)(x2 - x1) * (long long)(y2 - y1);
        C += 10LL + area;
    }

    // Final grid must match target
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            if (grid[y][x] != target[y][x]) {
                quitf(_wa,
                      "Final mural mismatch at (x=%d,y=%d): got %d expected %d",
                      x, y, grid[y][x], target[y][x]);
            }
        }
    }

    // No trailing tokens allowed
    if (!ouf.seekEof()) {
        quitf(_wa, "Trailing tokens found after the last operation");
    }

    // Scoring per statement:
    //   per-test score = round(1,000,000 * clamp(B / C, 0, 2))
    //   max per-test   = 2,000,000
    //
    // quitp expects a ratio in [0, 1] where 1.0 == full marks for this test.
    // Full marks = 2,000,000, so:
    //   ratio = clamp(B/C, 0, 2) / 2
    //
    // The "Ratio:" tag in the message must equal this [0,1] value so the
    // judge can parse it correctly.

    double bc;
    if (C == 0) {
        // C==0 only if q==0; problem guarantees NZ>=1 so this is infeasible.
        // Treat as zero score (should not normally be reached after target check above).
        bc = 0.0;
    } else {
        bc = (double)B / (double)C;
    }
    if (bc < 0.0) bc = 0.0;
    if (bc > 2.0) bc = 2.0;

    // ratio in [0,1]: this is what quitp uses and what "Ratio:" tag reports
    double ratio = bc / 2.0;

    // Compute the actual integer score as the statement describes, for display
    long long displayScore = llround(1000000.0 * bc);

    quitp(ratio,
          "Ratio: %.9f, score=%lld/2000000, B=%lld, C=%lld, q=%d",
          ratio, displayScore, B, C, q);

    return 0;
}