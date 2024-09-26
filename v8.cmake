set(V8_DIR "${CMAKE_SOURCE_DIR}/deps/v8")
set(V8_TARGET "x64.release")
set(V8_BUILD_DIR "${V8_DIR}/out.gn/${V8_TARGET}")

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/deps/depot_tools")
    message(STATUS "Cloning depot_tools...")
    execute_process(
        COMMAND git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps
        RESULT_VARIABLE result
    )
    if(result)
        message(FATAL_ERROR "Failed to clone depot_tools.")
    endif()
endif()

if (WIN32)
    set(ENV{PATH} "$ENV{PATH};${CMAKE_SOURCE_DIR}/deps/depot_tools")
    set(ENV{DEPOT_TOOLS_WIN_TOOLCHAIN} "0")
else ()
    set(ENV{PATH} "$ENV{PATH}:${CMAKE_SOURCE_DIR}/deps/depot_tools")
endif ()

# message(STATUS "Path is: $ENV{PATH}")

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/deps/v8" AND WIN32)
    message(STATUS "Running 'fetch v8'...")
    execute_process(
        COMMAND cmd /c fetch v8
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps
        RESULT_VARIABLE result
    )
    if(result)
        message(FATAL_ERROR "Failed to fetch v8.")
    endif()
elseif (NOT EXISTS "${CMAKE_SOURCE_DIR}/deps/v8")
    message(STATUS "Running 'fetch v8'...")
    execute_process(
            COMMAND fetch v8
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps
            RESULT_VARIABLE result
    )
    if(result)
        message(FATAL_ERROR "Failed to fetch v8.")
    endif()
endif()

# Run gclient sync BEGIN
message(STATUS "Running 'gclient sync'...")
if(WIN32)
    execute_process(
      COMMAND cmd /c gclient sync
      WORKING_DIRECTORY ${V8_DIR}
      RESULT_VARIABLE result
    )
else ()
    execute_process(
        COMMAND gclient sync
        WORKING_DIRECTORY ${V8_DIR}
        RESULT_VARIABLE result
    )
endif ()
if(result)
    message(FATAL_ERROR "Failed to run 'gclient sync'.")
endif()
# Run gclient sync END

if(WIN32)
    execute_process(
        COMMAND python tools/dev/v8gen.py -vv ${V8_TARGET}
        WORKING_DIRECTORY ${V8_DIR}
        RESULT_VARIABLE result
    )
else()
    execute_process(
        COMMAND tools/dev/v8gen.py -vv ${V8_TARGET}
        WORKING_DIRECTORY ${V8_DIR}
        RESULT_VARIABLE result
    )
endif()
if(result)
    message(FATAL_ERROR "Failed to run 'v8gen.py'.")
endif()


file(WRITE ${V8_BUILD_DIR}/args.gn "dcheck_always_on = false
is_component_build = false
is_debug = false
target_cpu = \"x64\"
use_custom_libcxx = false
v8_monolithic = true
v8_use_external_startup_data = false
v8_enable_i18n_support = false
treat_warnings_as_errors = false")


message(STATUS "Compiling v8...")
execute_process(
    COMMAND ninja -C ${V8_BUILD_DIR} v8_monolith
    WORKING_DIRECTORY ${V8_DIR}
    RESULT_VARIABLE result
)
if(result)
    message(FATAL_ERROR "Failed to build V8 using Ninja.")
endif()
