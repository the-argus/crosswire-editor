#pragma once
#include <vector>
#include <string>
#include <SDL.h>
#include "Inputs.h"
#include "Vec2.h"

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
    //Constructor
    Polygon(int x, int y){
        points.push_back((Vec2){x - 20, y + 20});
        points.push_back((Vec2){x - 20, y - 20});
        points.push_back((Vec2){x + 20, y - 20});
        selectedPoint = -1;
    }
    void updatePolygon(Inputs& i);
    void drawPolygon(SDL_Renderer* r, uint8_t red, uint8_t green, uint8_t blue);
    std::string SerializePolygon();
};
