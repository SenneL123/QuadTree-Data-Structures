#ifndef PROJECT_POINT_H
#define PROJECT_POINT_H

#include <functional>
#include <iostream>

struct Point {
    double x, y;

    // Default constructor
    Point() : x(0), y(0) {}
    
    // Constructor with parameters
    Point(double x, double y) : x(x), y(y) {}

    // Equality comparison
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    // Inequality comparison (good practice to have both)
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

// Hash function for Point
namespace std {
    template<>
    struct hash<Point> {
        size_t operator()(const Point& p) const {
            size_t h1 = hash<double>()(p.x);
            size_t h2 = hash<double>()(p.y);
            return h1 ^ (h2 << 1);
        }
    };
}

#endif //PROJECT_POINT_H