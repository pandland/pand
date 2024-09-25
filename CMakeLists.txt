cmake_minimum_required(VERSION 3.22)
project(pand)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(
    ./deps/v8/include
    ./deps/swcc/include
    ./deps/luxio/src
)

link_directories(
    ./deps/v8/out.gn/x64.release.sample/obj
    ./deps/luxio/build
    ./deps/swcc/target/release
)

add_definitions(-DV8_ENABLE_SANDBOX=1 -DV8_COMPRESS_POINTERS=1)

add_executable(pand core/main.cc)

target_link_libraries(pand
    swcc
    luxio
    v8_libplatform
    v8_monolith
    pthread
    rt
    dl
)

target_link_options(pand PRIVATE -pthread -lrt -ldl)
