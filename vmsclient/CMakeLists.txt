set(SOURCES
    main.cpp
    Connection.h
    Connection.cpp
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(vmsclient ${SOURCES})

target_link_libraries(vmsclient vmsnet)
