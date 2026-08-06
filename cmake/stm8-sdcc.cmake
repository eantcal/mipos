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

set(MIPOS_STM8_COMMON_SOURCES
    mipos/mipos_kernel.c
    mipos/mipos_task.c
    mipos/mipos_timer.c
    mipos/arch/stm8/mipos_bsp_stm8s.c
    mipos/arch/stm8/stm8s_lib/src/stm8s_tim4.c
    mipos/arch/stm8/stm8s_lib/src/stm8s_clk.c
    mipos/arch/stm8/stm8s_lib/src/stm8s_rst.c
    mipos/arch/stm8/stm8s_lib/src/stm8s_iwdg.c
    mipos/arch/stm8/stm8s_lib/src/stm8s_uart1.c
)

function(mipos_add_stm8_sdcc_image target_name output_basename)
    set(options)
    set(one_value_args MAIN)
    set(multi_value_args SOURCES DEFINITIONS)
    cmake_parse_arguments(ARG "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    set(image_sources
        ${ARG_MAIN}
        ${MIPOS_STM8_COMMON_SOURCES}
        ${ARG_SOURCES}
    )

    set(image_objects)
    foreach(source IN LISTS image_sources)
        string(REPLACE "/" "_" object_stem "${target_name}_${source}")
        string(REPLACE "\\" "_" object_stem "${object_stem}")
        string(REGEX REPLACE "\\.c$" "" object_stem "${object_stem}")
        set(object "${MIPOS_STM8_BUILD_DIR}/${object_stem}.rel")
        list(APPEND image_objects "${object}")

        add_custom_command(
            OUTPUT "${object}"
            COMMAND "${MIPOS_SDCC}"
                -mstm8
                -c
                ${MIPOS_STM8_INCLUDES}
                ${MIPOS_STM8_DEFINITIONS}
                ${ARG_DEFINITIONS}
                ${MIPOS_STM8_SDCC_FLAGS}
                "${CMAKE_CURRENT_SOURCE_DIR}/${source}"
                -o "${object}"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${source}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            VERBATIM
        )
    endforeach()

    set(image "${MIPOS_STM8_BUILD_DIR}/${output_basename}.ihx")
    add_custom_command(
        OUTPUT "${image}"
        COMMAND "${MIPOS_SDCC}"
            -mstm8
            --out-fmt-ihx
            -lstm8
            -o "${image}"
            ${image_objects}
        DEPENDS ${image_objects}
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        VERBATIM
    )

    add_custom_target(${target_name} DEPENDS "${image}")
    set(${target_name}_IMAGE "${image}" PARENT_SCOPE)
endfunction()

mipos_add_stm8_sdcc_image(stm8-sdcc-helloworld stm8s_example
    MAIN examples/stm8/main.c
)

mipos_add_stm8_sdcc_image(stm8-sdcc-console stm8s_console
    MAIN examples/stm8/console/main.c
    SOURCES
        mipos/mipos_console.c
        mipos/mipos_queue.c
    DEFINITIONS
        -DENABLE_MIPOS_CONSOLE=1
        -DMIPOS_STM8_UCSIM=1
        -DMIPOS_CONSOLE_TX_STACK=512
        -DMIPOS_CONSOLE_RX_STACK=512
        -DRS232_TX_QUEUE_LEN=4
        -DRS232_TX_BUF_LEN=64
        -DRS232_RX_BUF_LEN=32
)

add_custom_target(stm8-sdcc-all ALL
    DEPENDS stm8-sdcc-helloworld stm8-sdcc-console
)

if(MIPOS_STM8_SIMULATOR)
    add_custom_target(stm8-sdcc-sim
        COMMAND "${MIPOS_STM8_SIMULATOR}"
            -p "sstm8> "
            "${stm8-sdcc-helloworld_IMAGE}"
        DEPENDS stm8-sdcc-helloworld
        USES_TERMINAL
        VERBATIM
    )

    add_custom_target(stm8-sdcc-sim-console
        COMMAND "${MIPOS_STM8_SIMULATOR}"
            -g
            -X 16M
            -Z 7778
            -S uart=1,port=7777,raw
            -e "set error stack off"
            -e "pc 0x8010"
            "${stm8-sdcc-console_IMAGE}"
        DEPENDS stm8-sdcc-console
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

    add_custom_target(stm8-sdcc-sim-console
        COMMAND "${CMAKE_COMMAND}" -E echo
            "ucsim_stm8/sstm8 was not found. Install sdcc-ucsim to run the STM8 console image."
        DEPENDS stm8-sdcc-console
        VERBATIM
    )
endif()

message(STATUS "STM8 SDCC helloworld image: ${stm8-sdcc-helloworld_IMAGE}")
message(STATUS "STM8 SDCC console image: ${stm8-sdcc-console_IMAGE}")
