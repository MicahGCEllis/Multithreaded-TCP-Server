cmake_minimum_required(VERSION 3.10)
project(MultithreadedServer)
enable_language(ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(TEST_FILES tests/test_connection_handler.cpp
               tests/test_fiber_alignment.cpp
               tests/test_fiber_scheduler.cpp
               tests/test_iocp_accept.cpp
               tests/test_security.cpp
               tests/test_telemetry.cpp)

find_package(Threads REQUIRED)

# 1. Define the NetworkEngine library first
add_library(NetworkEngine OBJECT)

target_sources(NetworkEngine PRIVATE src/Fiber.cpp src/ThreadPool.cpp src/ConnectionHandler.cpp src/TcpServer.cpp src/Logger.cpp src/Security.cpp)

target_include_directories(NetworkEngine PUBLIC include)

foreach(TEST_SRC ${TEST_FILES})

    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    
    add_executable(${TEST_NAME} ${TEST_SRC} $<TARGET_OBJECTS:NetworkEngine>)
    target_include_directories(${TEST_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(${TEST_NAME} PRIVATE Threads::Threads ws2_32)
endforeach()
    
if(WIN32)
    target_sources(NetworkEngine PRIVATE 
        src/io/iocp_win.cpp 
        src/io/affinity_win.cpp
        src/io/telemetry_win.cpp
        src/asm/switch_win_x64.S
        src/asm/get_rsp_win_x64.S
    )
    target_link_libraries(NetworkEngine PUBLIC ws2_32)
elseif(APPLE)
    target_sources(NetworkEngine PRIVATE 
        src/io/kqueue_mac.cpp 
        src/io/affinity_mac.cpp
        src/io/telemetry_posix.cpp
        src/asm/switch_sysv_x64.S
    )
elseif(UNIX)
    target_sources(NetworkEngine PRIVATE 
        src/io/epoll_linux.cpp 
        src/io/affinity_linux.cpp
        src/io/telemetry_posix.cpp
        src/asm/switch_sysv_x64.S
    )
endif()

# 2. Now define the executable
add_executable(MultithreadedServer 
    src/main.cpp 
    $<TARGET_OBJECTS:NetworkEngine>
)

target_include_directories(MultithreadedServer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# 3. Link dependencies
target_link_libraries(MultithreadedServer PRIVATE Threads::Threads ws2_32)