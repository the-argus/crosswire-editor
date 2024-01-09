#include "util.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

float pointLineDistance(const Vec2& p, const Vec2& v, const Vec2& w) {
    // Line segment: v -> w, Point: p
    float l2 = pow(v.x - w.x, 2) + pow(v.y - w.y, 2);
    if (l2 == 0.0) return sqrt(pow(p.x - v.x, 2) + pow(p.y - v.y, 2)); // v == w case

    float t = ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2;
    t = std::max(0.0f, std::min(1.0f, t));
    Vec2 projection = {v.x + t * (w.x - v.x), v.y + t * (w.y - v.y)};
    return sqrt(pow(p.x - projection.x, 2) + pow(p.y - projection.y, 2));
}

float pointPointDistance(const Vec2& p, const Vec2& v) {
    float x = p.x - v.x;
    float y = p.y - v.y; 
    return sqrt(pow(y,2) + pow(x,2));
}

//Function that saves string to file
bool saveStringToFile(const std::string& data, const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    file << data;

    return file.good();
}

//Save level data to file
void saveLevelData(std::string data){
    std::string filepath = "output.txt";
    bool success = saveStringToFile(data, filepath);
    if (success) {
        std::cout << "File saved successfully." << std::endl;
    } else {
        std::cout << "Failed to save file." << std::endl;
    }
}