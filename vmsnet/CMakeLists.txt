set(SOURCES
    TcpConnector.cpp
    TcpAcceptor.cpp
)

add_library(vmsnet STATIC ${SOURCES})

target_link_libraries(vmsnet vmscore)
