#include <iostream>
#include "../include/Project/Quadtree.h"

int main() {
    Rect area{0, 0, 10, 10};
    QuadTree qt(area);

    qt.insert({1, 1});
    qt.insert({-3, 4});
    qt.insert({6, -2});
    qt.insert({7, 3});
    qt.insert({-8, -9});

    qt.print();

    return 0;
}
