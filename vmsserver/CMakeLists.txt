set(SOURCES
    main.cpp
    Connection.h
    Connection.cpp
    Utils.h
    Utils.cpp
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(vmsserver ${SOURCES})

target_link_libraries(vmsserver vmsnet)
