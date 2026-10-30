#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstdint>
#include <string>
#include <cstring>
#include <safetyhook.hpp>
#include <Hooking.Patterns.h>

namespace
{
    struct Config
    {
        int8_t vehKbXTurn;
        int8_t vehKbYTurn;
    } cfg;
    
    safetyhook::MidHook vehicleXHook, vehicleYHook;
    bool installed = false;

    std::string IniPath()
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

    int8_t Setting(const char* key, int8_t fallback, int8_t min, int8_t max)
    {
        int8_t value = GetPrivateProfileIntA("Camera", key, fallback, IniPath().c_str());

        if (value < min){
            return min;
        } else if (value > max){
            return max;
        }
        return value;
    }

    void LoadConfig()
    {
        cfg.vehKbXTurn = Setting("VehicleXTurnSpeed", 8, 1, 9);
        cfg.vehKbYTurn = Setting("VehicleYTurnSpeed", 6, 1, 9);
    }

    void Install()
    {
        auto axisX = hook::pattern("F3 0F 5E C1 F3 0F 11 44 24 60 0F 84").get_first(); // (F3 0F 5E C1 F3 0F 11 44 24 ? 0F 84) 60 = 0x60
        auto axisY = hook::pattern("F3 0F 5E C2 F3 0F 11 44 24 70 F3 0F").get_first(); // (F3 0F 5E C2 F3 0F 11 44 24 ? F3 0F) 70 = 0x70

        if (axisX && axisY){
            LoadConfig();
            DWORD oldProtect = 0;

            // Axis X
            VirtualProtect(axisX, 10, PAGE_EXECUTE_READWRITE, &oldProtect);
            memset(axisX, 0x90, 10);
            VirtualProtect(axisX, 10, oldProtect, &oldProtect);
            vehicleXHook = safetyhook::create_mid(axisX, [](SafetyHookContext &reg)
            {
                const int8_t turn = cfg.vehKbXTurn;
                const float adjustedTurnSpeed = reg.xmm0.f32[0] / (reg.xmm1.f32[0] * (1.0f - turn * 0.1f));
                float &turnSpeed = *(float*)(reg.esp + 0x60);
                turnSpeed = adjustedTurnSpeed;
            });

            // Axis Y
            VirtualProtect(axisY, 10, PAGE_EXECUTE_READWRITE, &oldProtect);
            memset(axisY, 0x90, 10);
            VirtualProtect(axisY, 10, oldProtect, &oldProtect);
            vehicleYHook = safetyhook::create_mid(axisY, [](SafetyHookContext &reg)
            {
                const int8_t turn = cfg.vehKbYTurn;
                const float adjustedTurnSpeed = reg.xmm0.f32[0] / (reg.xmm2.f32[0] * (1.0f - turn * 0.1f));
                float &turnSpeed = *(float*)(reg.esp + 0x70);
                turnSpeed = adjustedTurnSpeed;
            });

            installed = true;
        } else {
            return;
        }
    }

    DWORD WINAPI InitThread(void*)
    {
        for (int8_t i = 0; i < 60; ++i)
        {
            Install();
            if (installed) {
                return 0;
            }
            Sleep(1000);
        }
        return 0;
    }
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(instance);
        if (HANDLE thread = CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr)) CloseHandle(thread);
    }
    return TRUE;
}
