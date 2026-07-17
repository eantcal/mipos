cmake_minimum_required(VERSION 3.20)

option(MIPOS_TARGET_QEMU_ARM
       "Build the bare-metal ARM firmware for QEMU instead of the host simulator"
       OFF)

if(MIPOS_TARGET_QEMU_ARM)
    # Cross build: requires the arm-none-eabi toolchain, e.g.
    #   cmake -S . -B build-qemu-arm \
    #         -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-toolchain.cmake \
    #         -DMIPOS_TARGET_QEMU_ARM=ON
    project(mipos-arm-qemu LANGUAGES C ASM)

    set(CMAKE_C_STANDARD 99)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_C_EXTENSIONS OFF)

    include(cmake/qemu-arm.cmake)
    return()
endif()

set(MIPOS_ARCH "native" CACHE STRING "Simulator architecture: native, x86, or x64")
set_property(CACHE MIPOS_ARCH PROPERTY STRINGS native x86 x64)

if(APPLE)
    if(MIPOS_ARCH STREQUAL "x86")
        set(CMAKE_OSX_ARCHITECTURES i386 CACHE STRING "" FORCE)
    elseif(MIPOS_ARCH STREQUAL "x64")
        set(CMAKE_OSX_ARCHITECTURES x86_64 CACHE STRING "" FORCE)
    endif()
elseif(NOT CMAKE_GENERATOR MATCHES "Visual Studio")
    if(MIPOS_ARCH STREQUAL "x86")
        set(CMAKE_C_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} -m32")
        set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} -m32")
        set(CMAKE_ASM_FLAGS_INIT "${CMAKE_ASM_FLAGS_INIT} -m32")
        set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -m32")
        set(CMAKE_SHARED_LINKER_FLAGS_INIT "${CMAKE_SHARED_LINKER_FLAGS_INIT} -m32")
    elseif(MIPOS_ARCH STREQUAL "x64")
        set(CMAKE_C_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} -m64")
        set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} -m64")
        set(CMAKE_ASM_FLAGS_INIT "${CMAKE_ASM_FLAGS_INIT} -m64")
        set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -m64")
        set(CMAKE_SHARED_LINKER_FLAGS_INIT "${CMAKE_SHARED_LINKER_FLAGS_INIT} -m64")
    endif()
endif()

project(mipos-examples LANGUAGES C CXX)

include(CTest)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(MIPOS_HOST_APPLE_ARM64 OFF)
if(APPLE AND
   (CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|ARM64|AARCH64" OR
    CMAKE_OSX_ARCHITECTURES MATCHES "arm64|aarch64|ARM64|AARCH64"))
    set(MIPOS_HOST_APPLE_ARM64 ON)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MIPOS_BUILD_EXAMPLES "Build mipOS simulator examples" ON)
option(MIPOS_BUILD_TESTS "Build mipOS tests" ${BUILD_TESTING})
option(MIPOS_ADD_QEMU_RUN_TARGETS
       "Add Visual Studio targets that run QEMU ARM examples"
       ${WIN32})
set(MIPOS_NET_STACK "none" CACHE STRING "Optional network stack: none or lwip")
set_property(CACHE MIPOS_NET_STACK PROPERTY STRINGS none lwip)
set(MIPOS_NET_BACKEND "none" CACHE STRING "Optional network backend: none, pcap, tap, or all")
set_property(CACHE MIPOS_NET_BACKEND PROPERTY STRINGS none pcap tap all)

set(MIPOS_PUBLIC_DEFINITIONS
    MIPOS_TARGET_SIMU
    ENABLE_MIPOS_FS
    ENABLE_MIPOS_STDIO
    ENABLE_MIPOS_CONSOLE
    ENABLE_MIPOS_MALLOC
    ENABLE_MIPOS_MPOOL
)

set(MIPOS_SOURCES
    mipos/mipos_console.c
    mipos/mipos_tfs.c
    mipos/mipos_kernel.c
    mipos/mipos_malloc.c
    mipos/mipos_mpool.c
    mipos/mipos_queue.c
    mipos/mipos_stdio.c
    mipos/mipos_task.c
    mipos/mipos_timer.c
    mipos/mipos_diskio.c
    mipos/arch/simu/mipos_bsp_simu.c
)

add_library(mipos STATIC ${MIPOS_SOURCES})
target_include_directories(mipos
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mipos
        ${CMAKE_CURRENT_SOURCE_DIR}/mipos/arch/simu
)
target_compile_definitions(mipos PUBLIC ${MIPOS_PUBLIC_DEFINITIONS})

