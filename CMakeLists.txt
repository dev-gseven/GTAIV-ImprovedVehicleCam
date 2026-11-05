
cmake_minimum_required(VERSION 3.21)

project(ImprovedVehicleCam LANGUAGES C CXX)

add_library(ImprovedVehicleCam SHARED
    src/main.cpp
    src/config/Config.h
    src/config/Config.cpp
    src/camera/CoverCentering.h
    src/camera/CoverCentering.cpp
    src/camera/VehicleStuntJump.h
    src/camera/VehicleStuntJump.cpp
    src/camera/VehicleTheftCamera.h
    src/camera/VehicleTheftCamera.cpp
    src/camera/VehicleTurnSpeed.h
    src/camera/VehicleTurnSpeed.cpp
    src/utils/HookUtils.h

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
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    NDEBUG
)

target_include_directories(ImprovedVehicleCam PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/camera"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utils"

    "${CMAKE_CURRENT_SOURCE_DIR}/external/injector/safetyhook/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/hooking"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/injector/zydis"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/injector/injector"
)

set_target_properties(ImprovedVehicleCam PROPERTIES
    PREFIX ""
    SUFFIX ".asi"
    OUTPUT_NAME "ImprovedVehicleCam"
)

if(MSVC)
    target_compile_options(ImprovedVehicleCam PRIVATE
        /W3
        /O2
        /MT
    )

    target_link_options(ImprovedVehicleCam PRIVATE
        /MACHINE:X86
    )
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "ImprovedVehicleCam needs to be compiled in 32 bits (x86).")
endif()

add_custom_command(TARGET ImprovedVehicleCam POST_BUILD
    # Create .ini file
    COMMAND ${CMAKE_COMMAND} -E echo "[Camera]" > "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo ";Horizontal camera turn speed." >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo ";1 to 9 - Default is 8" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo "VehicleXTurnSpeed=8" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo "" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo ";Vertical camera turn speed." >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo ";1 to 9 - Default is 6" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo "VehicleYTurnSpeed=6" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo "" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo ";Vehicle Theft Camera." >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo ";0 or 1 - Default is 0" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo "VehicleTheftCam=0" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo "" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo ";Vehicle Stunt Camera." >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo ";0 or 1 - Default is 0" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo "VehicleStuntJump=0" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo "" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo ";Camera Center in Cover." >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo ";0 or 1 - Default is 0" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    COMMAND ${CMAKE_COMMAND} -E echo "CameraCenterInCover=0" >> "$<TARGET_FILE_DIR:ImprovedVehicleCam>/ImprovedVehicleCam.ini"
    # Copy licenses folder
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/licenses" "$<TARGET_FILE_DIR:ImprovedVehicleCam>/licenses"
    VERBATIM
)
