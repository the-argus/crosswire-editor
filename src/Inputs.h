#pragma once
#include <cstdint>
struct Inputs{
    int mouseX, mouseY;

    union{
        uint16_t rawButtonInputs;
        struct{
            uint16_t SelectPoint        : 1;
            uint16_t DragPoint          : 1;
            uint16_t NewPolygon         : 1;
            uint16_t DeletePolygon      : 1;
            uint16_t AddPoint           : 1;
            uint16_t DeletePoint        : 1;
            uint16_t SaveToFile         : 1;
            uint16_t IncrementSelection : 1;
            uint16_t DecrementSelection : 1;
        };
    };
};