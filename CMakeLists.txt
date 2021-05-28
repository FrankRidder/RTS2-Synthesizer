cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)

project(RTS_Synthesizer C)

add_executable(RTS_Synthesizer main.c 
	KeyboardEvents/inputThread.h KeyboardEvents/inputThread.c
	Sounds/SoundManager.h Sounds/SoundManager.c
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenAL REQUIRED)

target_link_libraries(${PROJECT_NAME} Threads::Threads)

target_link_libraries(${PROJECT_NAME} "${OPENAL_LIBRARY}" m)
