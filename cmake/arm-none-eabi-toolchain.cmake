# CMake toolchain file for the bare-metal ARM (arm-none-eabi) builds.
#
# Usage:
#   cmake -S . -B build-qemu-arm \
#         -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-toolchain.cmake \
#         -DMIPOS_TARGET_QEMU_ARM=ON

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

find_program(ARM_NONE_EABI_GCC arm-none-eabi-gcc REQUIRED)
find_program(ARM_NONE_EABI_OBJCOPY arm-none-eabi-objcopy REQUIRED)
find_program(ARM_NONE_EABI_SIZE arm-none-eabi-size REQUIRED)

set(CMAKE_C_COMPILER ${ARM_NONE_EABI_GCC})
set(CMAKE_ASM_COMPILER ${ARM_NONE_EABI_GCC})
set(CMAKE_OBJCOPY ${ARM_NONE_EABI_OBJCOPY})
set(CMAKE_SIZE ${ARM_NONE_EABI_SIZE})

# The compiler check cannot link a host executable
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
