set(V8_DIR "${CMAKE_SOURCE_DIR}/deps/v8")
set(V8_TARGET "x64.release.sample")
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

set(ENV{PATH} "$ENV{PATH}:${CMAKE_SOURCE_DIR}/deps/depot_tools")

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/deps/v8")
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

message(STATUS "Running 'gclient sync'...")
execute_process(
  COMMAND gclient sync
  WORKING_DIRECTORY ${V8_DIR}
  RESULT_VARIABLE result
)
if(result)
  message(FATAL_ERROR "Failed to run 'gclient sync'.")
endif()

execute_process(
    COMMAND tools/dev/v8gen.py ${V8_TARGET}
    WORKING_DIRECTORY ${V8_DIR}
    RESULT_VARIABLE result
)
if(result)
    message(FATAL_ERROR "Failed to run 'v8gen.py'.")
endif()

message(STATUS "Compiling v8...")
execute_process(
    COMMAND ninja -C ${V8_BUILD_DIR} v8_monolith
    WORKING_DIRECTORY ${V8_DIR}
    RESULT_VARIABLE result
)
if(result)
    message(FATAL_ERROR "Failed to build V8 using Ninja.")
endif()
