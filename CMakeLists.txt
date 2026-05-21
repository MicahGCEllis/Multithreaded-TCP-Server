# Set the minimum version of CMake
cmake_minimum_required(VERSION 3.10)

# Name of the project
project(MultithreadedServer)

# Set the C standard to 20 for modern threading
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the threading library on your system
find_package(Threads REQUIRED)

# Include the headers folder
include_directories(include)

# Tell CMake where the source code is
add_executable(MultithreadedServer src/main.cpp src/TcpServer.cpp 
               src/ConnectionHandler.cpp src/ThreadPool.cpp 
               src/Logger.cpp src/Security.cpp)

# Link the threads library to your project
target_link_libraries(MultithreadedServer PRIVATE Threads::Threads ws2_32)