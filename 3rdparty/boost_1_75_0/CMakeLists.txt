set(SOURCES
    system/error_code.cpp
    program_options/config_file.cpp
    program_options/utf8_codecvt_facet.cpp
    program_options/cmdline.cpp
    program_options/parsers.cpp
    program_options/split.cpp
    program_options/winmain.cpp
    program_options/variables_map.cpp
    program_options/convert.cpp
    program_options/options_description.cpp
    program_options/value_semantic.cpp
    program_options/positional_options.cpp
    thread/future.cpp
)

if (NOT WIN32)
    set(SOURCES ${SOURCES}
        thread/pthread/thread.cpp
        thread/pthread/once_atomic.cpp
    )
else ()
    set(SOURCES ${SOURCES}
        thread/win32/thread.cpp
        thread/win32/thread_primitives.cpp
        thread/win32/tss_dll.cpp
        thread/win32/tss_pe.cpp
    )
endif ()

add_definitions(-DBOOST_SYSTEM_STATIC_LINK=1 -DBOOST_THREAD_BUILD_LIB=1)
if (NOT WIN32)
    add_definitions(-DBOOST_THREAD_POSIX)
endif ()

add_library(boost STATIC ${SOURCES})

target_link_libraries(boost Threads::Threads)
