#pragma once
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include "Polygons.h"
#include "Inputs.h"

/**
 * @brief Represents a room/level with polygons representing areas
 */
struct Room{
private:
    std::vector<Polygon> Areas;
    int currentPolygon;
public:
    //Constructor
    Room(){
        currentPolygon = -1;
    }

    //Update the current room based on user input and selected polygon
    void updateRoom(Inputs i){

        //Add New Polygon
        if(i.NewPolygon){
            Areas.push_back(Polygon(i.mouseX,i.mouseY));
            currentPolygon = Areas.size() - 1;
        }
        //Delete current Polygon
        if(i.DeletePolygon){
            if(currentPolygon != -1 && Areas.size() != 0){
                Areas.erase(Areas.begin() + currentPolygon);
            }
        }

        //Polygon Selection Update
        if(i.IncrementSelection){
            if (!Areas.empty()) {
                currentPolygon = (currentPolygon + 1) % Areas.size();
            }
        }

        if(i.DecrementSelection){
            if (!Areas.empty()) {
                currentPolygon = (currentPolygon - 1 + Areas.size()) % Areas.size();
            }
        }
        //Update Polygon
        if(currentPolygon != -1){
            Areas[currentPolygon].updatePolygon(i);
        }
    }

    //Render all polygons of the current room onto the screen
    void drawRoom(SDL_Renderer* renderer){        
        const uint8_t BASE_RED = 255, BASE_GREEN = 255, BASE_BLUE = 255, SELECT_RED = 32, SELECT_GREEN = 255, SELECT_BLUE = 64;
        for(int i = 0; i < Areas.size(); i++){
            if(i == currentPolygon){
                Areas[i].drawPolygon(renderer, SELECT_RED, SELECT_GREEN, SELECT_BLUE);
            }else{
                Areas[i].drawPolygon(renderer, BASE_RED, BASE_GREEN, BASE_BLUE);
            }
        }
    }
    
    std::string SerializeRoom() {
        std::ostringstream oss;
        for (size_t i = 0; i < Areas.size(); i++) {
            oss << std::setw(4) << std::setfill('0') << i << " ";
            oss << Areas[i].SerializePolygon();
            oss << "\n";
        }
        return oss.str();
    }

    int getNumberOfPolygons(){
        return Areas.size();
    }
    void selectPolygon(int i){
        if(i < getNumberOfPolygons()){
            currentPolygon = i;
        }
    }
    int getSelectedPolygon(){
        return currentPolygon;
    }
};