project(Phase1)
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

find_package(OpenCV REQUIRED)

# Project Executable
add_executable (part1 part1.cpp colorinstance.cpp )
add_executable (part2 part2.cpp )
target_link_libraries(part1 ${OpenCV_LIBS})
target_link_libraries(part2 ${OpenCV_LIBS})
