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
    // The standalone version keeps the same game signatures and camera math as
    // FusionFix. Its only configuration is CamTurnSpeed.ini beside the ASI.
    struct Config
    {
        int8_t vehKbXTurn = 8;
        int8_t vehKbYTurn = 5;
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
        cfg.vehKbYTurn = Setting("VehicleYTurnSpeed", 5, 1, 9);
    }

    void* Find(const char* a, const char* b = nullptr)
    {
        hook::pattern p(a);
        if (p.empty() && b) p = hook::pattern(b);
        return p.empty() ? nullptr : p.get_first();
    }

    void Install()
    {
        auto axisX  = Find("F3 0F 5E C1 F3 0F 11 44 24 60 0F 84");
        auto axisY = Find("F3 0F 5E C2 F3 0F 11 44 24 70 F3 0F 10");

        if (axisX && axisY){
            LoadConfig();
            DWORD oldProtect = 0;

            // Axis X
            VirtualProtect(axisX, 10, PAGE_EXECUTE_READWRITE, &oldProtect);
            memset(axisX, 0x90, 10);
            VirtualProtect(axisX, 10, oldProtect, &oldProtect);
            vehicleXHook = safetyhook::create_mid(axisX, [](SafetyHookContext& r)
            {
                const int8_t turn = cfg.vehKbXTurn;
                const float v = r.xmm0.f32[0] / (r.xmm1.f32[0] * (1.0f - turn * 0.1f));
                float& target = *reinterpret_cast<float*>(r.esp + 0x60);
                target = v;
            });

            // Axis Y
            VirtualProtect(axisY, 10, PAGE_EXECUTE_READWRITE, &oldProtect);
            memset(axisY, 0x90, 10);
            VirtualProtect(axisY, 10, oldProtect, &oldProtect);
            vehicleYHook = safetyhook::create_mid(axisY, [](SafetyHookContext& r)
            {
                const int8_t turn = cfg.vehKbYTurn;
                const float v = r.xmm0.f32[0] / (r.xmm2.f32[0] * (1.0f - turn * 0.1f));
                float& target = *reinterpret_cast<float*>(r.esp + 0x70);
                target = v;
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
