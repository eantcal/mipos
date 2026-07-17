include(cmake/lwip-fetch.cmake)

add_library(lwipcore STATIC ${MIPOS_LWIP_SOURCES})
target_include_directories(lwipcore
    PUBLIC
        ${MIPOS_LWIP_INCLUDE_DIRS}
)
target_compile_definitions(lwipcore PUBLIC MIPOS_NET_STACK_LWIP=1)

if(MSVC)
    target_compile_definitions(lwipcore PUBLIC _CRT_SECURE_NO_WARNINGS)
    target_compile_options(lwipcore PRIVATE /W3)
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(lwipcore PRIVATE
        -Wno-unused-parameter
        -Wno-sign-compare
    )
endif()

add_library(mipos_lwip STATIC
    mipos/net/lwip/mipos_lwip_netif.c
)
target_include_directories(mipos_lwip
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mipos
        ${CMAKE_CURRENT_SOURCE_DIR}/mipos/net/lwip
)
target_link_libraries(mipos_lwip PUBLIC mipos lwipcore)
target_compile_definitions(mipos_lwip PUBLIC MIPOS_NET_STACK_LWIP=1)
