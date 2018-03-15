cmake_minimum_required(VERSION 3.6)
project(Particle-Sim-Pthreads)

File(GLOB SRC_FILES src/*.cpp)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-std=gnu++11 -lm -o3" )

message(src: ${SRC_FILES})
add_executable(Particle-Sim-Pthreads ${SRC_FILES})