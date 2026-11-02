#include <Windows.h>

#include "Config.h"

#include "VehicleTurnSpeed.h"
#include "VehicleTheftCamera.h"
#include "VehicleStuntJump.h"
#include "CoverCentering.h"

namespace
{
    bool installed = false;

    void Install()
    {
        Config::Load();

        Camera::InstallVehicleTurnSpeed();
        Camera::InstallTheftCamera();
        Camera::InstallVehicleStuntJump();
        Camera::InstallCoverCentering();

        installed = true;
    }

    DWORD WINAPI InitThread(void*)
    {
        for (int i = 0; i < 60; ++i)
        {
            Install();
            if (installed)
                return 0;
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
        if (HANDLE thread = CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr))
        {
            CloseHandle(thread);
        }
    }

    return TRUE;
}
