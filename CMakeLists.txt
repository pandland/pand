cmake_minimum_required(VERSION 3.22)
project(pand)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# In the newer v8 versions, lld may be required:
#set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")

add_definitions(-DV8_ENABLE_SANDBOX=1 -DV8_COMPRESS_POINTERS=1)

add_executable(pand 
    core/pand.h
    core/pand.cc
    core/errors.h
    core/errors.cc
    core/loader.h
    core/loader.cc
    core/runtime.h
    core/runtime.cc
    core/timer.h
    core/timer.cc
    core/bytes/extern.h
    core/bytes/hex.h
    core/bytes/transcoder.h
    core/bytes/transcoder.cc
    core/buffer.h
    core/buffer.cc
    core/writer.h
    core/tcp.h
    core/tcp.cc
    core/tcp_server.h
    core/tcp_server.cc
    core/main.cc
    core/fs.h
    core/fs.cc
)

target_include_directories(pand
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    )

# Our cmake deps
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/pandio pandio_build)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/ada ada_build)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/simdutf simdutf_build)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/fmt)

target_link_directories(pand PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/out.gn/x64.release/obj)

target_link_libraries(pand fmt::fmt)

if(WIN32)
    target_link_libraries(pand
            pandio
            ada
            simdutf
            v8_monolith.lib
            Advapi32.lib
            Winmm.lib
            Dbghelp.lib
    )
else ()
    target_link_libraries(pand
            pandio
            ada
            simdutf
            v8_libplatform
            v8_libbase 
            v8_monolith
            pthread
            rt
            dl
    )
    target_link_options(pand PRIVATE -pthread -lrt -ldl)
endif()
