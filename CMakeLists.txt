cmake_minimum_required(VERSION 3.10)
project(CPPHttpServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(include)

# Source files
file(GLOB SOURCES "src/*.cpp")

# Detect and add the correct libraries for Windows
if (WIN32)
    set(ADDITIONAL_LIBS ws2_32)
endif()

# Executable
add_executable(server ${SOURCES})

# Link libraries
target_link_libraries(server PRIVATE ${ADDITIONAL_LIBS})
