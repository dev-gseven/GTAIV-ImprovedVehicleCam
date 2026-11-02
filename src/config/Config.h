#pragma once
#include <cstdint>

namespace Config
{
    struct Settings
    {
        int8_t vehKbXTurn;
        int8_t vehKbYTurn;
        bool theftCam;
        bool stuntJump;
        bool coverCentering;
    };

    extern Settings cfg;

    void Load();
}