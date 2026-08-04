# Bare-metal ARM Cortex-A9 firmware for QEMU vexpress-a9.

if(NOT CMAKE_C_COMPILER_ID MATCHES "GNU")
    message(FATAL_ERROR "The vexpress-a9 port requires arm-none-eabi-gcc")
endif()

enable_language(ASM)

set(MIPOS_QEMU_VEXPRESS_A9_LINKER_SCRIPT
    ${CMAKE_CURRENT_SOURCE_DIR}/mipos/arch/qemu-vexpress-a9/linker_qemu_vexpress_a9.ld)

set(MIPOS_QEMU_VEXPRESS_A9_CPU_FLAGS -mcpu=cortex-a9 -marm)

set(MIPOS_QEMU_VEXPRESS_A9_CORE_SOURCES
    mipos/mipos_kernel.c
    mipos/mipos_task.c
    mipos/mipos_timer.c
    mipos/mipos_queue.c
    mipos/mipos_malloc.c
    mipos/mipos_mpool.c
    mipos/arch/qemu-vexpress-a9/mipos_bsp_qemu_vexpress_a9.c
    mipos/arch/qemu-vexpress-a9/startup_qemu_vexpress_a9.s
)

function(mipos_qemu_vexpress_a9_firmware name)
    cmake_parse_arguments(FW "" "MAIN" "SOURCES;DEFINITIONS" ${ARGN})

    add_executable(${name}
        ${FW_MAIN}
        ${MIPOS_QEMU_VEXPRESS_A9_CORE_SOURCES}
        ${FW_SOURCES}
    )

    set_target_properties(${name} PROPERTIES
        SUFFIX .elf
        LINK_DEPENDS ${MIPOS_QEMU_VEXPRESS_A9_LINKER_SCRIPT}
    )

    target_include_directories(${name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/mipos
        ${CMAKE_CURRENT_SOURCE_DIR}/mipos/arch/qemu-vexpress-a9
        ${CMAKE_CURRENT_SOURCE_DIR}/fatfs
    )

    target_compile_definitions(${name} PRIVATE
        MIPOS_TARGET_QEMU_VEXPRESS_A9=1
        ENABLE_MIPOS_MALLOC
        ENABLE_MIPOS_MPOOL
        ${FW_DEFINITIONS}
    )

    target_compile_options(${name} PRIVATE
        ${MIPOS_QEMU_VEXPRESS_A9_CPU_FLAGS}
        -ffreestanding
        -fno-builtin
        -ffunction-sections
        -fdata-sections
        -Wall
        -Wextra
        -Wno-incompatible-pointer-types
        -g
        -O0
    )

    target_link_options(${name} PRIVATE
        ${MIPOS_QEMU_VEXPRESS_A9_CPU_FLAGS}
        --specs=nosys.specs
        -nostartfiles
        -Wl,--gc-sections
        -Wl,-T${MIPOS_QEMU_VEXPRESS_A9_LINKER_SCRIPT}
        -Wl,-Map=${CMAKE_BINARY_DIR}/${name}.map
    )

    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:${name}> ${CMAKE_BINARY_DIR}/${name}.bin
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${name}>
    )
endfunction()

mipos_qemu_vexpress_a9_firmware(mipos-arm-qemu-vexpress-a9
    MAIN examples/qemu-vexpress-a9/helloworld/main.c
)
