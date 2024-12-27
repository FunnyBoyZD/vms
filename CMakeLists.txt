cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(VMS LANGUAGES CXX)

if (NOT WIN32)
    if (CMAKE_BUILD_TYPE STREQUAL "")
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
    endif ()
endif ()

set(VMS_OUT_DIR ${VMS_BINARY_DIR}/out)

set(VMS_INCLUDE_DIR "${VMS_SOURCE_DIR}/include")
include_directories(${VMS_INCLUDE_DIR})

find_package(Threads)

add_definitions(-DBOOST_ALL_NO_LIB -DBOOST_CHRONO_HEADER_ONLY -DBOOST_BIND_GLOBAL_PLACEHOLDERS)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${VMS_OUT_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${VMS_OUT_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${VMS_OUT_DIR}/lib)

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS -D_ENABLE_EXTENDED_ALIGNED_STORAGE)
    if (CMAKE_SYSTEM_VERSION)
        set(VER ${CMAKE_SYSTEM_VERSION})
        string(REPLACE "." "" VER ${VER})
        string(REGEX REPLACE "([0-9])" "0\\1" ver ${VER})
        add_definitions(-D_WIN32_WINNT=0x${ver})
    endif()
else ()
    if (APPLE)
        add_definitions(-D_ISOC99_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600 -D_DARWIN_C_SOURCE)
    else()
        add_definitions(-D_ISOC99_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600)
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -D_REENTRANT -fPIC -DPIC -Wno-unused-local-typedefs -Wno-psabi -Werror=unused-result")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")

    if (APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl, -undefined error -Wl,-t -v")
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-undefined -Wl,--exclude-libs,ALL")
    endif()
endif ()

add_subdirectory(3rdparty)
add_subdirectory(vmscore)
add_subdirectory(vmsnet)
add_subdirectory(vmsclient)
add_subdirectory(vmsserver)
