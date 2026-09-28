#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char* argv[]) {
    registerTestlibCmd(argc, argv);

    // ---- read problem input ----
    int m = inf.readInt();

    struct Ticket {
        int n;
        long long A, B, C, H;
        string s;
    };
    vector<Ticket> tickets(m);

    for (int i = 0; i < m; i++) {
        tickets[i].n = inf.readInt();
        tickets[i].A = inf.readLong();
        tickets[i].B = inf.readLong();
        tickets[i].C = inf.readLong();
        tickets[i].H = inf.readLong();
        tickets[i].s = inf.readToken();
        if ((int)tickets[i].s.size() != tickets[i].n) {
            quitf(_fail, "Input ticket %d has wrong length", i+1);
        }
    }

    // sticker counts
    long long stickers[10];
    for (int d = 0; d < 10; d++) stickers[d] = inf.readLong();

    // ---- compute baseline ----
    // Copy sticker counts for baseline
    long long base_stickers[10];
    for (int d = 0; d < 10; d++) base_stickers[d] = stickers[d];

    vector<string> base_restored(m);
    for (int i = 0; i < m; i++) {
        base_restored[i] = tickets[i].s;
        for (int j = 0; j < tickets[i].n; j++) {
            if (base_restored[i][j] == '?') {
                // smallest d with remaining stickers
                int chosen = -1;
                for (int d = 0; d <= 9; d++) {
                    if (base_stickers[d] > 0) { chosen = d; break; }
                }
                if (chosen == -1) quitf(_fail, "Baseline ran out of stickers");
                base_restored[i][j] = '0' + chosen;
                base_stickers[chosen]--;
            }
        }
    }

    // compute appeal helper
    auto compute_appeal = [](const Ticket& tk, const string& t) -> long long {
        int half = tk.n / 2;
        long long L = 0, R = 0;
        for (int j = 0; j < half; j++) L += (t[j] - '0');
        for (int j = half; j < tk.n; j++) R += (t[j] - '0');
        long long D = abs(L - R);
        long long E = 0;
        for (int j = 0; j + 1 < tk.n; j++) {
            if (t[j] == t[j+1]) E++;
        }
        long long val = 0;
        if (D == 0) val += tk.A;
        long long nd = tk.H - D;
        if (nd > 0) val += tk.B * nd;
        val += tk.C * E;
        return val;
    };

    long long obj_base = 0;
    for (int i = 0; i < m; i++) {
        obj_base += compute_appeal(tickets[i], base_restored[i]);
    }

    // ---- read and verify participant output ----
    // Track sticker usage
    long long used[10] = {};

    vector<string> part_restored(m);
    for (int i = 0; i < m; i++) {
        string t = ouf.readToken();
        if ((int)t.size() != tickets[i].n) {
            quitf(_wa, "Ticket %d: expected length %d, got %d", i+1, tickets[i].n, (int)t.size());
        }
        for (int j = 0; j < tickets[i].n; j++) {
            if (t[j] < '0' || t[j] > '9') {
                quitf(_wa, "Ticket %d: non-digit character at position %d", i+1, j+1);
            }
            if (tickets[i].s[j] != '?') {
                if (t[j] != tickets[i].s[j]) {
                    quitf(_wa, "Ticket %d: position %d should be '%c' but got '%c'",
                          i+1, j+1, tickets[i].s[j], t[j]);
                }
            } else {
                used[t[j] - '0']++;
            }
        }
        part_restored[i] = t;
    }

    // check EOF
    if (!ouf.seekEof()) {
        quitf(_wa, "Extra output after the last ticket");
    }

    // check sticker counts
    for (int d = 0; d <= 9; d++) {
        if (used[d] != stickers[d]) {
            quitf(_wa, "Sticker count mismatch for digit %d: needed %lld, used %lld",
                  d, stickers[d], used[d]);
        }
    }

    // ---- compute participant objective ----
    long long obj_you = 0;
    for (int i = 0; i < m; i++) {
        obj_you += compute_appeal(tickets[i], part_restored[i]);
    }

    // ---- scoring ----
    // score = 1000 * min(3, (obj_you + 1) / (obj_base + 1))
    // normalized ratio for quitp in [0,1]: divide by 3000
    double ratio = (double)(obj_you + 1) / (double)(obj_base + 1);
    double clamped_ratio = min(3.0, ratio);
    double score = 1000.0 * clamped_ratio; // 0..3000
    double qp = score / 3000.0;            // normalize to [0,1]
    if (qp < 0.0) qp = 0.0;
    if (qp > 1.0) qp = 1.0;

    quitp(qp,
          "Feasible. OBJ_you=%lld OBJ_base=%lld raw_score=%.4f Ratio: %.6f",
          obj_you, obj_base, score, qp);
}