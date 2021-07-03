cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)

project(RTS_Synthesizer C)

file(GLOB Synth_SRC
     "src/*.h"
     "src/*.c"
)

add_executable(RTS_Synthesizer ${Synth_SRC})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenAL REQUIRED)
find_library(LIBRT rt)

target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_link_libraries(${PROJECT_NAME} Threads::Threads ${LIBRT})
target_link_libraries(${PROJECT_NAME} "${OPENAL_LIBRARY}" m)
