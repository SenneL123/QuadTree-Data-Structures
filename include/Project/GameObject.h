#ifndef PROJECT_GAMEOBJECT_H
#define PROJECT_GAMEOBJECT_H

#include <string>

// Simple structure to represent a game object
struct GameObject {
    int id;
    std::string name;
    
    bool operator==(const GameObject& other) const {
        return id == other.id;
    }
};

// Hash function for GameObject
namespace std {
    template<>
    struct hash<GameObject> {
        size_t operator()(const GameObject& obj) const {
            return std::hash<int>()(obj.id);
        }
    };
}

#endif // PROJECT_GAMEOBJECT_H