if(MSVC)
    target_compile_definitions(mipos PUBLIC _CRT_SECURE_NO_WARNINGS)
    target_compile_options(mipos PRIVATE /W3)

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        enable_language(ASM_MASM)
        target_sources(mipos PRIVATE mipos/arch/simu/bspX64.asm)
    endif()
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_definitions(mipos PUBLIC _DEFAULT_SOURCE)
    target_compile_options(mipos PRIVATE
        -fno-common
        -Wno-incompatible-pointer-types
        -Wno-pointer-sign
    )

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        enable_language(ASM)
        target_sources(mipos PRIVATE mipos/arch/simu/bsp_gnu.S)
        # The red zone must be disabled where the kernel switches stacks;
        # the flag only exists for x86-64 targets.
        if(MIPOS_ARCH STREQUAL "x64" OR
           CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
            target_compile_options(mipos PRIVATE -mno-red-zone)
        endif()
    endif()
endif()

if(APPLE)
    target_compile_options(mipos PRIVATE -Wparentheses-equality)
    target_link_libraries(mipos PUBLIC
        "-framework CoreFoundation"
        "-framework IOKit"
        "-lSystem"
    )
endif()

add_library(fatfs STATIC
    fatfs/diskio.c
    fatfs/ff.c
    fatfs/ffsystem.c
    fatfs/ffunicode.c
)
target_include_directories(fatfs
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/fatfs
        ${CMAKE_CURRENT_SOURCE_DIR}/mipos
)
target_link_libraries(fatfs PUBLIC mipos)

if(MSVC)
    target_compile_definitions(fatfs PUBLIC _CRT_SECURE_NO_WARNINGS)
    target_compile_options(fatfs PRIVATE /W3)
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(fatfs PRIVATE
        -fno-common
        -Wno-incompatible-pointer-types
        -Wno-pointer-sign
    )
endif()

if(MIPOS_NET_STACK STREQUAL "lwip")
    include(cmake/lwip.cmake)
elseif(NOT MIPOS_NET_STACK STREQUAL "none")
    message(FATAL_ERROR
        "Unsupported MIPOS_NET_STACK='${MIPOS_NET_STACK}' (use none or lwip)")
endif()

if(NOT MIPOS_NET_BACKEND STREQUAL "none" AND NOT TARGET mipos_lwip)
    message(FATAL_ERROR
        "MIPOS_NET_BACKEND='${MIPOS_NET_BACKEND}' requires MIPOS_NET_STACK=lwip")
elseif(NOT MIPOS_NET_BACKEND STREQUAL "none" AND
       NOT MIPOS_NET_BACKEND STREQUAL "pcap" AND
       NOT MIPOS_NET_BACKEND STREQUAL "tap" AND
       NOT MIPOS_NET_BACKEND STREQUAL "all")
    message(FATAL_ERROR
        "Unsupported MIPOS_NET_BACKEND='${MIPOS_NET_BACKEND}' (use none, pcap, tap, or all)")
endif()

if(MIPOS_BUILD_EXAMPLES)
    foreach(example IN ITEMS helloworld malloc timers multitask console filesystem fatfs)
        add_executable(example-${example} examples/simu/${example}/main.c)
        target_link_libraries(example-${example} PRIVATE mipos)
    endforeach()

    target_link_libraries(example-fatfs PRIVATE fatfs)

    set_target_properties(example-fatfs PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:example-fatfs>")

    add_custom_command(
        TARGET example-fatfs POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/examples/simu/fatfs/disk.img
            $<TARGET_FILE_DIR:example-fatfs>/disk.img
    )

    if(MIPOS_NET_BACKEND STREQUAL "pcap" OR MIPOS_NET_BACKEND STREQUAL "all")
        add_executable(example-net-pcap examples/simu/net_pcap/main.c)
        target_link_libraries(example-net-pcap PRIVATE mipos_lwip)

        if(WIN32)
            target_compile_definitions(example-net-pcap PRIVATE WIN32_LEAN_AND_MEAN)
        elseif(UNIX)
            find_path(MIPOS_PCAP_INCLUDE_DIR
                NAMES pcap/pcap.h
            )
            find_library(MIPOS_PCAP_LIBRARY
                NAMES pcap
            )

            if(NOT MIPOS_PCAP_INCLUDE_DIR OR NOT MIPOS_PCAP_LIBRARY)
                message(FATAL_ERROR
                    "The pcap example requires libpcap development files "
                    "(for example: sudo apt install libpcap-dev)")
            endif()

            target_include_directories(example-net-pcap PRIVATE
                ${MIPOS_PCAP_INCLUDE_DIR})
            target_link_libraries(example-net-pcap PRIVATE
                ${MIPOS_PCAP_LIBRARY})
        else()
            message(FATAL_ERROR
                "The pcap example currently supports Windows/Npcap and Linux/libpcap only")
        endif()
    endif()

    if(MIPOS_NET_BACKEND STREQUAL "tap" OR MIPOS_NET_BACKEND STREQUAL "all")
        if(WIN32)
            add_executable(example-net-tap examples/simu/net_tap/main.c)
            target_link_libraries(example-net-tap PRIVATE mipos_lwip)
            target_compile_definitions(example-net-tap PRIVATE WIN32_LEAN_AND_MEAN)
        elseif(UNIX)
            add_executable(example-net-tap examples/simu/net_tap/main.c)
            target_link_libraries(example-net-tap PRIVATE mipos_lwip)
        else()
            message(FATAL_ERROR
                "The TAP example currently supports Windows/TAP-Windows and Linux TAP only")
        endif()
    endif()
endif()

if(MIPOS_ADD_QEMU_RUN_TARGETS)
    if(WIN32)
        find_program(MIPOS_POWERSHELL
            NAMES pwsh powershell
        )

        if(MIPOS_POWERSHELL)
            function(mipos_add_qemu_run_target name firmware)
                add_custom_target(${name}
                    COMMAND ${MIPOS_POWERSHELL}
                        -NoLogo
                        -NoProfile
                        -ExecutionPolicy Bypass
                        -File "${CMAKE_CURRENT_SOURCE_DIR}/scripts/run-qemu.ps1"
                        ${firmware}
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    USES_TERMINAL
                    VERBATIM
                )
                set_target_properties(${name} PROPERTIES
                    FOLDER "QEMU Examples")
            endfunction()

            mipos_add_qemu_run_target(qemu-filesystem filesystem)
            mipos_add_qemu_run_target(qemu-console console)
            mipos_add_qemu_run_target(qemu-fatfs fatfs)
            mipos_add_qemu_run_target(qemu-helloworld helloworld)
        else()
            message(WARNING
                "PowerShell not found: QEMU run targets will not be added")
        endif()
    else()
        function(mipos_add_qemu_run_target name firmware)
            add_custom_target(${name}
                COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/run-qemu.sh"
                    ${firmware}
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                USES_TERMINAL
                VERBATIM
            )
            set_target_properties(${name} PROPERTIES
                FOLDER "QEMU Examples")
        endfunction()

        mipos_add_qemu_run_target(qemu-filesystem filesystem)
        mipos_add_qemu_run_target(qemu-console console)
        mipos_add_qemu_run_target(qemu-fatfs fatfs)
        mipos_add_qemu_run_target(qemu-helloworld helloworld)
    endif()
endif()

if(MIPOS_BUILD_TESTS)
    include(FetchContent)

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(mipos_tests
        tests/mipos_queue_tests.cpp
        tests/mipos_mpool_tests.cpp
        tests/mipos_malloc_tests.cpp
    )
    target_link_libraries(mipos_tests PRIVATE mipos GTest::gtest_main)

    add_executable(mipos_scheduler_smoke tests/mipos_scheduler_smoke.c)
    target_link_libraries(mipos_scheduler_smoke PRIVATE mipos)

    if(TARGET mipos_lwip)
        add_executable(mipos_lwip_tests tests/mipos_lwip_tests.cpp)
        target_link_libraries(mipos_lwip_tests
            PRIVATE
                mipos_lwip
                GTest::gtest_main
        )
    endif()

    include(GoogleTest)
    gtest_discover_tests(mipos_tests)
    if(TARGET mipos_lwip_tests)
        gtest_discover_tests(mipos_lwip_tests)
    endif()

    if(NOT MIPOS_HOST_APPLE_ARM64)
        add_test(NAME mipos_scheduler_smoke COMMAND mipos_scheduler_smoke)
        set_tests_properties(mipos_scheduler_smoke PROPERTIES TIMEOUT 5)
    else()
        message(STATUS
            "Skipping mipos_scheduler_smoke on macOS ARM64: simulator "
            "context switching is not supported on this host architecture")
    endif()
endif()
