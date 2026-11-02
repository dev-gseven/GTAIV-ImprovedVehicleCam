#include "VehicleTurnSpeed.h"
#include "Config.h"

#include <Windows.h>
#include <cstring>
#include <safetyhook.hpp>
#include <Hooking.Patterns.h>

namespace Camera
{
    namespace
    {
        safetyhook::MidHook vehicleXHook;
        safetyhook::MidHook vehicleYHook;
    }

    void InstallVehicleTurnSpeed()
    {
        auto axisXPattern = hook::pattern("F3 0F 5E C1 F3 0F 11 44 24 60 0F 84").get_first(); // (F3 0F 5E C1 F3 0F 11 44 24 ? 0F 84) 60 = 0x60
        auto axisYPattern = hook::pattern("F3 0F 5E C2 F3 0F 11 44 24 70 F3 0F").get_first(); // (F3 0F 5E C2 F3 0F 11 44 24 ? F3 0F) 70 = 0x70

        if (!axisXPattern || !axisYPattern)
            return;

        DWORD oldProtect = 0;

        // Axis X
        VirtualProtect(axisXPattern, 10, PAGE_EXECUTE_READWRITE, &oldProtect);
        memset(axisXPattern, 0x90, 10);
        VirtualProtect(axisXPattern, 10, oldProtect, &oldProtect);
        vehicleXHook = safetyhook::create_mid(axisXPattern, [](SafetyHookContext &reg)
        {
            const int8_t turn = Config::cfg.vehKbXTurn;
            const float adjustedTurnSpeed = reg.xmm0.f32[0] / (reg.xmm1.f32[0] * (1.0f - turn * 0.1f));
            float &turnSpeed = *(float*)(reg.esp + 0x60);
            turnSpeed = adjustedTurnSpeed;
        });

        // Axis Y
        VirtualProtect(axisYPattern, 10, PAGE_EXECUTE_READWRITE, &oldProtect);
        memset(axisYPattern, 0x90, 10);
        VirtualProtect(axisYPattern, 10, oldProtect, &oldProtect);
        vehicleYHook = safetyhook::create_mid(axisYPattern, [](SafetyHookContext &reg)
        {
            const int8_t turn = Config::cfg.vehKbYTurn;
            const float adjustedTurnSpeed = reg.xmm0.f32[0] / (reg.xmm2.f32[0] * (1.0f - turn * 0.1f));
            float &turnSpeed = *(float*)(reg.esp + 0x70);
            turnSpeed = adjustedTurnSpeed;
        });
    }
}