#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

// Compute total objective value for a given grid painting
long long computeObjective(
    int n, int m, int L,
    const vector<string>& routes,
    const vector<int>& reqA,
    const vector<int>& reqB,
    const vector<long long>& reqW,
    int q,
    const vector<string>& grid
) {
    int k = (int)routes.size();

    // Build barcode for each route
    vector<string> barcode(k);
    for (int i = 0; i < k; i++) {
        barcode[i].reserve(L);
        int r = 0, c = 0;
        barcode[i] += grid[r][c];
        for (char ch : routes[i]) {
            if (ch == 'R') c++;
            else           r++;
            barcode[i] += grid[r][c];
        }
    }

    long long total = 0;
    for (int i = 0; i < q; i++) {
        const string& sa = barcode[reqA[i]];
        const string& sb = barcode[reqB[i]];
        int t = -1;
        for (int j = 0; j < L; j++) {
            if (sa[j] != sb[j]) { t = j; break; }
        }
        if (t < 0) continue;
        if (sa[t] == '0' && sb[t] == '1') {
            total += reqW[i] * (long long)(L - t);
        }
    }
    return total;
}

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- Read problem input ----
    int n = inf.readInt();
    int m = inf.readInt();
    int k = inf.readInt();
    int q = inf.readInt();

    vector<string> routes(k);
    for (int i = 0; i < k; i++)
        routes[i] = inf.readToken();

    vector<int>       reqA(q), reqB(q);
    vector<long long> reqW(q);
    long long sumW = 0;
    for (int i = 0; i < q; i++) {
        reqA[i] = inf.readInt() - 1;
        reqB[i] = inf.readInt() - 1;
        reqW[i] = inf.readLong();
        sumW   += reqW[i];
    }

    int       L = n + m - 1;
    long long U = (long long)L * sumW;

    // ---- Read and validate participant output ----
    vector<string> grid(n);
    for (int i = 0; i < n; i++) {
        grid[i] = ouf.readToken();
        if ((int)grid[i].size() != m)
            quitf(_wa, "Row %d has length %d, expected %d", i + 1, (int)grid[i].size(), m);
        for (int j = 0; j < m; j++) {
            char c = grid[i][j];
            if (c != '0' && c != '1')
                quitf(_wa, "Row %d col %d has invalid character '%c'", i + 1, j + 1, c);
        }
    }
    // Ensure no trailing content
    if (!ouf.seekEof())
        quitf(_wa, "Extra content found after the %d required rows", n);

    // ---- Compute participant objective V ----
    long long V = computeObjective(n, m, L, routes, reqA, reqB, reqW, q, grid);

    // ---- Compute baseline B over 6 fixed paintings ----
    long long B = 0;

    // Helper lambda: evaluate a grid defined by a function cell(i,j)->char
    auto evalGrid = [&](auto cellFn) -> long long {
        vector<string> g(n, string(m, '0'));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++)
                g[i][j] = cellFn(i, j);
        return computeObjective(n, m, L, routes, reqA, reqB, reqW, q, g);
    };

    // 1. all 0
    B = max(B, evalGrid([](int, int) -> char { return '0'; }));
    // 2. all 1
    B = max(B, evalGrid([](int, int) -> char { return '1'; }));
    // 3. i mod 2
    B = max(B, evalGrid([](int i, int) -> char { return (char)('0' + i % 2); }));
    // 4. 1 - (i mod 2)
    B = max(B, evalGrid([](int i, int) -> char { return (char)('0' + 1 - i % 2); }));
    // 5. j mod 2
    B = max(B, evalGrid([](int, int j) -> char { return (char)('0' + j % 2); }));
    // 6. 1 - (j mod 2)
    B = max(B, evalGrid([](int, int j) -> char { return (char)('0' + 1 - j % 2); }));

    // ---- Compute normalised score ratio ----
    long long denom = max(1LL, U - B);
    double ratio = (double)(V - B) / (double)denom;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    quitp(ratio,
          "V=%lld B=%lld U=%lld denom=%lld Ratio: %.9f",
          V, B, U, denom, ratio);

    return 0;
}