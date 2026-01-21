#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "../include/Project/Quadtree.h"
#include "../include/Project/AxisAlignedBoundingBox.h"

using namespace std;
using namespace Project;

TEST_CASE("AxisAlignedBoundingBox collides function", "[aabb]") {
    SECTION("Two AABBs overlap") {
        AxisAlignedBoundingBox a(0, 0, 10, 10);
        AxisAlignedBoundingBox b(5, 5, 15, 15);
        REQUIRE(a.intersects(b) == true);
    }
    
    SECTION("AABBs do not overlap") {
        AxisAlignedBoundingBox a(0, 0, 10, 10);
        AxisAlignedBoundingBox b(20, 20, 30, 30);
        REQUIRE(a.intersects(b) == false);
    }
    
    SECTION("AABBs touch edges") {
        AxisAlignedBoundingBox a(0, 0, 10, 10);
        AxisAlignedBoundingBox b(10, 10, 20, 20);
        REQUIRE(a.intersects(b) == true);
    }
}

TEST_CASE("Quadtree basic functionality", "[quadtree]") {
    AxisAlignedBoundingBox bounds(0, 0, 100, 100);
    Quadtree<int> qt(bounds, 2); // Small region capacity for testing subdivision
    
    SECTION("Insert and query single point") {
        AxisAlignedBoundingBox aabb1(10, 10, 20, 20);
        qt.insert(aabb1, 1);
        
        auto result = qt.query_region(AxisAlignedBoundingBox(0, 0, 100, 100));
        REQUIRE(result.size() == 1);
        REQUIRE(result.count(1) == 1);
    }
    
    SECTION("Insert multiple points and query") {
        qt.insert(AxisAlignedBoundingBox(10, 10, 20, 20), 1);
        qt.insert(AxisAlignedBoundingBox(30, 30, 40, 40), 2);
        qt.insert(AxisAlignedBoundingBox(50, 50, 60, 60), 3);
        
        auto result = qt.query_region(AxisAlignedBoundingBox(0, 0, 100, 100));
        REQUIRE(result.size() == 3);
        REQUIRE(result.count(1) == 1);
        REQUIRE(result.count(2) == 1);
        REQUIRE(result.count(3) == 1);
    }
    
    SECTION("Query with specific region") {
        qt.insert(AxisAlignedBoundingBox(10, 10, 20, 20), 1); // In SW
        qt.insert(AxisAlignedBoundingBox(70, 70, 80, 80), 2); // In NE
        
        // Query only SW quadrant
        auto swResult = qt.query_region(AxisAlignedBoundingBox(0, 0, 50, 50));
        REQUIRE(swResult.size() == 1);
        REQUIRE(swResult.count(1) == 1);
        
        // Query only NE quadrant
        auto neResult = qt.query_region(AxisAlignedBoundingBox(50, 50, 100, 100));
        REQUIRE(neResult.size() == 1);
        REQUIRE(neResult.count(2) == 1);
    }
}

TEST_CASE("Quadtree subdivision", "[quadtree]") {
    AxisAlignedBoundingBox bounds(0, 0, 100, 100);
    Quadtree<int> qt(bounds, 1); // Force subdivision after first insert
    
    SECTION("Subdivision occurs when capacity is exceeded") {
        qt.insert(AxisAlignedBoundingBox(10, 10, 20, 20), 1);
        // This second insert should trigger subdivision
        qt.insert(AxisAlignedBoundingBox(30, 30, 40, 40), 2);
        
        // Both points should still be found
        auto result = qt.query_region(bounds);
        REQUIRE(result.size() == 2);
        REQUIRE(result.count(1) == 1);
        REQUIRE(result.count(2) == 1);
    }
    
    SECTION("Objects that span multiple quadrants") {
        // This AABB spans all four quadrants
        qt.insert(AxisAlignedBoundingBox(40, 40, 60, 60), 1);
        
        // Should be found in all quadrants
        REQUIRE(qt.query_region(AxisAlignedBoundingBox(0, 0, 50, 50)).count(1) == 1); // SW
        REQUIRE(qt.query_region(AxisAlignedBoundingBox(50, 0, 100, 50)).count(1) == 1); // SE
        REQUIRE(qt.query_region(AxisAlignedBoundingBox(0, 50, 50, 100)).count(1) == 1); // NW
        REQUIRE(qt.query_region(AxisAlignedBoundingBox(50, 50, 100, 100)).count(1) == 1); // NE
    }
}

TEST_CASE("Quadtree iterators", "[quadtree][iterator]") {
    AxisAlignedBoundingBox bounds(0, 0, 100, 100);
    Quadtree<string> qt(bounds, 2);
    
    qt.insert(AxisAlignedBoundingBox(10, 10, 20, 20), "A");
    qt.insert(AxisAlignedBoundingBox(30, 30, 40, 40), "B");
    qt.insert(AxisAlignedBoundingBox(50, 50, 60, 60), "C");
    
    SECTION("Iterator visits all elements") {
        set<string> found;
        for (auto it = qt.begin(); it != qt.end(); ++it) {
            found.insert(it->second);
        }
        
        REQUIRE(found.size() == 3);
        REQUIRE(found.count("A") == 1);
        REQUIRE(found.count("B") == 1);
        REQUIRE(found.count("C") == 1);
    }
    
    SECTION("Empty tree has empty iterator range") {
        Quadtree<string> empty_qt(bounds, 2);
        REQUIRE(empty_qt.begin() == empty_qt.end());
    }
}

TEST_CASE("Quadtree edge cases", "[quadtree][edge]") {
    SECTION("Empty quadtree") {
        AxisAlignedBoundingBox bounds(0, 0, 100, 100);
        Quadtree<int> qt(bounds, 2);
        
        REQUIRE(qt.empty() == true);
        REQUIRE(qt.size() == 0);
        REQUIRE(qt.query_region(bounds).empty() == true);
    }
    
    SECTION("Insert at boundaries") {
        AxisAlignedBoundingBox bounds(0, 0, 100, 100);
        Quadtree<int> qt(bounds, 2);
        
        // Insert at each corner and edge
        qt.insert(AxisAlignedBoundingBox(0, 0, 0, 0), 1);       // SW corner
        qt.insert(AxisAlignedBoundingBox(100, 0, 100, 0), 2);   // SE corner
        qt.insert(AxisAlignedBoundingBox(0, 100, 0, 100), 3);   // NW corner
        qt.insert(AxisAlignedBoundingBox(100, 100, 100, 100), 4); // NE corner
        qt.insert(AxisAlignedBoundingBox(50, 0, 50, 0), 5);     // South edge
        
        REQUIRE(qt.size() == 5);
        
        // Query the whole area
        auto result = qt.query_region(bounds);
        REQUIRE(result.size() == 5);
        for (int i = 1; i <= 5; ++i) {
            REQUIRE(result.count(i) == 1);
        }
    }
    
    SECTION("Query with empty region") {
        AxisAlignedBoundingBox bounds(0, 0, 100, 100);
        Quadtree<int> qt(bounds, 2);
        
        qt.insert(AxisAlignedBoundingBox(10, 10, 20, 20), 1);
        
        // Query with non-overlapping region
        auto result = qt.query_region(AxisAlignedBoundingBox(200, 200, 300, 300));
        REQUIRE(result.empty() == true);
    }
}