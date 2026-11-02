#include "VehicleTheftCamera.h"
#include "Config.h"
#include <safetyhook.hpp>
#include <Hooking.Patterns.h>

namespace Camera {

	namespace CIntermezzoEventPlayerJackVehicle{
	    SafetyHookInline shDetectTrigger = {};
	    bool __fastcall DetectTrigger(void* _this, void* edx)
	    {
	        static bool TheftCam = Config::cfg.theftCam;
	        if (!TheftCam)
	        {
	            return false;
	        }

	        return shDetectTrigger.unsafe_fastcall<bool>(_this, edx);
	    }
	}

	void InstallTheftCamera(){

	// CarTheftCamera
    auto carJackPattern = hook::pattern("55 8B EC 83 E4 ? 83 EC ? 56 57 8B F9 E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? E8 ? ? ? ? 8B F0 85 F6 0F 84 ? ? ? ? 8B 8E");
    CIntermezzoEventPlayerJackVehicle::shDetectTrigger = safetyhook::create_inline(carJackPattern.get_first(0), CIntermezzoEventPlayerJackVehicle::DetectTrigger);

	}

}
