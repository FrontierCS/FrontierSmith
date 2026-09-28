#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

static const double MY_PI = acos(-1.0);
static const double H_TRI = sqrt(3.0) / 2.0;

struct Pt { double x, y; };

static double crossPt(Pt O, Pt A, Pt B){
    return (A.x-O.x)*(B.y-O.y)-(A.y-O.y)*(B.x-O.x);
}

static vector<Pt> convexHull(vector<Pt> pts){
    int n = (int)pts.size();
    if(n < 1) return pts;
    sort(pts.begin(), pts.end(), [](const Pt& a, const Pt& b){
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });
    vector<Pt> u;
    for(auto& p : pts){
        if(!u.empty() && fabs(p.x-u.back().x)<1e-12 && fabs(p.y-u.back().y)<1e-12) continue;
        u.push_back(p);
    }
    pts = u;
    n = (int)pts.size();
    if(n < 2) return pts;
    vector<Pt> hull;
    hull.reserve(2*n);
    for(int i=0;i<n;i++){
        while((int)hull.size()>=2 && crossPt(hull[hull.size()-2],hull[hull.size()-1],pts[i])<=0)
            hull.pop_back();
        hull.push_back(pts[i]);
    }
    int lo = (int)hull.size()+1;
    for(int i=n-2;i>=0;i--){
        while((int)hull.size()>=lo && crossPt(hull[hull.size()-2],hull[hull.size()-1],pts[i])<=0)
            hull.pop_back();
        hull.push_back(pts[i]);
    }
    hull.pop_back();
    return hull;
}

static double hullPerimeter(vector<Pt>& pts){
    auto hull = convexHull(pts);
    int n = (int)hull.size();
    if(n<=1) return 0.0;
    double perim=0;
    for(int i=0;i<n;i++){
        int j=(i+1)%n;
        double dx=hull[j].x-hull[i].x, dy=hull[j].y-hull[i].y;
        perim+=sqrt(dx*dx+dy*dy);
    }
    return perim;
}

static void addPolygon(char type, int xi, int yi, char dir, vector<Pt>& pts){
    double x=xi, y=yi;
    if(type=='S'){
        pts.push_back({x,   y});
        pts.push_back({x+1, y});
        pts.push_back({x+1, y+1});
        pts.push_back({x,   y+1});
    } else if(type=='C'){
        for(int k=0;k<64;k++){
            double ang=2.0*MY_PI*k/64.0;
            pts.push_back({x+0.5+0.5*cos(ang), y+0.5+0.5*sin(ang)});
        }
    } else { // T
        if(dir=='U'){
            pts.push_back({x,       y});
            pts.push_back({x+1,     y});
            pts.push_back({x+0.5,   y+H_TRI});
        } else if(dir=='D'){
            pts.push_back({x,       y+1});
            pts.push_back({x+1,     y+1});
            pts.push_back({x+0.5,   y+1-H_TRI});
        } else if(dir=='L'){
            pts.push_back({x+1,     y});
            pts.push_back({x+1,     y+1});
            pts.push_back({x+1-H_TRI, y+0.5});
        } else { // R
            pts.push_back({x,       y});
            pts.push_back({x,       y+1});
            pts.push_back({x+H_TRI, y+0.5});
        }
    }
}

static Pt refPoint(char type, int xi, int yi, char dir){
    double x=xi, y=yi;
    double s3_6 = sqrt(3.0)/6.0;
    if(type=='S' || type=='C'){
        return {x+0.5, y+0.5};
    } else {
        if(dir=='U') return {x+0.5, y+s3_6};
        if(dir=='D') return {x+0.5, y+1.0-s3_6};
        if(dir=='L') return {x+1.0-s3_6, y+0.5};
        /* R */      return {x+s3_6, y+0.5};
    }
}

struct CableInfo { int u, v; long long w; };

