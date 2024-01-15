#pragma once
#include <vector>
#include <string>
#ifdef ZIGBUILD
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include "Inputs.h"
#include "Vec2.h"
#include <span>

const float SELECT_DISTANCE = 25.0f;

struct Polygon{
private:
    //Variables    
    std::vector<Vec2> points;
    int selectedPoint;

    //Private Functions
    void addPoint(Inputs& i);
    void selectPoint(Inputs& i);
    void dragPoint(Inputs& i);
    void deletePoint(Inputs& i);

public:
    inline constexpr const std::vector<Vec2>& getPoints() const {return points;}
    Polygon(const std::span<const Vec2>& vertices) {
        points.reserve(vertices.size());
        // copy the vertices
        for (const auto& vertex : vertices) {
            points.push_back(vertex);
        }
        selectedPoint = -1;
    }

    //Constructor
    Polygon(int x, int y){
        points.push_back((Vec2){static_cast<float>(x - 20), static_cast<float>(y + 20)});
        points.push_back((Vec2){static_cast<float>(x - 20), static_cast<float>(y - 20)});
        points.push_back((Vec2){static_cast<float>(x + 20), static_cast<float>(y - 20)});
        selectedPoint = -1;
    }
    void updatePolygon(Inputs& i);
    void drawPolygon(SDL_Renderer* r, uint8_t red, uint8_t green, uint8_t blue);
    std::string SerializePolygon();
};
