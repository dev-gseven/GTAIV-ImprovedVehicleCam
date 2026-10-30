#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstdint>
#include <string>
#include <cstring>
#include <optional>
#include <safetyhook.hpp>
#include <Hooking.Patterns.h>
#include <Zydis.h>

#define force_return_address(addr) (*(uintptr_t*)(regs.esp - 4) = (addr))
#define return_to(addr) do { force_return_address(addr); return; } while (0)

safetyhook::MidHook vehicleXHook, vehicleYHook;
bool installed = false;

struct Config
{
    int8_t vehKbXTurn;
    int8_t vehKbYTurn;
    bool theftCam;
    bool stuntJump;
} cfg;

namespace CIntermezzoEventPlayerJackVehicle
{
    SafetyHookInline shDetectTrigger = {};
    bool __fastcall DetectTrigger(void* _this, void* edx)
    {
        static bool TheftCam = cfg.theftCam;
        if (!TheftCam)
        {
            return false;
        }

        return shDetectTrigger.unsafe_fastcall<bool>(_this, edx);
    }
}

template <typename T>
std::optional<uintptr_t> resolve_next_displacement(T ip)
{
    if (!ip)
        return std::nullopt;

    ZydisDecoder decoder{};
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_STACK_WIDTH_32);

    uintptr_t currentIp = reinterpret_cast<uintptr_t>(ip);
    for (size_t count = 0; count < 20; ++count)
    {
        ZydisDecodedInstruction instruction{};
        ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT]{};
        const auto status = ZydisDecoderDecodeFull(
            &decoder,
            reinterpret_cast<const void*>(currentIp),
            ZYDIS_MAX_INSTRUCTION_LENGTH,
            &instruction,
            operands);

        if (!ZYAN_SUCCESS(status))
            return std::nullopt;

        if (instruction.meta.category == ZYDIS_CATEGORY_COND_BR)
        {
            for (uint32_t i = 0; i < instruction.operand_count_visible; ++i)
            {
                const auto& operand = operands[i];
                if (operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE && operand.imm.is_relative)
                    return currentIp + instruction.length + static_cast<intptr_t>(operand.imm.value.s);
            }

            if ((instruction.attributes & ZYDIS_ATTRIB_IS_RELATIVE) && instruction.raw.disp.size > 0)
                return currentIp + instruction.length + static_cast<intptr_t>(instruction.raw.disp.value);

            return std::nullopt;
        }

        currentIp += instruction.length;
    }

    return std::nullopt;
}

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
    cfg.theftCam = Setting("VehicleTheftCam", 0, 0, 1);
    cfg.stuntJump = Setting("VehicleStuntJump", 0, 0, 1);
}

void Install()
{
    auto axisXPattern = hook::pattern("F3 0F 5E C1 F3 0F 11 44 24 60 0F 84").get_first(); // (F3 0F 5E C1 F3 0F 11 44 24 ? 0F 84) 60 = 0x60
    auto axisYPattern = hook::pattern("F3 0F 5E C2 F3 0F 11 44 24 70 F3 0F").get_first(); // (F3 0F 5E C2 F3 0F 11 44 24 ? F3 0F) 70 = 0x70

    if (axisXPattern && axisYPattern){
        LoadConfig();
        DWORD oldProtect = 0;

        // Axis X
        VirtualProtect(axisXPattern, 10, PAGE_EXECUTE_READWRITE, &oldProtect);
        memset(axisXPattern, 0x90, 10);
        VirtualProtect(axisXPattern, 10, oldProtect, &oldProtect);
        vehicleXHook = safetyhook::create_mid(axisXPattern, [](SafetyHookContext &reg)
        {
            const int8_t turn = cfg.vehKbXTurn;
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
            const int8_t turn = cfg.vehKbYTurn;
            const float adjustedTurnSpeed = reg.xmm0.f32[0] / (reg.xmm2.f32[0] * (1.0f - turn * 0.1f));
            float &turnSpeed = *(float*)(reg.esp + 0x70);
            turnSpeed = adjustedTurnSpeed;
        });

        installed = true;
    } else {
        return;
    }


    // CarTheftCamera
    auto carJackPattern = hook::pattern("55 8B EC 83 E4 ? 83 EC ? 56 57 8B F9 E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? E8 ? ? ? ? 8B F0 85 F6 0F 84 ? ? ? ? 8B 8E");
    CIntermezzoEventPlayerJackVehicle::shDetectTrigger = safetyhook::create_inline(carJackPattern.get_first(0), CIntermezzoEventPlayerJackVehicle::DetectTrigger);

    auto stuntJumpPattern = hook::pattern("8B 0D ? ? ? ? 83 C1 ? 51 B9");
    static auto loc_CC3F66 = resolve_next_displacement(stuntJumpPattern.get_first(27)).value();
    static auto CStuntJumpManager__Update_Hook = safetyhook::create_mid(stuntJumpPattern.get_first(9), [](SafetyHookContext& regs)
    {
        static bool StuntJumpCam = cfg.stuntJump;
        if (!StuntJumpCam)
        {
            return_to(loc_CC3F66);
        }
    });
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

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(instance);
        if (HANDLE thread = CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr)) CloseHandle(thread);
    }
    return TRUE;
}
