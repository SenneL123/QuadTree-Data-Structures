#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "../include/Project/Quadtree.h"


TEST_CASE("QuadTree basic insertion", "[quadtree]") {
    Rect boundary{0, 0, 100, 100};
    QuadTree qt(boundary);
    
    REQUIRE(qt.insert({10, 10}));
    REQUIRE(qt.insert({20, 20}));
    REQUIRE(qt.insert({-10, -10}));
    REQUIRE(qt.insert({-20, -20}));
}

TEST_CASE("QuadTree boundary checks", "[quadtree]") {
    Rect boundary{0, 0, 100, 100};
    QuadTree qt(boundary);
    
    // Points inside boundary
    REQUIRE(qt.insert({99, 99}));  // Edge of boundary
    REQUIRE(qt.insert({1, 1}));    // Inside boundary
    
    // Points outside boundary
    REQUIRE_FALSE(qt.insert({101, 50}));  // Outside x
    REQUIRE_FALSE(qt.insert({50, 101}));  // Outside y
    REQUIRE_FALSE(qt.insert({-101, 50})); // Outside negative x
    REQUIRE_FALSE(qt.insert({50, -101})); // Outside negative y
}

TEST_CASE("QuadTree query range", "[quadtree]") {
    Rect boundary{0, 0, 100, 100};
    QuadTree qt(boundary);
    
    // Insert points in different quadrants
    qt.insert({10, 10});   // NW
    qt.insert({-10, 10});  // NE
    qt.insert({10, -10});  // SW
    qt.insert({-10, -10}); // SE
    
    // Query NW quadrant (x > 0, y > 0)
    // The Rect constructor takes (centerX, centerY, halfWidth, halfHeight)
    // To query the NW quadrant (0 to 100, 0 to 100), we need to use half of those values
    Rect queryNW{50, 50, 50, 50};  // Center at (50,50) with halfWidth=50, halfHeight=50
    auto result = qt.queryRange(queryNW);
    
    // Should find only the point in the NW quadrant
    REQUIRE(result.size() == 1);
    if (!result.empty()) {
        REQUIRE(result[0].x == 10);
        REQUIRE(result[0].y == 10);
    }
    
    // Test with a smaller range in the NW quadrant
    Rect smallNW{25, 25, 25, 25};  // Center at (25,25) with halfWidth=25, halfHeight=25
    auto nwResult = qt.queryRange(smallNW);
    REQUIRE(nwResult.size() == 1);  // Only {10, 10} should be in this range
    if (!nwResult.empty()) {
        REQUIRE(nwResult[0].x == 10);
        REQUIRE(nwResult[0].y == 10);
    }
}

TEST_CASE("QuadTree subdivision", "[quadtree]") {
    Rect boundary{0, 0, 100, 100};
    QuadTree qt(boundary);
    
    // Insert more points than capacity (4)
    for (int i = 0; i < 5; ++i) {
        REQUIRE(qt.insert({static_cast<double>(i), static_cast<double>(i)}));
    }
    
    // Should have subdivided and accepted all points
    REQUIRE(qt.queryRange(boundary).size() == 5);
}