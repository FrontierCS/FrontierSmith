#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // --- Read problem input ---
    int M = inf.readInt();
    int N = inf.readInt();

    vector<long long> A(M);
    for (int i = 0; i < M; i++)
        A[i] = inf.readLong();

    vector<vector<long long>> B(M, vector<long long>(M, 0LL));
    for (int i = 0; i < M; i++)
        for (int j = 0; j < M; j++)
            B[i][j] = inf.readLong();

    // --- Read and validate participant output ---
    vector<string> addr(M);
    for (int i = 0; i < M; i++) {
        // Use readToken which skips whitespace/newlines
        if (ouf.seekEof())
            quitf(_wa, "Expected address for item %d but got EOF", i + 1);
        string tok = ouf.readToken();
        if (tok == ".") {
            addr[i] = "";  // empty string = root address
        } else {
            // validate binary string
            for (char c : tok)
                if (c != '0' && c != '1')
                    quitf(_wa, "Address for item %d contains invalid character '%c'",
                          i + 1, c);
            if ((int)tok.size() > N)
                quitf(_wa, "Address for item %d has length %d which exceeds N=%d",
                      i + 1, (int)tok.size(), N);
            addr[i] = tok;
        }
    }

    // Check no trailing tokens (soft check: skip whitespace then test)
    if (!ouf.seekEof())
        quitf(_wa, "Extra output after %d addresses", M);

    // Check all addresses are distinct
    {
        set<string> seen;
        for (int i = 0; i < M; i++) {
            if (seen.count(addr[i]))
                quitf(_wa, "Duplicate address found for item %d", i + 1);
            seen.insert(addr[i]);
        }
    }

    // Check prefix-free:
    // Sort addresses; adjacent pairs in sorted order cover all prefix relationships.
    {
        // Build sorted index
        vector<string> sv = addr;
        sort(sv.begin(), sv.end());
        for (int i = 0; i + 1 < (int)sv.size(); i++) {
            const string& s1 = sv[i];
            const string& s2 = sv[i + 1];
            if (s2.size() >= s1.size() && s2.substr(0, s1.size()) == s1)
                quitf(_wa, "Addresses are not prefix-free: \"%s\" is a prefix of \"%s\"",
                      s1.c_str(), s2.c_str());
        }
    }

    // --- Compute participant cost C_you ---
    long long C_you = 0;
    for (int i = 0; i < M; i++)
        C_you += A[i] * (long long)addr[i].size();

    for (int i = 0; i < M; i++) {
        for (int j = i + 1; j < M; j++) {
            if (B[i][j] == 0) continue;
            int lcp_len = 0;
            int minlen = (int)min(addr[i].size(), addr[j].size());
            while (lcp_len < minlen && addr[i][lcp_len] == addr[j][lcp_len])
                lcp_len++;
            long long dist = (long long)addr[i].size()
                           + (long long)addr[j].size()
                           - 2LL * lcp_len;
            C_you += B[i][j] * dist;
        }
    }

    // --- Compute baseline cost C_base ---
    // L = ceil(log2(M)); if M == 1 then L = 0 and address is "."
    vector<string> base_addr(M);
    if (M == 1) {
        base_addr[0] = "";
    } else {
        int L = 0;
        while ((1LL << L) < (long long)M) L++;
        for (int i = 0; i < M; i++) {
            string s(L, '0');
            for (int b = 0; b < L; b++)
                s[L - 1 - b] = ((i >> b) & 1) ? '1' : '0';
            base_addr[i] = s;
        }
    }

    long long C_base = 0;
    for (int i = 0; i < M; i++)
        C_base += A[i] * (long long)base_addr[i].size();

    for (int i = 0; i < M; i++) {
        for (int j = i + 1; j < M; j++) {
            if (B[i][j] == 0) continue;
            int lcp_len = 0;
            int minlen = (int)min(base_addr[i].size(), base_addr[j].size());
            while (lcp_len < minlen && base_addr[i][lcp_len] == base_addr[j][lcp_len])
                lcp_len++;
            long long dist = (long long)base_addr[i].size()
                           + (long long)base_addr[j].size()
                           - 2LL * lcp_len;
            C_base += B[i][j] * dist;
        }
    }

    double ratio = max(0.0L, ((long double)C_base - C_you) /
                                  ((long double)C_base + 1.0L));
    quitp(ratio, "C=%lld B=%lld Ratio: %.12f", C_you, C_base, ratio);
}
