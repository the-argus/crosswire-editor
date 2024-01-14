#pragma once
#include <cstdint>
struct Inputs{
    int mouseX, mouseY;

    union{
        uint16_t rawButtonInputs;
        struct{
            uint16_t Select             : 1;
            uint16_t DragPoint          : 1;
            uint16_t New                : 1;
            uint16_t Delete             : 1;
            uint16_t Cancel             : 1;
            uint16_t AddPoint           : 1;
            uint16_t DeletePoint        : 1;
            uint16_t SaveToFile         : 1;
            uint16_t IncrementSelection : 1;
            uint16_t DecrementSelection : 1;
            uint16_t SetPlayerSpawn     : 1;
        };
    };
};
