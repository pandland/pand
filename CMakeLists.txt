cmake_minimum_required(VERSION 3.22)
project(pand)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-DV8_ENABLE_SANDBOX=1 -DV8_COMPRESS_POINTERS=1)

add_executable(pand 
    core/pand.h
    core/pand.cc
    core/mod.h
    core/mod.cc
    core/runtime.h
    core/runtime.cc
    core/errors.h
    core/errors.cc
    core/timer.h
    core/timer.cc
    core/tcp.h
    core/tcp.cc
    core/tcp_server.h
    core/tcp_server.cc
    core/main.cc
)

target_include_directories(pand SYSTEM
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/swcc/include)

# Our cmake deps
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/pandio pandio_build)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/ada ada_build)

target_link_directories(pand
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/out.gn/x64.release/obj
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/swcc/target/release)


if(WIN32)
    target_link_libraries(pand
            pandio
            ada
            v8_monolith.lib
            Advapi32.lib
            Winmm.lib
            Dbghelp.lib
    )
else ()
    target_link_libraries(pand
            pandio
            ada
            v8_libplatform
            v8_monolith
            pthread
            rt
            dl
    )
    target_link_options(pand PRIVATE -pthread -lrt -ldl)
endif()
