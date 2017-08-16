cmake_minimum_required(VERSION 2.8.9)
project (mipos-exmaples)

set(CMAKE_C_FLAGS "-g -Wno-incompatible-pointer-types -m32 -DMIPOS_TARGET_SIMU -DENABLE_MIPOS_FS -DENABLE_MIPOS_CONSOLE -DENABLE_MIPOS_MALLOC -DENABLE_MIPOS_MPOOL")

include_directories(mipos)

add_executable(
  example-console
  examples/console/main.c
  mipos/mipos_bsp_simu.c
  mipos/mipos_console.c 
  mipos/mipos_fs.c 
  mipos/mipos_kernel.c 
  mipos/mipos_malloc.c 
  mipos/mipos_mpool.c 
  mipos/mipos_queue.c 
  mipos/mipos_stdio.c 
  mipos/mipos_task.c 
  mipos/mipos_timer.c )

add_executable(
  example-helloworld
  examples/helloworld/main.c
  mipos/mipos_bsp_simu.c
  mipos/mipos_console.c 
  mipos/mipos_fs.c 
  mipos/mipos_kernel.c 
  mipos/mipos_malloc.c 
  mipos/mipos_mpool.c 
  mipos/mipos_queue.c 
  mipos/mipos_stdio.c 
  mipos/mipos_task.c 
  mipos/mipos_timer.c )