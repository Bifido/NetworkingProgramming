cmake_minimum_required(VERSION 3.1)
project(Network)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    main.cpp
    SocketDetails.h
    Socket.h
    Socket.cpp
    IpAddress.cpp
    IpAddress.h
    Types.h
    #sub platforms
        win32/SocketImpl.cpp
        win32/SocketImpl.h
        unix/SocketImpl.cpp
        unix/SocketImpl.h

    #netlib
        netlib/Channel.h
        netlib/Connection.h
        netlib/NetLanDiscovery.h
        netlib/NetTime.h
        netlib/EndPoint.h
        netlib/Reliability.h
        netlib/BitStream.h

        netlib/Channel.cpp
        netlib/Connection.cpp
        netlib/NetLanDiscovery.cpp
        netlib/NetTime.cpp
        netlib/EndPoint.cpp
        netlib/Reliability.cpp
        netlib/BitStream.cpp
)
add_executable(Network ${SOURCE_FILES} Debug.cpp Debug.h netlib/NetReactor.cpp netlib/NetReactor.h netlib/NetAcceptor.cpp netlib/NetAcceptor.h netlib/NetConnector.cpp netlib/NetConnector.h)