#include <Project/QuadtreeAssignment.h>
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>

// Simple structure to represent a game object
struct GameObject {
    int id;
    std::string name;
    
    bool operator==(const GameObject& other) const {
        return id == other.id;
    }
};

int main() {
    std::cout << "=== Quadtree Demo ===\n\n";
    
    // Create a quadtree that covers a 1000x1000 area
    const double WORLD_SIZE = 1000.0;
    Quadtree<GameObject> qt(0, 0, WORLD_SIZE, WORLD_SIZE, 4);
    
    // Create some game objects
    std::vector<GameObject> objects;
    objects.push_back({1, "Player"});
    objects.push_back({2, "Enemy 1"});
    objects.push_back({3, "Enemy 2"});
    objects.push_back({4, "Power-up"});
    objects.push_back({5, "Obstacle 1"});
    
    // Insert objects with random positions
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> pos_dist(0, WORLD_SIZE - 50);
    
    std::cout << "Inserting objects into the quadtree:\n";
    for (const auto& obj : objects) {
        double x = pos_dist(gen);
        double y = pos_dist(gen);
        double size = 20.0 + pos_dist(gen) / 50.0; // Random size between 20 and 40
        
        qt.insert(obj, x, y, x + size, y + size);
        std::cout << "- " << obj.name << " at (" << x << ", " << y 
                 << ") size: " << size << "x" << size << "\n";
    }
    
    // Perform some queries
    std::cout << "\nPerforming range queries:\n";
    
    // Query 1: Top-left quadrant
    {
        double qx1 = 0, qy1 = 0, qx2 = WORLD_SIZE/2, qy2 = WORLD_SIZE/2;
        auto results = qt.query_range(qx1, qy1, qx2, qy2);
        std::cout << "Query 1 (Top-left quadrant): Found " << results.size() << " objects\n";
    }
    
    // Query 2: Center area
    {
        double center_x = WORLD_SIZE/2, center_y = WORLD_SIZE/2;
        double size = 100;
        auto results = qt.query_range(center_x - size/2, center_y - size/2, 
                                     center_x + size/2, center_y + size/2);
        std::cout << "Query 2 (Center area): Found " << results.size() << " objects\n";
    }
    
    // Performance test
    std::cout << "\nPerformance test: Inserting 10,000 objects...\n";
    
    Quadtree<int> perf_qt(0, 0, WORLD_SIZE, WORLD_SIZE, 8);
    const int NUM_OBJECTS = 10000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_OBJECTS; ++i) {
        double x = pos_dist(gen);
        double y = pos_dist(gen);
        double size = 5.0 + pos_dist(gen) / 200.0; // Small random size
        perf_qt.insert(i, x, y, x + size, y + size);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Inserted " << NUM_OBJECTS << " objects in " 
              << duration.count() << " ms\n";
    
    // Query test
    std::cout << "Performing 1000 random queries...\n";
    
    start = std::chrono::high_resolution_clock::now();
    
    int total_results = 0;
    for (int i = 0; i < 1000; ++i) {
        double qx = pos_dist(gen);
        double qy = pos_dist(gen);
        double qsize = 50.0 + pos_dist(gen) / 20.0; // Query size between 50 and 100
        
        auto results = perf_qt.query_range(qx, qy, qx + qsize, qy + qsize);
        total_results += results.size();
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Performed 1000 queries in " << duration.count() << " ms\n";
    std::cout << "Average results per query: " << (total_results / 1000.0) << "\n";
    
    std::cout << "\n=== Demo Complete ===\n";
    
    return 0;
}
