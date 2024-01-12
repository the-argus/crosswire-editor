#include "Polygons.h"
#ifdef ZIGBUILD
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include <cmath>
#include <limits>
#include "util.h"
#include <sstream>
#include <string>
#include <iomanip>

void Polygon::addPoint(Inputs& i){
        if (i.AddPoint && points.size() >= 2) {
            float minDistance = std::numeric_limits<float>::max();
            size_t closestSegmentIndex = 0;

            Vec2 mousePoint = {(float)i.mouseX, (float)i.mouseY};
            for (size_t j = 0; j < points.size(); j++) {
                float dist = pointLineDistance(mousePoint, points[j], points[(j + 1) % points.size()]);
                if (dist < minDistance) {
                    minDistance = dist;
                    closestSegmentIndex = j;
                }
            }

            Vec2 midPoint = {
                (points[closestSegmentIndex].x + points[(closestSegmentIndex + 1) % points.size()].x) / 2.0f,
                (points[closestSegmentIndex].y + points[(closestSegmentIndex + 1) % points.size()].y) / 2.0f
            };
            points.insert(points.begin() + closestSegmentIndex + 1, midPoint);
        }
    }


void Polygon::selectPoint(Inputs& i){
        selectedPoint = -1;
        Vec2 mousePoint = {(float)i.mouseX, (float)i.mouseY};
        for(int i = 0; i < points.size(); i++){
            float dist = pointPointDistance(points[i], mousePoint);
            if(dist < SELECT_DISTANCE){
                selectedPoint = i;
                break;
            }
        }
    }
void Polygon::dragPoint(Inputs& i){
    if(selectedPoint != -1){
        points[selectedPoint].x = i.mouseX;
        points[selectedPoint].y = i.mouseY;
    }
}

void Polygon::deletePoint(Inputs& i){
    if(selectedPoint != -1){
        if(points.size() > 3){
            points.erase(points.begin() + selectedPoint);
        }
    }
}
void Polygon::updatePolygon(Inputs& i){
    if (i.AddPoint) {
        addPoint(i);
    }
    if(i.SelectPoint){
        selectPoint(i);
    } else if(i.DragPoint){
        dragPoint(i);
    } else if(i.DeletePoint){ 
        deletePoint(i);
    }
}

void Polygon::drawPolygon(SDL_Renderer* r, uint8_t red, uint8_t green, uint8_t blue){
    SDL_SetRenderDrawColor(r, red, green, blue, 255); //White Lines

    if (points.size() < 2) {
        return;
    }
    
    //Render Rectangles
    SDL_Rect rect;
    rect.w = 8;
    rect.h = 8;
    for (size_t i = 0; i < points.size() - 1; ++i) {
        SDL_RenderDrawLine(r, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
        rect.x = points[i].x - 4;
        rect.y = points[i].y - 4;
        SDL_RenderFillRect(r, &rect);
    }
    SDL_RenderDrawLine(r, points.back().x, points.back().y, points[0].x, points[0].y);
        rect.x = points.back().x - 4;
        rect.y = points.back().y - 4;
        SDL_RenderFillRect(r, &rect);
}

std::string Polygon::SerializePolygon() {
    std::ostringstream oss;
    for (const Vec2& point : points) {
        oss << std::fixed << std::setprecision(2) << point.x << " " << point.y << " ";
    }
    return oss.str();
}
