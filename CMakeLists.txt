# TerritoryEngine/CMakeLists.txt

cmake_minimum_required(VERSION 3.15)
project(TerritoryEngine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(third_party)

# add_library(TerritoryCore src/Core/Application.cpp ...)
# target_link_libraries(TerritoryCore PUBLIC glfw glad glm)
# target_include_directories(TerritoryCore PUBLIC include)