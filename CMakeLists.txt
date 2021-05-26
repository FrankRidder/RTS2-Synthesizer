cmake_minimum_required(VERSION 3.15)
set(CMAKE_C_STANDARD 99)

project(RTS_Synthesizer C)

add_executable(RTS_Synthesizer main.c KeyboardEvents/inputThread.h KeyboardEvents/inputThread.c)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)
