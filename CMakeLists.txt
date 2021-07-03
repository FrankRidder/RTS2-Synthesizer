cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)

project(RTS_Synthesizer C)

find_library(SOUNDIO_LIBRARY soundio HINTS /usr/lib/x86_64-linux-gnu)

add_executable(RTS_Synthesizer 
	src/main.c 
	src/inputThread.h 	src/inputThread.c
	src/SoundManager.h 	src/SoundManager.c
	src/waveGenerator.h 	src/waveGenerator.c
	src/filter.h		src/filter.c
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenAL REQUIRED)

target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_link_libraries(${PROJECT_NAME} ${SOUNDIO_LIBRARY})
target_link_libraries(${PROJECT_NAME} "${OPENAL_LIBRARY}" m)
