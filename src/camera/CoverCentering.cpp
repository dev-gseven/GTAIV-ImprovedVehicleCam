#include <CoverCentering.h>
#include <Config.h>
#include <cfloat>
#include <Hooking.Patterns.h>
#include <injector.hpp>

namespace Camera
{
    void InstallCoverCentering()
    {
        if (Config::cfg.coverCentering)
            return;

            static constexpr float xmm_0 = FLT_MAX / 2.0f;
            auto coverCenterPattern = hook::pattern("F3 0F 10 05 ? ? ? ? F3 0F 58 46 ? 89 8C 24");
            injector::WriteMemory(coverCenterPattern.get_first(4), &xmm_0, true);
    }
}
