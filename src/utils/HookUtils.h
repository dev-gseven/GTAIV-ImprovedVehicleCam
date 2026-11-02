#pragma once

#include <cstdint>
#include <optional>
#include <Zydis.h>

#define force_return_address(addr) (*(uintptr_t*)(regs.esp - 4) = (addr))
#define return_to(addr) do { force_return_address(addr); return; } while (0)


namespace HookUtils
{
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

}