cmake_minimum_required(VERSION 3.10)
project(mipos-examples)

# Detect if the compiler is GCC or Clang and set flags
if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(COMPILER_FLAGS "-fno-common")
endif()


if(APPLE)
# Set the C flags
set(CMAKE_C_FLAGS 
  "-v -g -Wno-incompatible-pointer-types -Wno-pointer-sign \
  -fno-common -Wparentheses-equality\
  -DMIPOS_TARGET_SIMU \
  -DENABLE_MIPOS_FS \
  -DENABLE_MIPOS_STDIO \
  -DENABLE_MIPOS_CONSOLE \
  -DENABLE_MIPOS_MALLOC \
  -DENABLE_MIPOS_MPOOL"
)
# Set architecture to x86_64
set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(LINKER_FLAGS "-Wl,--warn-common")

else()
set(CMAKE_C_FLAGS 
  "-g -m32 -Wno-incompatible-pointer-types -Wno-pointer-sign \
  -fno-common -Wparentheses-equality\
  -DMIPOS_TARGET_SIMU \
  -DENABLE_MIPOS_FS \
  -DENABLE_MIPOS_STDIO \
  -DENABLE_MIPOS_CONSOLE \
  -DENABLE_MIPOS_MALLOC \
  -DENABLE_MIPOS_MPOOL"
)
endif()

# Include directories
include_directories(mipos mipos/arch/simu fatfs)

# MIPOS library
add_library(mipos STATIC
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

# FATFS library
add_library(fatfs STATIC
  fatfs/diskio.c
  fatfs/ff.c
  fatfs/ffsystem.c
  fatfs/ffunicode.c
)

# Add executables and link libraries
foreach(example IN ITEMS helloworld malloc timers multitask console filesystem fatfs)
  add_executable(example-${example} examples/simu/${example}/main.c)
  target_link_libraries(example-${example} mipos)
endforeach()

# Link fatfs library to example-fatfs
target_link_libraries(example-fatfs fatfs)

# Custom command for example-fatfs
add_custom_command(
  TARGET example-fatfs POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/simu/fatfs/disk.img
    ${CMAKE_CURRENT_BINARY_DIR}/disk.img
)

# Link against system libraries for macOS
if(APPLE)
  target_link_libraries(mipos PUBLIC "-framework CoreFoundation")
  target_link_libraries(mipos PUBLIC "-framework IOKit")
  target_link_libraries(mipos PUBLIC "-lSystem")
  
  # Apply additional linker flags
  set_target_properties(mipos PROPERTIES LINK_FLAGS "${LINKER_FLAGS}")
  
  # Use force_load flag to address duplicate symbols issue if necessary
  set_target_properties(mipos PROPERTIES LINK_FLAGS "-Wl,-force_load,libmipos.a")
endif()
