cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -std=c++0x -fopenmp -fPIC")

find_package( OpenCV REQUIRED )

INCLUDE_DIRECTORIES(/home/cv/opencv3/include)

add_subdirectory(src)

file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
set(files "${SOURCES}" "${HEADERS}")

add_executable(drnmppr-detect-marker src/drnmppr-detect-marker.cpp)
target_link_libraries(drnmppr-detect-marker ${OpenCV_LIBS})

add_executable(drnmppr-create-dictionary src/drnmppr-create-dictionary.cpp)
target_link_libraries(drnmppr-create-dictionary ${OpenCV_LIBS})

add_executable(drnmppr-create-marker src/drnmppr-create-marker.cpp)
target_link_libraries(drnmppr-create-marker ${OpenCV_LIBS})

add_executable(drnmppr-calibrate-camera src/drnmppr-calibrate-camera.cpp)
target_link_libraries(drnmppr-calibrate-camera ${OpenCV_LIBS})