#include "Config.h"
#include <Windows.h>
#include <string>

namespace Config
{
    Settings cfg{};

    static std::string IniPath()
    {
        HMODULE self = nullptr;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&IniPath), &self);
        char path[MAX_PATH]{};
        GetModuleFileNameA(self, path, MAX_PATH);
        std::string result(path);
        const auto slash = result.find_last_of("/\\");
        result.resize(slash == std::string::npos ? 0 : slash + 1);
        return result + "ImprovedVehicleCam.ini";
    }

    static int8_t Setting(const char* key, int8_t fallback, int8_t min, int8_t max)
    {
        int8_t value = GetPrivateProfileIntA("Camera", key, fallback, IniPath().c_str());

        if (value < min){
            return min;
        } else if (value > max){
            return max;
        }
        return value;
    }

    void Load()
    {
        cfg.vehKbXTurn = Setting("VehicleXTurnSpeed", 8, 1, 9);
        cfg.vehKbYTurn = Setting("VehicleYTurnSpeed", 6, 1, 9);
        cfg.theftCam = Setting("VehicleTheftCam", 0, 0, 1);
        cfg.stuntJump = Setting("VehicleStuntJump", 0, 0, 1);
        cfg.coverCentering = Setting("CameraCenterInCover", 0, 0, 1);
    }
}