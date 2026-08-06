# STM8/SDCC firmware build.
#
# This build intentionally uses custom commands instead of asking CMake to
# treat SDCC as a normal hosted C compiler. The target is a freestanding STM8
# image, and keeping the SDCC invocation explicit makes the generated files and
# simulator scripts predictable across Windows, Linux, and CI.

find_program(MIPOS_SDCC sdcc)
if(NOT MIPOS_SDCC)
    message(FATAL_ERROR
        "SDCC was not found. Install SDCC with STM8 support, then reconfigure.")
endif()

find_program(MIPOS_STM8_SIMULATOR NAMES ucsim_stm8 sstm8)

set(MIPOS_STM8_MCU_MODEL "STM8S208" CACHE STRING
    "STM8S/A device define used by the SDCC build")
set(MIPOS_STM8_OUTPUT_BASENAME "stm8s_example" CACHE STRING
    "Base name for the generated STM8 firmware image")
set(MIPOS_STM8_SDCC_FLAGS "--std-c99;--opt-code-size" CACHE STRING
    "Additional SDCC flags for the STM8 build")

set(MIPOS_STM8_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/stm8-sdcc")
file(MAKE_DIRECTORY "${MIPOS_STM8_BUILD_DIR}")

set(MIPOS_STM8_INCLUDES
    -I${CMAKE_CURRENT_SOURCE_DIR}/mipos
    -I${CMAKE_CURRENT_SOURCE_DIR}/mipos/arch/stm8
    -I${CMAKE_CURRENT_SOURCE_DIR}/mipos/arch/stm8/stm8s_lib/inc
)

set(MIPOS_STM8_DEFINITIONS
    -DMIPOS_TARGET_STM8=1
    -D${MIPOS_STM8_MCU_MODEL}
)

set(MIPOS_STM8_SOURCES
    examples/stm8/main.c
    mipos/mipos_kernel.c
    mipos/mipos_task.c
    mipos/arch/stm8/mipos_bsp_stm8s.c
    mipos/arch/stm8/stm8s_lib/src/stm8s_tim4.c
    mipos/arch/stm8/stm8s_lib/src/stm8s_clk.c
    mipos/arch/stm8/stm8s_lib/src/stm8s_rst.c
    mipos/arch/stm8/stm8s_lib/src/stm8s_iwdg.c
)

set(MIPOS_STM8_OBJECTS)
foreach(source IN LISTS MIPOS_STM8_SOURCES)
    string(REPLACE "/" "_" object_stem "${source}")
    string(REPLACE "\\" "_" object_stem "${object_stem}")
    string(REGEX REPLACE "\\.c$" "" object_stem "${object_stem}")
    set(object "${MIPOS_STM8_BUILD_DIR}/${object_stem}.rel")
    list(APPEND MIPOS_STM8_OBJECTS "${object}")

    add_custom_command(
        OUTPUT "${object}"
        COMMAND "${MIPOS_SDCC}"
            -mstm8
            -c
            ${MIPOS_STM8_INCLUDES}
            ${MIPOS_STM8_DEFINITIONS}
            ${MIPOS_STM8_SDCC_FLAGS}
            "${CMAKE_CURRENT_SOURCE_DIR}/${source}"
            -o "${object}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${source}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        VERBATIM
    )
endforeach()

set(MIPOS_STM8_IHX
    "${MIPOS_STM8_BUILD_DIR}/${MIPOS_STM8_OUTPUT_BASENAME}.ihx")

add_custom_command(
    OUTPUT "${MIPOS_STM8_IHX}"
    COMMAND "${MIPOS_SDCC}"
        -mstm8
        --out-fmt-ihx
        -lstm8
        -o "${MIPOS_STM8_IHX}"
        ${MIPOS_STM8_OBJECTS}
    DEPENDS ${MIPOS_STM8_OBJECTS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    VERBATIM
)

add_custom_target(stm8-sdcc-helloworld ALL
    DEPENDS "${MIPOS_STM8_IHX}"
)


if(MIPOS_STM8_SIMULATOR)
    add_custom_target(stm8-sdcc-sim
        COMMAND "${MIPOS_STM8_SIMULATOR}"
            -p "sstm8> "
            "${MIPOS_STM8_IHX}"
        DEPENDS stm8-sdcc-helloworld
        USES_TERMINAL
        VERBATIM
    )
else()
    add_custom_target(stm8-sdcc-sim
        COMMAND "${CMAKE_COMMAND}" -E echo
            "ucsim_stm8/sstm8 was not found. Install sdcc-ucsim to run the STM8 image."
        DEPENDS stm8-sdcc-helloworld
        VERBATIM
    )
endif()

message(STATUS "STM8 SDCC image: ${MIPOS_STM8_IHX}")