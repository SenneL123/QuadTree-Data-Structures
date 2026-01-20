//
// Created by senne on 19/11/2025.
//

#ifndef PROJECT_QUADTREE_H
#define PROJECT_QUADTREE_H

#include <vector>
#include <unordered_set>
#include "Point.h"

struct Rect {
    double x, y;      // center
    double halfWidth, halfHeight;

    bool contains(const Point& p) const {
        return (p.x >= x - halfWidth && 
                p.x <= x + halfWidth && 
                p.y >= y - halfHeight && 
                p.y <= y + halfHeight);
    }

    bool intersects(const Rect& other) const {
        return !(other.x - other.halfWidth > x + halfWidth ||
                 other.x + other.halfWidth < x - halfWidth ||
                 other.y - other.halfHeight > y + halfHeight ||
                 other.y + other.halfHeight < y - halfHeight);
    }
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
    std::unordered_set<Point> queryRange(const Rect& range) const;
    
    // Getter for boundary (needed for rendering)
    const Rect& getBoundary() const { return boundary; }
    // Getter for points (needed for rendering)
    const std::vector<Point>& getPoints() const { return points; }
    // Check if node is divided (needed for rendering)
    bool isDivided() const { return divided; }
    // Get child nodes (needed for rendering)
    QuadTree* getNW() const { return nw; }
    QuadTree* getNE() const { return ne; }
    QuadTree* getSW() const { return sw; }
    QuadTree* getSE() const { return se; }
    
    // Clear the quadtree
    void clear();
};

#endif //PROJECT_QUADTREE_H
