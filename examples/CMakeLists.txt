cmake_minimum_required(VERSION 2.8.9)
project (mipos-exmaples)

set(CMAKE_C_FLAGS 
  "-g -Wno-incompatible-pointer-types -m32 \
  -DMIPOS_TARGET_SIMU \
  -DENABLE_MIPOS_FS \
  -DENABLE_MIPOS_STDIO \
  -DENABLE_MIPOS_CONSOLE \
  -DENABLE_MIPOS_MALLOC \
  -DENABLE_MIPOS_MPOOL")

include_directories(../mipos ../mipos/arch/simu ../fatfs)

add_library( mipos STATIC
  ../mipos/mipos_console.c 
  ../mipos/mipos_fs.c 
  ../mipos/mipos_kernel.c 
  ../mipos/mipos_malloc.c 
  ../mipos/mipos_mpool.c 
  ../mipos/mipos_queue.c 
  ../mipos/mipos_stdio.c 
  ../mipos/mipos_task.c 
  ../mipos/mipos_timer.c 
  ../mipos/mipos_diskio.c 
  ../mipos/arch/simu/mipos_bsp_simu.c
)

add_library( fatfs STATIC
  ../fatfs/diskio.c
  ../fatfs/ff.c
  ../fatfs/ff.h
  ../fatfs/ffsystem.c
  ../fatfs/ffunicode.c
)

add_executable( example-helloworld simu/helloworld/main.c)
target_link_libraries( example-helloworld mipos )

add_executable( example-timers simu/timers/main.c)
target_link_libraries( example-timers mipos )

add_executable( example-multitask simu/multitask/main.c)
target_link_libraries( example-multitask mipos )

add_executable( example-console simu/console/main.c)
target_link_libraries( example-console mipos )

add_executable( example-filesystem simu/filesystem/main.c)
target_link_libraries( example-filesystem mipos )

add_executable( example-fatfs simu/fatfs/main.c)
target_link_libraries( example-fatfs mipos fatfs )

add_custom_command(
        TARGET example-fatfs POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/simu/fatfs/disk.img
                ${CMAKE_CURRENT_BINARY_DIR}/disk.img)

