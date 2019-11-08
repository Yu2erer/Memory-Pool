#include <iostream>
#include <vector>
#include "YY_Allocator.h"

using namespace std;

int main() {
    const int n = 40;
    int *p[n];

    for (int i = 0; i < n; i ++) {
        p[i] = YY::Allocator<int>::allocate();
        *p[i] = i;
    }

    for (int i = 0; i < n; i ++) {
        cout << p[i] << ": " << *p[i] << endl;
        YY::Allocator<int>::deallocate(p[i]);
    }
    cout << endl;

    // Test Container
    vector<int, YY::Allocator<int>> vec;
    vec.push_back(3);
    vec.push_back(5);
    return 0;
}