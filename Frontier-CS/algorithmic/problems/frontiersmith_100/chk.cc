#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char** argv) {
    registerTestlibCmd(argc, argv);
    int N=inf.readInt(), M=inf.readInt();
    long long F=inf.readLong(), P0=inf.readLong(), P1=inf.readLong();
    vector<string> A(N);
    long long ones=0;
    for(auto& row:A) {row=inf.readToken(); ones+=count(row.begin(),row.end(),'1');}
    // A guaranteed feasible baseline: paint nothing.
    long long B=P1*ones;
    int K=ouf.readInt(0,200000,"K");
    // Row interval endpoints propagate vertically or on one of two diagonals.
    // Three difference grids give O(N*M + K) time even for repeated giant stencils.
    int stride=M+2;
    vector<int> vertical((N+1)*stride), downRight((N+1)*stride), downLeft((N+1)*stride);
    auto event=[&](vector<int>& g,int r,int c,int delta) {
        if(r<=N && c>=0 && c<=M) g[r*stride+c]+=delta;
    };
    auto segment=[&](vector<int>& g,int r,int c,int h,int dc,int delta) {
        event(g,r,c,delta); event(g,r+h,c+dc*h,-delta);
    };
    for(int z=0;z<K;++z) {
        string t=ouf.readToken();
        int i=ouf.readInt(1,N,"i")-1, j=ouf.readInt(1,M,"j")-1;
        int h=ouf.readInt(1,min(N,M),"h");
        if(i+h>N || j+h>M) quitf(_wa,"stencil outside image");
        if(t=="TR") {
            segment(downRight,i,j,h,1,1); segment(vertical,i,j+h,h,0,-1);
        } else if(t=="BL") {
            segment(vertical,i,j,h,0,1); segment(downRight,i,j+1,h,1,-1);
        } else if(t=="TL") {
            segment(vertical,i,j,h,0,1); segment(downLeft,i,j+h,h,-1,-1);
        } else if(t=="BR") {
            segment(downLeft,i,j+h-1,h,-1,1); segment(vertical,i,j+h,h,0,-1);
        } else quitf(_wa,"unknown stencil type");
    }
    if(!ouf.seekEof()) quitf(_wa,"extra output");
    long long FP=0,FN=0;
    for(int r=0;r<N;++r) {
        int cover=0;
        for(int c=0;c<=M;++c) {
            int p=r*stride+c;
            if(r) {
                vertical[p]+=vertical[p-stride];
                if(c) downRight[p]+=downRight[p-stride-1];
                if(c<M) downLeft[p]+=downLeft[p-stride+1];
            }
            cover+=vertical[p]+downRight[p]+downLeft[p];
            if(c<M) {FP+=(cover>0 && A[r][c]=='0'); FN+=(cover==0 && A[r][c]=='1');}
        }
    }
    long long C=F*K+P0*FP+P1*FN;
    double ratio=max(0.0L,((long double)B-C)/((long double)B+1));
    quitp(ratio,"C=%lld B=%lld FP=%lld FN=%lld Ratio: %.12f",C,B,FP,FN,ratio);
}
