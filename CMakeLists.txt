
cmake_minimum_required(VERSION 3.21)

project(ImprovedVehicleCam LANGUAGES C CXX)

add_library(ImprovedVehicleCam SHARED
    src/ImprovedVehicleCam.cpp
    external/hooking/Hooking.Patterns.h
    external/hooking/Hooking.Patterns.cpp
    external/injector/zydis/Zydis.h
    external/injector/zydis/Zydis.c
    external/injector/safetyhook/src/allocator.cpp
    external/injector/safetyhook/src/easy.cpp
    external/injector/safetyhook/src/inline_hook.cpp
    external/injector/safetyhook/src/mid_hook.cpp
    external/injector/safetyhook/src/os.windows.cpp
    external/injector/safetyhook/src/utility.cpp
    external/injector/safetyhook/src/vmt_hook.cpp
)

target_compile_features(ImprovedVehicleCam PRIVATE cxx_std_23)

target_compile_definitions(ImprovedVehicleCam PRIVATE
    NDEBUG
)

target_include_directories(ImprovedVehicleCam PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/injector/safetyhook/include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/hooking"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/injector/zydis"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/injector/safetyhook/include"
)

set_target_properties(ImprovedVehicleCam PROPERTIES
    PREFIX ""
    SUFFIX ".asi"
    OUTPUT_NAME "ImprovedVehicleCam"
)

if(MSVC)
    target_compile_options(ImprovedVehicleCam PRIVATE
        /W3
        /MT
    )

    target_link_options(ImprovedVehicleCam PRIVATE
        /MACHINE:X86
    )
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "ImprovedVehicleCam precisa ser compilado para 32 bits (x86).")
endif()

add_custom_command(TARGET ImprovedVehicleCam POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "[Camera]" > "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo ";Horizontal camera turn speed." >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo ";1 to 9 - Recommended is 8" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo "VehicleXTurnSpeed=8" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo "" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo ";Vertical camera turn speed" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo ";1 to 9 - Recommended is 5" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo "VehicleYTurnSpeed=5" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    VERBATIM
)
