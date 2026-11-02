#include "VehicleStuntJump.h"
#include "Config.h"
#include "HookUtils.h"

#include <Hooking.Patterns.h>
#include <safetyhook.hpp>

namespace Camera
{
    void InstallVehicleStuntJump()
    {
        if (Config::cfg.stuntJump)
            return;

        auto stuntJumpPattern = hook::pattern("8B 0D ? ? ? ? 83 C1 ? 51 B9");
        static auto returnAddress = HookUtils::resolve_next_displacement(stuntJumpPattern.get_first(27)).value();
        static auto stuntJumpHook = safetyhook::create_mid(stuntJumpPattern.get_first(9), [](SafetyHookContext& regs)
        {
            return_to(returnAddress);
        });
    }
}
