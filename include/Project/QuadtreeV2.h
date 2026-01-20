//
// Quadtree implementation with configurable capacity and AABB-based storage
//

#ifndef PROJECT_QUADTREE_V2_H
#define PROJECT_QUADTREE_V2_H

#include <vector>
#include <memory>
#include <algorithm>

// Axis-Aligned Bounding Box (AABB)
struct AABB {
    double x, y;             // center
    double halfWidth, halfHeight;  // half dimensions

    // Check if this AABB contains a point
    bool contains(double px, double py) const {
        return (px >= x - halfWidth &&
                px <= x + halfWidth &&
                py >= y - halfHeight &&
                py <= y + halfHeight);
    }

    // Check if this AABB intersects with another AABB
    bool intersects(const AABB& other) const {
        return !(other.x - other.halfWidth > x + halfWidth ||
                 other.x + other.halfWidth < x - halfWidth ||
                 other.y - other.halfHeight > y + halfHeight ||
                 other.y + other.halfHeight < y - halfHeight);
    }
};

template<typename T>
struct QuadtreeItem {
    T data;
    AABB bounds;
    
    QuadtreeItem(const T& data, const AABB& bounds) 
        : data(data), bounds(bounds) {}
};

template<typename T>
class QuadTree {
private:
    AABB boundary;
    size_t capacity;
    std::vector<QuadtreeItem<T>> items;
    bool divided = false;
    
    std::unique_ptr<QuadTree> nw;
    std::unique_ptr<QuadTree> ne;
    std::unique_ptr<QuadTree> sw;
    std::unique_ptr<QuadTree> se;
    
    void subdivide();

public:

    QuadTree(const AABB& boundary, size_t capacity = 4);
    
    ~QuadTree() = default;
    
    // Prevent copying
    QuadTree(const QuadTree&) = delete;
    QuadTree& operator=(const QuadTree&) = delete;
    
    // Allow moving
    QuadTree(QuadTree&&) = default;
    QuadTree& operator=(QuadTree&&) = default;

    bool insert(const T& data, const AABB& bounds);

    std::vector<T> queryRange(const AABB& range) const;

    void clear();
    
    // Getters for visualization/debugging
    const AABB& getBoundary() const { return boundary; }
    const std::vector<QuadtreeItem<T>>& getItems() const { return items; }
    bool isDivided() const { return divided; }
    const QuadTree* getNW() const { return nw.get(); }
    const QuadTree* getNE() const { return ne.get(); }
    const QuadTree* getSW() const { return sw.get(); }
    const QuadTree* getSE() const { return se.get(); }
};

// Implementation of template methods

template<typename T>
QuadTree<T>::QuadTree(const AABB& boundary, size_t capacity)
    : boundary(boundary), capacity(capacity) {}

template<typename T>
void QuadTree<T>::subdivide() {
    double x = boundary.x;
    double y = boundary.y;
    double hw = boundary.halfWidth / 2.0;
    double hh = boundary.halfHeight / 2.0;

    // Create four children that divide the current boundary into four quadrants
    nw = std::make_unique<QuadTree>(AABB{x - hw/2, y - hh/2, hw, hh}, capacity);
    ne = std::make_unique<QuadTree>(AABB{x + hw/2, y - hh/2, hw, hh}, capacity);
    sw = std::make_unique<QuadTree>(AABB{x - hw/2, y + hh/2, hw, hh}, capacity);
    se = std::make_unique<QuadTree>(AABB{x + hw/2, y + hh/2, hw, hh}, capacity);
    
    divided = true;
}

template<typename T>
bool QuadTree<T>::insert(const T& data, const AABB& bounds) {
    // If the item's bounds don't intersect this node's boundary, return false
    if (!boundary.intersects(bounds)) {
        return false;
    }

    // If we have space and we're not divided, add the item to this node
    if (items.size() < capacity && !divided) {
        items.emplace_back(data, bounds);
        return true;
    }

    // If we're not already divided, subdivide now
    if (!divided) {
        subdivide();
        
        // Redistribute all items to children
        for (const auto& item : items) {
            nw->insert(item.data, item.bounds) ||
            ne->insert(item.data, item.bounds) ||
            sw->insert(item.data, item.bounds) ||
            se->insert(item.data, item.bounds);
        }
        items.clear();
    }

    // Insert the new item into the appropriate child(ren)
    bool inserted = nw->insert(data, bounds) ||
                   ne->insert(data, bounds) ||
                   sw->insert(data, bounds) ||
                   se->insert(data, bounds);
                   
    return inserted;
}

template<typename T>
std::vector<T> QuadTree<T>::queryRange(const AABB& range) const {
    std::vector<T> results;
    
    // If the range doesn't intersect this node's boundary, return empty results
    if (!boundary.intersects(range)) {
        return results;
    }
    
    // Check items in this node
    for (const auto& item : items) {
        if (range.intersects(item.bounds)) {
            results.push_back(item.data);
        }
    }
    
    // If this node is divided, query all children
    if (divided) {
        auto nwResults = nw->queryRange(range);
        auto neResults = ne->queryRange(range);
        auto swResults = sw->queryRange(range);
        auto seResults = se->queryRange(range);
        
        // Combine results
        results.reserve(results.size() + 
                       nwResults.size() + neResults.size() + 
                       swResults.size() + seResults.size());
        
        results.insert(results.end(), nwResults.begin(), nwResults.end());
        results.insert(results.end(), neResults.begin(), neResults.end());
        results.insert(results.end(), swResults.begin(), swResults.end());
        results.insert(results.end(), seResults.begin(), seResults.end());
    }
    
    return results;
}

template<typename T>
void QuadTree<T>::clear() {
    items.clear();
    if (divided) {
        nw.reset();
        ne.reset();
        sw.reset();
        se.reset();
        divided = false;
    }
}

#endif // PROJECT_QUADTREE_V2_H
