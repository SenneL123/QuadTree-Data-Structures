//
// Created by senne on 19/11/2025.
//
#include "../include/Project/Quadtree.h"
#include <iostream>
#include <string>

// Rect member functions
bool Rect::contains(const Point& p) const {
    return (p.x >= x - halfWidth &&
            p.x <= x + halfWidth &&
            p.y >= y - halfHeight &&
            p.y <= y + halfHeight);
}

bool Rect::intersects(const Rect& other) const {
    return !(other.x - other.halfWidth > x + halfWidth ||
             other.x + other.halfWidth < x - halfWidth ||
             other.y - other.halfHeight > y + halfHeight ||
             other.y + other.halfHeight < y - halfHeight);
}

// QuadTree member functions
QuadTree::QuadTree(const Rect& boundary)
    : boundary(boundary), divided(false), nw(nullptr), ne(nullptr), sw(nullptr), se(nullptr) {}

QuadTree::~QuadTree() {
    delete nw;
    delete ne;
    delete sw;
    delete se;
}

void QuadTree::subdivide() {
    double x = boundary.x;
    double y = boundary.y;
    double hw = boundary.halfWidth / 2;
    double hh = boundary.halfHeight / 2;

    nw = new QuadTree({x - hw, y - hh, hw, hh});
    ne = new QuadTree({x + hw, y - hh, hw, hh});
    sw = new QuadTree({x - hw, y + hh, hw, hh});
    se = new QuadTree({x + hw, y + hh, hw, hh});

    divided = true;
}

bool QuadTree::insert(const Point& p) {
    if (!boundary.contains(p)) {
        return false;
    }

    if (points.size() < CAPACITY) {
        points.push_back(p);
        return true;
    }

    if (!divided) {
        subdivide();
        // Redistribute points to children
        for (const auto& point : points) {
            nw->insert(point) || ne->insert(point) ||
            sw->insert(point) || se->insert(point);
        }
        points.clear();  // Clear points from this node
    }

    // Insert the new point
    return (nw->insert(p) || ne->insert(p) ||
            sw->insert(p) || se->insert(p));
}

std::vector<Point> QuadTree::queryRange(const Rect& range) const {
    std::vector<Point> pointsInRange;
    
    if (!boundary.intersects(range)) {
        return pointsInRange;
    }
    
    for (const auto& p : points) {
        if (range.contains(p)) {
            pointsInRange.push_back(p);
        }
    }
    
    if (divided) {
        auto nwPoints = nw->queryRange(range);
        auto nePoints = ne->queryRange(range);
        auto swPoints = sw->queryRange(range);
        auto sePoints = se->queryRange(range);
        
        pointsInRange.insert(pointsInRange.end(), nwPoints.begin(), nwPoints.end());
        pointsInRange.insert(pointsInRange.end(), nePoints.begin(), nePoints.end());
        pointsInRange.insert(pointsInRange.end(), swPoints.begin(), swPoints.end());
        pointsInRange.insert(pointsInRange.end(), sePoints.begin(), sePoints.end());
    }
    
    return pointsInRange;
}

void QuadTree::print(int level) const {
    std::string indent(level * 2, ' ');
    std::cout << indent << "Node: " << points.size() << " points\n";

    if (divided) {
        nw->print(level + 1);
        ne->print(level + 1);
        sw->print(level + 1);
        se->print(level + 1);
    }
}