static double computeObj(
    int n,
    long long lam,
    const string& types,
    const vector<CableInfo>& cables,
    const vector<pair<int,int>>& cells,
    const vector<char>& dirs)
{
    // Convex hull perimeter
    vector<Pt> allPts;
    allPts.reserve(n*64);
    for(int i=0;i<n;i++){
        addPolygon(types[i], cells[i].first, cells[i].second, dirs[i], allPts);
    }
    double P = hullPerimeter(allPts);

    // Cable cost
    double L = 0.0;
    for(auto& c : cables){
        int u = c.u-1, v = c.v-1;
        Pt pu = refPoint(types[u], cells[u].first, cells[u].second, dirs[u]);
        Pt pv = refPoint(types[v], cells[v].first, cells[v].second, dirs[v]);
        double dx=pu.x-pv.x, dy=pu.y-pv.y;
        L += c.w * sqrt(dx*dx+dy*dy);
    }

    return (double)lam * P + L;
}

int main(int argc, char* argv[]){
    registerTestlibCmd(argc, argv);

    // Read problem input
    int W = inf.readInt();
    int H = inf.readInt();
    int n = inf.readInt();
    int m = inf.readInt();
    int b = inf.readInt();
    long long lam = inf.readLong();

    string types = inf.readToken();
    if((int)types.size() != n)
        quitf(_fail, "types string length %d != n=%d", (int)types.size(), n);

    set<pair<int,int>> blockedSet;
    for(int i=0;i<b;i++){
        int xb = inf.readInt();
        int yb = inf.readInt();
        blockedSet.insert({xb,yb});
    }

    vector<CableInfo> cables(m);
    for(int i=0;i<m;i++){
        cables[i].u = inf.readInt();
        cables[i].v = inf.readInt();
        cables[i].w = inf.readLong();
    }

    // Read participant output
    vector<pair<int,int>> cells(n);
    vector<char>          dirs(n);
    set<pair<int,int>>    usedCells;

    static const string VALID_DIRS = "UDLR";
    for(int i=0;i<n;i++){
        int x = ouf.readInt();
        int y = ouf.readInt();
        string ds = ouf.readToken();
        if((int)ds.size()!=1 || VALID_DIRS.find(ds[0])==string::npos)
            quitf(_wa,"Exhibit %d: invalid direction '%s'", i+1, ds.c_str());
        char dir=ds[0];
        if(x<0||x>=W||y<0||y>=H)
            quitf(_wa,"Exhibit %d: cell (%d,%d) out of bounds", i+1,x,y);
        if(blockedSet.count({x,y}))
            quitf(_wa,"Exhibit %d: cell (%d,%d) is blocked",i+1,x,y);
        if(usedCells.count({x,y}))
            quitf(_wa,"Exhibit %d: cell (%d,%d) already used",i+1,x,y);
        usedCells.insert({x,y});
        cells[i]={x,y};
        dirs[i]=dir;
    }

    // Strict EOF check: no trailing tokens allowed
    if(!ouf.seekEof())
        quitf(_wa, "Extra output after %d exhibits", n);

    // Compute participant objective O
    double O = computeObj(n, lam, types, cables, cells, dirs);

    // Compute baseline: free cells in row-major order (x varies faster within row,
    // rows from y=0 upward), all dir='U'
    vector<pair<int,int>> freeCells;
    freeCells.reserve(W*H);
    for(int fy=0;fy<H;fy++)
        for(int fx=0;fx<W;fx++)
            if(!blockedSet.count({fx,fy}))
                freeCells.push_back({fx,fy});

    if((int)freeCells.size()<n)
        quitf(_fail,"Not enough free cells for baseline: need %d have %d",n,(int)freeCells.size());

    vector<pair<int,int>> baseCells(freeCells.begin(), freeCells.begin()+n);
    vector<char>          baseDirs(n,'U');
    double B = computeObj(n, lam, types, cables, baseCells, baseDirs);

    // score = 200 * B / (B + O)
    // We pass ratio = B/(B+O) to quitp; the subtask max is 200, so final = 200 * ratio
    double ratio;
    if(B + O < 1e-15){
        // Both zero: treat as baseline quality => ratio = 0.5 => score = 100
        ratio = 0.5;
    } else {
        ratio = B / (B + O);
    }
    // Clamp to [0,1]
    if(ratio < 0.0) ratio = 0.0;
    if(ratio > 1.0) ratio = 1.0;

    quitp(ratio, "Ratio: %.9f (B=%.6f, O=%.6f, lambda=%lld)", ratio, B, O, lam);
    return 0;
}