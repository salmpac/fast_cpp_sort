#include <cassert>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <bitset>
#include <iomanip>
#include <queue>
#include <cmath>
#include <ctime>
#include <numeric>
#include <string>
#include <chrono>
#include <random>
#include <stdint.h>
#include <complex>
#include <array>
#include <sqlite3.h>
#include <unistd.h>
#include <thread>

#include "Sorter.h"

using namespace std;

typedef long long ll;
typedef long double ld;
#define all(x) x.begin(), x.end()
const ll MOD2 = 1000000007;
const ll MOD = 998244353;
const ll inf = 1e18;
const ld eps = 1e-8;
double PI = asin(0.5) * (ld)6;

//std::mt19937 rnd(chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());
mt19937 rnd(52);

template<typename X>
void check_sorted(vector<X>& x) {
    for (size_t i = 0; i + 1 < x.size(); ++i) {
        assert(x[i] <= x[i + 1]);
    }
}

void ff() {
    int n = 1e8;
    vector<double> x(n);
    for (int i = 0; i < n; ++i) {
        x[i] = (double)rnd() / 11.0;
    }
    vector<double> xcp = x;
    chrono::milliseconds start_time = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch());
    sort(x.begin(), x.end());
    chrono::milliseconds end_time = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch());
    cout << "sort time: " << (end_time - start_time).count() << '\n';
    check_sorted(x);

    start_time = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch());
    Sorter<double>::sort(xcp, 7);
    end_time = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch());
    cout << "m_sort time: " << (end_time - start_time).count() << '\n';
    check_sorted(xcp);
}

void solve() {
    ff();
}

signed main(void) {
    //freopen(t.c_str(), "r", stdin);
    cout << fixed << setprecision(20);
    //ios_base::sync_with_stdio(0);
    //cin.tie(0); cout.tie(0);
    int t = 1; srand(time(0)); //cin >> t;
    for (int i = 0; i < t; ++i) {
        //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        solve();
        //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        //std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << std::endl;
    }
    return 0;
}