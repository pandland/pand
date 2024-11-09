cmake_minimum_required(VERSION 3.22)
project(pandio C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)

set(SRC_FILES
        include/pandio/core.h
        src/internal.h
        include/pandio/tcp.h
        include/pandio/queue.h
        include/pandio/heap.h
        include/pandio/timers.h
        src/timers.c
        src/internal.c
        src/threadpool.c
        src/random.c
        include/pandio/threadpool.h
        include/pandio/err.h
        src/dlib.c
        include/pandio/fs.h
)

if (WIN32)
    list(APPEND SRC_FILES
            src/sys/event_iocp.c
            src/sys/tcp_win32.c
            src/sys/notifier_win32.c
            src/sys/thread_win32.c
            src/sys/fs_win32.c
    )

    add_library(pandio STATIC ${SRC_FILES})
    target_compile_definitions(pandio PUBLIC WIN32_LEAN_AND_MEAN)
    target_link_libraries(pandio PRIVATE advapi32)

elseif (UNIX)
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        list(APPEND SRC_FILES src/sys/event_epoll.c)
    endif ()

    set(KQUEUE_SYSTEMS "Darwin;FreeBSD;OpenBSD;NetBSD;DragonFly")
    if (CMAKE_SYSTEM_NAME IN_LIST KQUEUE_SYSTEMS)
        list(APPEND SRC_FILES src/sys/event_kqueue.c)
    endif ()

    list(APPEND SRC_FILES
        src/sys/tcp_unix.c
        src/sys/notifier_unix.c
        src/sys/thread_unix.c
        src/sys/fs_unix.c
    )

    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)

    add_library(pandio STATIC ${SRC_FILES})
    target_link_libraries(pandio PRIVATE pthread)
    target_link_libraries(pandio PRIVATE dl)
endif ()

target_include_directories(pandio
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
