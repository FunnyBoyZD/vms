set(SOURCES
    Assert.cpp
    Logger.cpp
    Executor.cpp
)

add_library(vmscore STATIC ${SOURCES})

target_link_libraries(vmscore Threads::Threads boost)

if (UNIX AND NOT APPLE)
    target_link_libraries(vmscore rt)
endif ()
