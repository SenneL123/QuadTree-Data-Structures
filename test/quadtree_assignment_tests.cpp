#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <Project/QuadtreeAssignment.h>
#include <vector>
#include <string>
#include <random>

TEST_CASE("Quadtree Construction", "[construction]") {
    SECTION("Valid construction") {
        REQUIRE_NOTHROW(Quadtree<int>(0, 0, 100, 100, 4));
        REQUIRE_NOTHROW(Quadtree<std::string>(-100, -100, 100, 100, 10));
    }
    
    SECTION("Invalid construction") {
        REQUIRE_THROWS_AS(Quadtree<int>(100, 0, 0, 100, 4), std::invalid_argument); // x_min > x_max
        REQUIRE_THROWS_AS(Quadtree<int>(0, 100, 100, 0, 4), std::invalid_argument); // y_min > y_max
        REQUIRE_THROWS_AS(Quadtree<int>(0, 0, 100, 100, 0), std::invalid_argument); // capacity = 0
    }
}

TEST_CASE("Quadtree Insertion", "[insertion]") {
    Quadtree<int> qt(0, 0, 100, 100, 2);
    
    SECTION("Insert within bounds") {
        REQUIRE(qt.insert(1, 10, 10, 20, 20));
        REQUIRE(qt.insert(2, 30, 30, 40, 40));
        REQUIRE(qt.size() == 2);
    }
    
    SECTION("Insert outside bounds") {
        REQUIRE_FALSE(qt.insert(1, -10, -10, -5, -5));
        REQUIRE_FALSE(qt.insert(2, 101, 101, 110, 110));
        REQUIRE(qt.size() == 0);
    }
    
    SECTION("Insert causes subdivision") {
        REQUIRE(qt.insert(1, 10, 10, 20, 20));
        REQUIRE(qt.insert(2, 30, 30, 40, 40));
        REQUIRE(qt.insert(3, 50, 50, 60, 60)); // Should cause subdivision
        REQUIRE(qt.size() == 3);
    }
}

TEST_CASE("Quadtree Query Range", "[query]")
{
    Quadtree<int> qt(0, 0, 100, 100, 2);
    
    // Insert test data
    qt.insert(1, 10, 10, 20, 20);
    qt.insert(2, 30, 30, 40, 40);
    qt.insert(3, 50, 50, 60, 60);
    qt.insert(4, 70, 70, 80, 80);
    
    SECTION("Query entire space") {
        auto results = qt.query_range(0, 0, 100, 100);
        REQUIRE(results.size() == 4);
    }
    
    SECTION("Query partial range") {
        auto results = qt.query_range(0, 0, 30, 30);
        REQUIRE(results.size() == 1);
        REQUIRE(results[0] == 1);
    }
    
    SECTION("Query empty range") {
        auto results = qt.query_range(90, 90, 95, 95);
        REQUIRE(results.empty());
    }
    
    SECTION("Query edge case - point on boundary") {
        auto results = qt.query_range(20, 20, 30, 30);
        REQUIRE(results.size() == 1);
        REQUIRE(results[0] == 1);
    }
}

TEST_CASE("Quadtree Clear", "[clear]") {
    Quadtree<int> qt(0, 0, 100, 100);
    
    qt.insert(1, 10, 10, 20, 20);
    qt.insert(2, 30, 30, 40, 40);
    
    REQUIRE(qt.size() == 2);
    qt.clear();
    
    SECTION("After clear") {
        REQUIRE(qt.size() == 0);
        REQUIRE(qt.empty());
        
        // Should be able to insert again after clear
        REQUIRE(qt.insert(3, 50, 50, 60, 60));
        REQUIRE(qt.size() == 1);
    }
}

TEST_CASE("Quadtree Stress Test", "[stress]") {
    const int NUM_ELEMENTS = 1000;
    Quadtree<int> qt(0, 0, 1000, 1000, 10);
    std::vector<std::pair<int, std::tuple<double, double, double, double>>> elements;
    
    // Generate random elements
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> pos_dist(0, 1000);
    std::uniform_real_distribution<> size_dist(1, 20);
    
    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        double x = pos_dist(gen);
        double y = pos_dist(gen);
        double w = size_dist(gen);
        double h = size_dist(gen);
        
        // Ensure bounds are within the quadtree
        x = std::min(x, 1000.0 - w);
        y = std::min(y, 1000.0 - h);
        
        qt.insert(i, x, y, x + w, y + h);
        elements.emplace_back(i, std::make_tuple(x, y, x + w, y + h));
    }
    
    REQUIRE(qt.size() == NUM_ELEMENTS);
    
    // Test random queries
    for (int i = 0; i < 10; ++i) {
        double qx = pos_dist(gen);
        double qy = pos_dist(gen);
        double qw = size_dist(gen) * 10;
        double qh = size_dist(gen) * 10;
        
        auto results = qt.query_range(qx, qy, qx + qw, qy + qh);
        
        // Brute-force verification
        std::vector<int> expected;
        for (const auto& elem : elements) {
            double ex1, ey1, ex2, ey2;
            std::tie(ex1, ey1, ex2, ey2) = elem.second;
            
            // Check if element intersects with query range
            if (!(ex2 < qx || ex1 > qx + qw || ey2 < qy || ey1 > qy + qh)) {
                expected.push_back(elem.first);
            }
        }
        
        REQUIRE(results.size() == expected.size());
        
        // Verify all expected elements are in results
        for (int id : expected) {
            REQUIRE(std::find(results.begin(), results.end(), id) != results.end());
        }
    }
}

TEST_CASE("Quadtree with custom types", "[custom_type]") {
    struct GameObject {
        int id;
        std::string name;
        
        bool operator==(const GameObject& other) const {
            return id == other.id && name == other.name;
        }
    };
    
    Quadtree<GameObject> qt(0, 0, 100, 100);
    
    GameObject obj1{1, "Player"};
    GameObject obj2{2, "Enemy"};
    
    qt.insert(obj1, 10, 10, 20, 20);
    qt.insert(obj2, 30, 30, 40, 40);
    
    SECTION("Query for objects") {
        auto results = qt.query_range(0, 0, 100, 100);
        REQUIRE(results.size() == 2);
        REQUIRE(std::find(results.begin(), results.end(), obj1) != results.end());
        REQUIRE(std::find(results.begin(), results.end(), obj2) != results.end());
    }
    
    SECTION("Query with no results") {
        auto results = qt.query_range(50, 50, 60, 60);
        REQUIRE(results.empty());
    }
}
