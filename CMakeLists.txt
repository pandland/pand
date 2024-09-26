cmake_minimum_required(VERSION 3.22)
project(pand)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-DV8_ENABLE_SANDBOX=1 -DV8_COMPRESS_POINTERS=1)

add_executable(pand core/main.cc)

target_include_directories(pand
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/swcc/include)

# Add pandio library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/pandio pandio_build)

target_link_directories(pand
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8/out.gn/x64.release.sample/obj
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/swcc/target/release)


target_link_libraries(pand
    swcc
    pandio
    v8_libplatform
    v8_monolith
    pthread
    rt
    dl
)

target_link_options(pand PRIVATE -pthread -lrt -ldl)
