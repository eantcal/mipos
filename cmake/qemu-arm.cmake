# Bare-metal ARM Cortex-M firmware for QEMU (board: lm3s6965evb).
#
# Produces, for each firmware <name>:
#   <name>.elf  - firmware image for qemu-system-arm -kernel
#   <name>.bin  - raw binary
#   <name>.map  - link map
#
# Run with:
#   qemu-system-arm -M lm3s6965evb -kernel <name>.elf -nographic
# or use scripts/run-qemu.ps1 / scripts/run-qemu.sh.

enable_language(ASM)

option(MIPOS_QEMU_ARM_NET_SELFTEST
       "Build the QEMU ARM lwIP ARP/ICMP self-test firmware"
       OFF)

set(MIPOS_QEMU_ARM_LINKER_SCRIPT
    ${CMAKE_CURRENT_SOURCE_DIR}/mipos/arch/qemu-arm/linker_qemu_arm.ld)

set(MIPOS_QEMU_ARM_CPU_FLAGS -mcpu=cortex-m3 -mthumb)

set(MIPOS_QEMU_ARM_CORE_SOURCES
    mipos/mipos_kernel.c
    mipos/mipos_task.c
    mipos/mipos_timer.c
    mipos/mipos_queue.c
    mipos/mipos_malloc.c
    mipos/mipos_mpool.c
    mipos/arch/qemu-arm/mipos_bsp_qemu_arm.c
    mipos/arch/qemu-arm/startup_qemu_arm.s
    mipos/arch/qemu-arm/vectors_qemu_arm.c
)

# mipos_qemu_arm_firmware(<name> MAIN <main.c>
#                         [SOURCES <extra sources...>]
#                         [DEFINITIONS <extra compile definitions...>])
function(mipos_qemu_arm_firmware name)
    cmake_parse_arguments(FW "" "MAIN" "SOURCES;DEFINITIONS" ${ARGN})

    add_executable(${name}
        ${FW_MAIN}
        ${MIPOS_QEMU_ARM_CORE_SOURCES}
        ${FW_SOURCES}
    )

    set_target_properties(${name} PROPERTIES
        SUFFIX .elf
        LINK_DEPENDS ${MIPOS_QEMU_ARM_LINKER_SCRIPT}
    )

    target_include_directories(${name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/mipos
        ${CMAKE_CURRENT_SOURCE_DIR}/mipos/arch/qemu-arm
        ${CMAKE_CURRENT_SOURCE_DIR}/fatfs
    )

    target_compile_definitions(${name} PRIVATE
        MIPOS_TARGET_QEMU_ARM=1
        ENABLE_MIPOS_MALLOC
        ENABLE_MIPOS_MPOOL
        ${FW_DEFINITIONS}
    )

    target_compile_options(${name} PRIVATE
        ${MIPOS_QEMU_ARM_CPU_FLAGS}
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

    # newlib provides memcpy/memset, sprintf, and setjmp/longjmp (used for
    # the cooperative context switch); -nostartfiles keeps our own startup
    # code and vector table.
    target_link_options(${name} PRIVATE
        ${MIPOS_QEMU_ARM_CPU_FLAGS}
        --specs=nosys.specs
        -nostartfiles
        -Wl,--gc-sections
        -Wl,-T${MIPOS_QEMU_ARM_LINKER_SCRIPT}
        -Wl,-Map=${CMAKE_BINARY_DIR}/${name}.map
    )

    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:${name}> ${CMAKE_BINARY_DIR}/${name}.bin
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${name}>
    )
endfunction()

# Hello-world firmware: boots mipOS and prints a tick every second
mipos_qemu_arm_firmware(mipos-arm-qemu
    MAIN examples/qemu-arm/helloworld/main.c
)

# Console firmware: interactive mipOS CLI on the emulated UART
mipos_qemu_arm_firmware(mipos-arm-qemu-console
    MAIN examples/qemu-arm/console/main.c
    SOURCES mipos/mipos_console.c
    DEFINITIONS ENABLE_MIPOS_CONSOLE
)

# Filesystem firmware: interactive console with a RAM-backed mipOS tiny FS
mipos_qemu_arm_firmware(mipos-arm-qemu-filesystem
    MAIN examples/qemu-arm/filesystem/main.c
    SOURCES
        mipos/mipos_console.c
        mipos/mipos_tfs.c
    DEFINITIONS
        ENABLE_MIPOS_CONSOLE
        ENABLE_MIPOS_FS
)

# FatFs firmware: host-file-backed block device via QEMU semihosting.
mipos_qemu_arm_firmware(mipos-arm-qemu-fatfs
    MAIN examples/qemu-arm/fatfs/main.c
    SOURCES
        mipos/mipos_console.c
        mipos/mipos_diskio.c
        mipos/arch/qemu-arm/mipos_qemu_semihosting.c
        mipos/arch/qemu-arm/mipos_qemu_hostdisk.c
        fatfs/diskio.c
        fatfs/ff.c
        fatfs/ffsystem.c
        fatfs/ffunicode.c
    DEFINITIONS
        ENABLE_MIPOS_CONSOLE
)

if(MIPOS_QEMU_ARM_NET_SELFTEST)
    include(cmake/lwip-fetch.cmake)

    mipos_qemu_arm_firmware(mipos-arm-qemu-net-selftest
        MAIN examples/qemu-arm/net-selftest/main.c
        SOURCES
            mipos/net/lwip/mipos_lwip_netif.c
            ${MIPOS_LWIP_SOURCES}
        DEFINITIONS
            MIPOS_NET_STACK_LWIP=1
    )

    target_include_directories(mipos-arm-qemu-net-selftest PRIVATE
        ${MIPOS_LWIP_INCLUDE_DIRS}
    )

    target_compile_options(mipos-arm-qemu-net-selftest PRIVATE
        -Wno-unused-parameter
        -Wno-sign-compare
    )
endif()
