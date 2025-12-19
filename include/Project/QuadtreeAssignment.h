#ifndef PROJECT_QUADTREE_ASSIGNMENT_H
#define PROJECT_QUADTREE_ASSIGNMENT_H

#include <vector>
#include <memory>
#include <stdexcept>

/**
 * @brief A quadtree implementation for spatial partitioning with configurable capacity.
 * 
 * @tparam T The type of data to store in the quadtree
 */
template <typename T>
class Quadtree {
public:
    /**
     * @brief Construct a new Quadtree with the given bounds and capacity
     * 
     * @param x_min Minimum x-coordinate of the quadtree bounds
     * @param y_min Minimum y-coordinate of the quadtree bounds
     * @param x_max Maximum x-coordinate of the quadtree bounds
     * @param y_max Maximum y-coordinate of the quadtree bounds
     * @param region_capacity Maximum number of elements in a region before splitting
     * @throws std::invalid_argument if bounds are invalid or capacity is zero
     */
    Quadtree(double x_min, double y_min, double x_max, double y_max, size_t region_capacity = 4);
    
    /**
     * @brief Destroy the Quadtree object
     */
    ~Quadtree() = default;
    
    // Prevent copying
    Quadtree(const Quadtree&) = delete;
    Quadtree& operator=(const Quadtree&) = delete;
    
    // Allow moving
    Quadtree(Quadtree&&) = default;
    Quadtree& operator=(Quadtree&&) = default;
    
    /**
     * @brief Insert an element with the given bounds into the quadtree
     * 
     * @param element The element to insert
     * @param x_min Minimum x-coordinate of the element's bounds
     * @param y_min Minimum y-coordinate of the element's bounds
     * @param x_max Maximum x-coordinate of the element's bounds
     * @param y_max Maximum y-coordinate of the element's bounds
     * @return true if the element was inserted, false if it's outside the quadtree bounds
     */
    bool insert(const T& element, double x_min, double y_min, double x_max, double y_max);
    
    /**
     * @brief Query the quadtree for elements that could intersect with the given range
     * 
     * @param x_min Minimum x-coordinate of the query range
     * @param y_min Minimum y-coordinate of the query range
     * @param x_max Maximum x-coordinate of the query range
     * @param y_max Maximum y-coordinate of the query range
     * @return std::vector<T> Vector of elements that could intersect with the range
     */
    std::vector<T> query_range(double x_min, double y_min, double x_max, double y_max) const;
    
    /**
     * @brief Clear all elements from the quadtree
     */
    void clear();
    
    /**
     * @brief Get the number of elements in the quadtree
     * 
     * @return size_t Number of elements
     */
    size_t size() const;
    
    /**
     * @brief Check if the quadtree is empty
     * 
     * @return true if the quadtree is empty, false otherwise
     */
    bool empty() const;
    
private:
    struct AABB {
        double x_min, y_min, x_max, y_max;
        
        bool intersects(const AABB& other) const {
            return !(x_max < other.x_min || x_min > other.x_max ||
                    y_max < other.y_min || y_min > other.y_max);
        }
        
        bool contains(const AABB& other) const {
            return (other.x_min >= x_min && other.x_max <= x_max &&
                    other.y_min >= y_min && other.y_max <= y_max);
        }
        
        bool contains_point(double x, double y) const {
            return (x >= x_min && x <= x_max && y >= y_min && y <= y_max);
        }
    };
    
    struct Node {
        AABB bounds;
        std::vector<std::pair<T, AABB>> elements;
        std::unique_ptr<Node> nw, ne, sw, se;
        
        Node(double x_min, double y_min, double x_max, double y_max)
            : bounds{x_min, y_min, x_max, y_max} {}
    };
    
    std::unique_ptr<Node> root_;
    size_t region_capacity_;
    size_t size_;
    
    void subdivide(Node* node);
    void query_range_recursive(const Node* node, const AABB& range, std::vector<T>& results) const;
    void get_all_elements(const Node* node, std::vector<std::pair<T, AABB>>& elements) const;
};

// Implementation
template <typename T>
Quadtree<T>::Quadtree(double x_min, double y_min, double x_max, double y_max, size_t region_capacity)
    : region_capacity_(region_capacity), size_(0) {
    
    if (x_min >= x_max || y_min >= y_max) {
        throw std::invalid_argument("Invalid bounds: min must be less than max");
    }
    
    if (region_capacity == 0) {
        throw std::invalid_argument("Region capacity must be greater than zero");
    }
    
    root_ = std::make_unique<Node>(x_min, y_min, x_max, y_max);
}

