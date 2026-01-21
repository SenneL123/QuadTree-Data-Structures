#ifndef PROJECT_AXIS_ALIGNED_BOUNDING_BOX_H
#define PROJECT_AXIS_ALIGNED_BOUNDING_BOX_H

#include <stdexcept>

namespace Project {

class AxisAlignedBoundingBox {
    double minX, minY, maxX, maxY;

public:
    AxisAlignedBoundingBox(double minX, double minY, double maxX, double maxY)
        : minX(minX), minY(minY), maxX(maxX), maxY(maxY) {
        if (minX > maxX || minY > maxY) {
            throw std::invalid_argument("Invalid bounding box coordinates");
        }
    }

    double getMinX() const { return minX; }
    double getMinY() const { return minY; }
    double getMaxX() const { return maxX; }
    double getMaxY() const { return maxY; }

    double getWidth() const { return maxX - minX; }
    double getHeight() const { return maxY - minY; }
    double getCenterX() const { return (minX + maxX) / 2.0; }
    double getCenterY() const { return (minY + maxY) / 2.0; }

    bool contains(double x, double y) const {
        return x >= minX && x <= maxX && y >= minY && y <= maxY;
    }

    bool intersects(const AxisAlignedBoundingBox& other) const {
        return !(other.maxX < minX || 
                other.minX > maxX || 
                other.maxY < minY || 
                other.minY > maxY);
    }

    friend bool collides(const AxisAlignedBoundingBox& one, const AxisAlignedBoundingBox& two) {
        return one.intersects(two);
    }
};

} // namespace Project

#endif //PROJECT_AXIS_ALIGNED_BOUNDING_BOX_H
