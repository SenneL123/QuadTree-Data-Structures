#include <catch2/catch.hpp>
#include "../include/Project/Quadtree.h"
#include <random>
#include <algorithm>

TEST_CASE("Rect contains and intersects", "[rect]")
{
    SECTION("Contains point")
    {
        Rect r{0, 0, 10, 10};
        
        // Center point
        REQUIRE(r.contains({0, 0}));
        
        // Edge points
        REQUIRE(r.contains({10, 0}));
        REQUIRE(r.contains({-10, 0}));
        REQUIRE(r.contains({0, 10}));
        REQUIRE(r.contains({0, -10}));
        
        // Corner points
        REQUIRE(r.contains({10, 10}));
        REQUIRE(r.contains({-10, 10}));
        REQUIRE(r.contains({10, -10}));
        REQUIRE(r.contains({-10, -10}));
        
        // Outside points
        REQUIRE_FALSE(r.contains({11, 0}));
        REQUIRE_FALSE(r.contains({0, 11}));
        REQUIRE_FALSE(r.contains({-11, 0}));
        REQUIRE_FALSE(r.contains({0, -11}));
    }
    
    SECTION("Intersects with other rect")
    {
        Rect r1{0, 0, 10, 10};
        
        // Overlapping
        REQUIRE(r1.intersects({5, 5, 5, 5}));
        
        // Edge contact
        REQUIRE(r1.intersects({15, 0, 5, 5}));  // Right edge
        REQUIRE(r1.intersects({-15, 0, 5, 5})); // Left edge
        
        // No intersection
        REQUIRE_FALSE(r1.intersects({20, 20, 5, 5}));
        REQUIRE_FALSE(r1.intersects({-20, -20, 5, 5}));
    }
}

TEST_CASE("QuadTree edge cases", "[quadtree]")
{
    SECTION("Empty tree")
    {
        Rect boundary{0, 0, 100, 100};
        QuadTree qt(boundary);
        
        // Query on empty tree
        auto result = qt.queryRange(boundary);
        REQUIRE(result.empty());
    }
    
    SECTION("Insert duplicate points")
    {
        Rect boundary{0, 0, 100, 100};
        QuadTree qt(boundary);
        
        Point p{10, 10};
        REQUIRE(qt.insert(p));
        REQUIRE(qt.insert(p));  // Should allow duplicates
        
        auto result = qt.queryRange(boundary);
        REQUIRE(result.size() == 2);
    }
    
    SECTION("Large number of points")
    {
        Rect boundary{0, 0, 1000, 1000};
        QuadTree qt(boundary);
        
        std::vector<Point> points;
        for (int i = 0; i < 1000; ++i) {
            points.push_back({static_cast<double>(i % 1000 - 500), 
                            static_cast<double>((i * 37) % 1000 - 500)});
        }
        
        for (const auto& p : points) {
            qt.insert(p);
        }
        
        // Query all points
        auto result = qt.queryRange(boundary);
        REQUIRE(result.size() == points.size());
        
        // Query a small region
        Rect smallRegion{0, 0, 10, 10};
        auto smallResult = qt.queryRange(smallRegion);
        
        // Manually verify points in the small region
        size_t expectedCount = 0;
        for (const auto& p : points) {
            if (smallRegion.contains(p)) {
                expectedCount++;
            }
        }
        REQUIRE(smallResult.size() == expectedCount);
    }
}

TEST_CASE("QuadTree stress test", "[quadtree][stress]")
{
    const int NUM_POINTS = 10000;
    const double BOUNDARY_SIZE = 1000.0;
    
    Rect boundary{0, 0, BOUNDARY_SIZE, BOUNDARY_SIZE};
    QuadTree qt(boundary);
    std::vector<Point> points;
    
    // Generate random points
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-BOUNDARY_SIZE, BOUNDARY_SIZE);
    
    for (int i = 0; i < NUM_POINTS; ++i) {
        Point p{dis(gen), dis(gen)};
        points.push_back(p);
        REQUIRE(qt.insert(p));
    }
    
    // Test random queries
    for (int i = 0; i < 100; ++i) {
        double x = dis(gen);
        double y = dis(gen);
        double hw = std::abs(dis(gen)) / 2.0;
        double hh = std::abs(dis(gen)) / 2.0;
        
        Rect queryRect{x, y, hw, hh};
        auto result = qt.queryRange(queryRect);
        
        // Verify results
        for (const auto& p : result) {
            REQUIRE(queryRect.contains(p));
        }
        
        // Cross-validate with brute force
        size_t expectedCount = 0;
        for (const auto& p : points) {
            if (queryRect.contains(p)) {
                expectedCount++;
            }
        }
        
        REQUIRE(result.size() == expectedCount);
    }
}
