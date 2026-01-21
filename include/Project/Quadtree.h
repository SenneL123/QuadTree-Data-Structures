#ifndef PROJECT_QUADTREE_H
#define PROJECT_QUADTREE_H

#include <vector>
#include <unordered_set>
#include <memory>
#include <stdexcept>
#include "AxisAlignedBoundingBox.h"

namespace Project {

template <typename MetadataType>
class Quadtree {
public:
    struct QuadTreeNode {
        AxisAlignedBoundingBox bounds;
        std::vector<std::pair<AxisAlignedBoundingBox, MetadataType>> objects;
        std::unique_ptr<QuadTreeNode> nw, ne, sw, se;
        bool is_divided = false;
        
        QuadTreeNode(const AxisAlignedBoundingBox& bounds) : bounds(bounds) {}
    };

private:
    std::unique_ptr<QuadTreeNode> root;
    unsigned int region_capacity;
    size_t total_objects = 0;
    
    // Type alias for the container used to store objects
    using Container = std::vector<std::pair<AxisAlignedBoundingBox, MetadataType>>;

    void subdivide(QuadTreeNode* node) {
        if (node->is_divided) return;
        
        double x = node->bounds.getCenterX();
        double y = node->bounds.getCenterY();
        double halfWidth = node->bounds.getWidth() / 4.0;
        double halfHeight = node->bounds.getHeight() / 4.0;
        
        // Create four children that divide the current node into four quadrants
        node->nw = std::make_unique<QuadTreeNode>(
            AxisAlignedBoundingBox(node->bounds.getMinX(), y, x, node->bounds.getMaxY()));
            
        node->ne = std::make_unique<QuadTreeNode>(
            AxisAlignedBoundingBox(x, y, node->bounds.getMaxX(), node->bounds.getMaxY()));
            
        node->sw = std::make_unique<QuadTreeNode>(
            AxisAlignedBoundingBox(node->bounds.getMinX(), node->bounds.getMinY(), x, y));
            
        node->se = std::make_unique<QuadTreeNode>(
            AxisAlignedBoundingBox(x, node->bounds.getMinY(), node->bounds.getMaxX(), y));
            
        node->is_divided = true;
        
        // Redistribute objects into children
        auto& objects = node->objects;
        for (size_t i = 0; i < objects.size(); ) {
            bool moved = false;
            const auto& aabb = objects[i].first;
            
            if (node->nw->bounds.intersects(aabb)) {
                node->nw->objects.push_back(objects[i]);
                moved = true;
            }
            if (node->ne->bounds.intersects(aabb)) {
                node->ne->objects.push_back(objects[i]);
                moved = true;
            }
            if (node->sw->bounds.intersects(aabb)) {
                node->sw->objects.push_back(objects[i]);
                moved = true;
            }
            if (node->se->bounds.intersects(aabb)) {
                node->se->objects.push_back(objects[i]);
                moved = true;
            }
            
            if (moved) {
                // Remove from parent if moved to any child
                objects[i] = std::move(objects.back());
                objects.pop_back();
            } else {
                ++i;
            }
        }
    }
    
    void query_region_helper(const QuadTreeNode* node, const AxisAlignedBoundingBox& range,
                           std::unordered_set<MetadataType>& result) const {
        if (!node) return;
        
        if (!node->bounds.intersects(range)) {
            return;
        }
        
        // Check objects in this node
        for (const auto& [aabb, meta] : node->objects) {
            if (range.intersects(aabb)) {
                result.insert(meta);
            }
        }
        
        // Recursively check children
        if (node->is_divided) {
            query_region_helper(node->nw.get(), range, result);
            query_region_helper(node->ne.get(), range, result);
            query_region_helper(node->sw.get(), range, result);
            query_region_helper(node->se.get(), range, result);
        }
    }
    
    template <typename Func>
    void for_each_helper(QuadTreeNode* node, Func&& func) const {
        if (!node) return;
        
        // Process objects in this node
        for (const auto& [aabb, meta] : node->objects) {
            func(aabb, meta);
        }
        
        // Recursively process children
        if (node->is_divided) {
            for_each_helper(node->nw.get(), std::forward<Func>(func));
            for_each_helper(node->ne.get(), std::forward<Func>(func));
            for_each_helper(node->sw.get(), std::forward<Func>(func));
            for_each_helper(node->se.get(), std::forward<Func>(func));
        }
    }

public:
    // Iterator implementation
    class Iterator {
        struct StackNode {
            const QuadTreeNode* node;
            size_t index;
        };

        std::vector<StackNode> stack;
        const QuadTreeNode* current_node;
        size_t current_index;

