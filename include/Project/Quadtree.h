//
// Created by senne on 19/11/2025.
//

#ifndef PROJECT_QUADTREE_H
#define PROJECT_QUADTREE_H

#include <vector>

struct Point {
    double x, y;
};

struct Rect {
    double x, y;      // center
    double halfWidth, halfHeight;

    bool contains(const Point& p) const;
    bool intersects(const Rect& other) const;
};

class QuadTree {
private:
    static const int CAPACITY = 4;

    Rect boundary;
    std::vector<Point> points;
    bool divided;
    QuadTree* nw;
    QuadTree* ne;
    QuadTree* sw;
    QuadTree* se;
    
    void subdivide();

public:
    QuadTree(const Rect& boundary);
    ~QuadTree();

    bool insert(const Point& p);
    void print(int level = 0) const;
    std::vector<Point> queryRange(const Rect& range) const;
};

#endif //PROJECT_QUADTREE_H
