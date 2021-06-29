cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)

project(RTS_Synthesizer C)
add_subdirectory(src)

add_executable(RTS_Synthesizer main.c 
	inputThread.h inputThread.c
	SoundManager.h SoundManager.c
	waveGenerator.h waveGenerator.c
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenAL REQUIRED)

target_link_libraries(${PROJECT_NAME} Threads::Threads)

target_link_libraries(${PROJECT_NAME} "${OPENAL_LIBRARY}" m)