template <typename T>
bool Quadtree<T>::insert(const T& element, double x_min, double y_min, double x_max, double y_max) {
    AABB elem_bounds{x_min, y_min, x_max, y_max};
    
    // Check if element is within the quadtree bounds
    if (!root_->bounds.contains(elem_bounds)) {
        return false;
    }
    
    // Start insertion from the root
    Node* current = root_.get();
    
    while (true) {
        // If this node is a leaf and has capacity, add the element
        if (!current->nw && 
            (current->elements.size() < region_capacity_ || current == root_.get())) {
            current->elements.emplace_back(element, elem_bounds);
            size_++;
            
            // If we've reached capacity, subdivide if this is not the root
            if (current->elements.size() > region_capacity_ && current != root_.get()) {
                subdivide(current);
            }
            return true;
        }
        
        // If this node is already divided, find the appropriate child
        if (current->nw) {
            // Calculate midpoints
            double mid_x = (current->bounds.x_min + current->bounds.x_max) / 2.0;
            double mid_y = (current->bounds.y_min + current->bounds.y_max) / 2.0;
            
            // Check which quadrants the element belongs to
            bool in_nw = (x_min <= mid_x && y_min <= mid_y);
            bool in_ne = (x_max > mid_x && y_min <= mid_y);
            bool in_sw = (x_min <= mid_x && y_max > mid_y);
            bool in_se = (x_max > mid_x && y_max > mid_y);
            
            // If the element spans multiple quadrants, keep it in this node
            if ((in_nw && in_ne) || (in_sw && in_se) || 
                (in_nw && in_sw) || (in_ne && in_se)) {
                current->elements.emplace_back(element, elem_bounds);
                size_++;
                return true;
            }
            
            // Otherwise, move to the appropriate child
            if (in_nw) current = current->nw.get();
            else if (in_ne) current = current->ne.get();
            else if (in_sw) current = current->sw.get();
            else current = current->se.get();
        } else {
            // This node needs to be subdivided
            subdivide(current);
        }
    }
}

template <typename T>
void Quadtree<T>::subdivide(Node* node) {
    if (node->nw) return;  // Already subdivided
    
    double mid_x = (node->bounds.x_min + node->bounds.x_max) / 2.0;
    double mid_y = (node->bounds.y_min + node->bounds.y_max) / 2.0;
    
    // Create four children
    node->nw = std::make_unique<Node>(node->bounds.x_min, node->bounds.y_min, mid_x, mid_y);
    node->ne = std::make_unique<Node>(mid_x, node->bounds.y_min, node->bounds.x_max, mid_y);
    node->sw = std::make_unique<Node>(node->bounds.x_min, mid_y, mid_x, node->bounds.y_max);
    node->se = std::make_unique<Node>(mid_x, mid_y, node->bounds.x_max, node->bounds.y_max);
    
    // Redistribute elements to children
    std::vector<std::pair<T, AABB>> remaining_elements;
    
    for (const auto& elem : node->elements) {
        const AABB& bounds = elem.second;
        bool placed = false;
        
        // Check which quadrants the element belongs to
        bool in_nw = (bounds.x_min <= mid_x && bounds.y_min <= mid_y);
        bool in_ne = (bounds.x_max > mid_x && bounds.y_min <= mid_y);
        bool in_sw = (bounds.x_min <= mid_x && bounds.y_max > mid_y);
        bool in_se = (bounds.x_max > mid_x && bounds.y_max > mid_y);
        
        // If the element fits entirely in one quadrant, move it there
        if (in_nw && !in_ne && !in_sw && !in_se) {
            node->nw->elements.push_back(elem);
            placed = true;
        } else if (!in_nw && in_ne && !in_sw && !in_se) {
            node->ne->elements.push_back(elem);
            placed = true;
        } else if (!in_nw && !in_ne && in_sw && !in_se) {
            node->sw->elements.push_back(elem);
            placed = true;
        } else if (!in_nw && !in_ne && !in_sw && in_se) {
            node->se->elements.push_back(elem);
            placed = true;
        }
        
        if (!placed) {
            remaining_elements.push_back(elem);
        }
    }
    
    // Keep elements that span multiple quadrants in this node
    node->elements = std::move(remaining_elements);
}

template <typename T>
std::vector<T> Quadtree<T>::query_range(double x_min, double y_min, double x_max, double y_max) const {
    std::vector<T> results;
    AABB range{x_min, y_min, x_max, y_max};
    query_range_recursive(root_.get(), range, results);
    return results;
}

template <typename T>
void Quadtree<T>::query_range_recursive(const Node* node, const AABB& range, std::vector<T>& results) const {
    if (!node) return;
    
    // Check if the query range intersects this node's bounds
    if (!node->bounds.intersects(range)) {
        return;
    }
    
    // Check elements in this node
    for (const auto& elem : node->elements) {
        if (range.intersects(elem.second)) {
            results.push_back(elem.first);
        }
    }
    
    // Recursively check children
    if (node->nw) {
        query_range_recursive(node->nw.get(), range, results);
        query_range_recursive(node->ne.get(), range, results);
        query_range_recursive(node->sw.get(), range, results);
        query_range_recursive(node->se.get(), range, results);
    }
}

template <typename T>
void Quadtree<T>::clear() {
    root_ = std::make_unique<Node>(root_->bounds.x_min, root_->bounds.y_min,
                                  root_->bounds.x_max, root_->bounds.y_max);
    size_ = 0;
}

template <typename T>
size_t Quadtree<T>::size() const {
    return size_;
}

template <typename T>
bool Quadtree<T>::empty() const {
    return size_ == 0;
}

template <typename T>
void Quadtree<T>::get_all_elements(const Node* node, std::vector<std::pair<T, AABB>>& elements) const {
    if (!node) return;
    
    elements.insert(elements.end(), node->elements.begin(), node->elements.end());
    
    if (node->nw) {
        get_all_elements(node->nw.get(), elements);
        get_all_elements(node->ne.get(), elements);
        get_all_elements(node->sw.get(), elements);
        get_all_elements(node->se.get(), elements);
    }
}

#endif // PROJECT_QUADTREE_ASSIGNMENT_H
