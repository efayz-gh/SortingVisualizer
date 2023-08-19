cmake_minimum_required(VERSION 3.24)
project(sortingvisualizer)

set(CMAKE_CXX_STANDARD 23)

file(GLOB INCLUDE "include/*.h" "include/*.cpp")

add_executable(sortingvisualizer main.cpp ${INCLUDE})

set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR "C:/libs/SFML/lib/cmake/SFML")

find_package(SFML COMPONENTS system window graphics audio network REQUIRED)

include_directories(${SFML_INCLUDE_DIR}, ./include)
target_link_libraries(sortingvisualizer sfml-system sfml-window sfml-graphics sfml-audio sfml-network)