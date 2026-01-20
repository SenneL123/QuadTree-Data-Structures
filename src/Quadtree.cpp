//
// Created by senne on 19/11/2025.
//
#include "../include/Project/Quadtree.h"
#include <iostream>
#include <string>


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

std::unordered_set<Point> QuadTree::queryRange(const Rect& range) const {
    std::unordered_set<Point> pointsInRange;

    // If this node's boundary doesn't intersect with the query range, return empty set
    if (!boundary.intersects(range)) {
        return pointsInRange;
    }

    // Check points in this node
    for (const auto& p : points) {
        if (range.contains(p)) {
            pointsInRange.insert(p);
        }
    }

    // If this node has children, query them as well
    if (divided) {
        // Query each child and merge results
        auto mergeResults = [&pointsInRange](const std::unordered_set<Point>& childPoints) {
            pointsInRange.insert(childPoints.begin(), childPoints.end());
        };

        mergeResults(nw->queryRange(range));
        mergeResults(ne->queryRange(range));
        mergeResults(sw->queryRange(range));
        mergeResults(se->queryRange(range));
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

void QuadTree::clear() {
    // Clear all points in this node
    points.clear();
    
    // Recursively clear child nodes if they exist
    if (divided) {
        nw->clear();
        ne->clear();
        sw->clear();
        se->clear();
        
        // Delete child nodes
        delete nw; nw = nullptr;
        delete ne; ne = nullptr;
        delete sw; sw = nullptr;
        delete se; se = nullptr;
        
        divided = false;
    }
}