        void move_to_next() {
            while (!stack.empty()) {
                auto& top = stack.back();
                current_node = top.node;
                current_index = top.index;

                // If we've processed all objects in this node
                if (current_index >= current_node->objects.size()) {
                    if (current_node->is_divided) {
                        // Push children onto the stack in reverse order (we'll pop them in order)
                        stack.pop_back();
                        if (current_node->se) stack.push_back({current_node->se.get(), 0});
                        if (current_node->sw) stack.push_back({current_node->sw.get(), 0});
                        if (current_node->ne) stack.push_back({current_node->ne.get(), 0});
                        if (current_node->nw) stack.push_back({current_node->nw.get(), 0});
                    } else {
                        stack.pop_back();
                    }
                } else {
                    // We have an object to process
                    top.index++;
                    return;
                }
            }
            // If we get here, we've processed all nodes
            current_node = nullptr;
            current_index = 0;
        }

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<AxisAlignedBoundingBox, MetadataType>;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;
        
        Iterator() : current_node(nullptr), current_index(0) {}
        
        explicit Iterator(const QuadTreeNode* root) : current_node(nullptr), current_index(0) {
            if (root) {
                stack.push_back({root, 0});
                move_to_next();
            }
        }
        
        reference operator*() const { 
            return current_node->objects[current_index]; 
        }
        
        pointer operator->() const { 
            return &current_node->objects[current_index]; 
        }
        
        // Pre-increment
        Iterator& operator++() {
            move_to_next();
            return *this;
        }
        
        // Post-increment
        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        friend bool operator==(const Iterator& a, const Iterator& b) {
            return a.current_node == b.current_node && 
                  (a.current_node == nullptr || a.current_index == b.current_index);
        }
        
        friend bool operator!=(const Iterator& a, const Iterator& b) {
            return !(a == b);
        }
    };

    Quadtree(const AxisAlignedBoundingBox& bounds, unsigned int region_capacity)
        : root(std::make_unique<QuadTreeNode>(bounds)), region_capacity(region_capacity) {
        if (region_capacity == 0) {
            throw std::invalid_argument("Region capacity must be greater than 0");
        }
    }
    
    // Move constructor
    Quadtree(Quadtree&& other) noexcept = default;
    
    // Move assignment
    Quadtree& operator=(Quadtree&& other) noexcept = default;
    
    // No copy
    Quadtree(const Quadtree&) = delete;
    Quadtree& operator=(const Quadtree&) = delete;
    
    ~Quadtree() = default;

    void insert(const AxisAlignedBoundingBox& aabb, const MetadataType& meta) {
        if (!aabb.intersects(root->bounds)) {
            throw std::out_of_range("AABB is outside the bounds of the quadtree");
        }
        
        insert_recursive(root.get(), aabb, meta);
        total_objects++;
    }
    
    void insert_recursive(QuadTreeNode* node, const AxisAlignedBoundingBox& aabb, const MetadataType& meta) {
        // If this node is a leaf and has capacity, add the object
        if (!node->is_divided) {
            if (node->objects.size() < region_capacity) {
                node->objects.emplace_back(aabb, meta);
                return;
            }
            
            // If we've reached capacity, subdivide and continue
            subdivide(node);
        }
        
        // If divided, try to insert into children
        bool inserted = false;
        
        if (node->nw->bounds.intersects(aabb)) {
            insert_recursive(node->nw.get(), aabb, meta);
            inserted = true;
        }
        if (node->ne->bounds.intersects(aabb)) {
            insert_recursive(node->ne.get(), aabb, meta);
            inserted = true;
        }
        if (node->sw->bounds.intersects(aabb)) {
            insert_recursive(node->sw.get(), aabb, meta);
            inserted = true;
        }
        if (node->se->bounds.intersects(aabb)) {
            insert_recursive(node->se.get(), aabb, meta);
            inserted = true;
        }
        
        // If the AABB doesn't fit in any child, keep it in this node
        if (!inserted) {
            node->objects.emplace_back(aabb, meta);
        }
    }
    
    std::unordered_set<MetadataType> query_region(const AxisAlignedBoundingBox& range) const {
        std::unordered_set<MetadataType> result;
        query_region_helper(root.get(), range, result);
        return result;
    }
    
    // Iterator methods
    Iterator begin() const {
        return Iterator(root.get());
    }
    
    Iterator end() const {
        return Iterator();
    }
    
    // Get the root node (for visualization)
    const QuadTreeNode* get_root() const {
        return root.get();
    }
    
    size_t size() const { return total_objects; }
    bool empty() const { return total_objects == 0; }
    
    // For testing and debugging
    template <typename Func>
    void for_each(Func&& func) const {
        for_each_helper(root.get(), std::forward<Func>(func));
    }
};

} // namespace Project

#endif //PROJECT_QUADTREE_H
