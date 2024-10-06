cmake_minimum_required(VERSION 3.22)
project(pand)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    # v8 checks __cplusplus version
    add_compile_options(/Zc:__cplusplus)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:libcmt.lib")
    set(CMAKE_CXX_FLAGS_RELEASE "/MD /D_ITERATOR_DEBUG_LEVEL=0")
endif()

add_definitions(-DV8_ENABLE_SANDBOX=1 -DV8_COMPRESS_POINTERS=1)

add_executable(pand 
    core/main.cc
    core/loader.cc
    core/loader.h    
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
