cmake_minimum_required(VERSION 3.15)
project(TerritoryEngine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(third_party)

add_library(TerritoryCore 
    src/Core/Application.cpp
    src/Renderer/Shader.cpp
    src/Memory/ShaderStorageBuffer.cpp
    src/Pipeline/ComputeDispatcher.cpp
    src/Renderer/InstancedRenderer.cpp
)

target_include_directories(TerritoryCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(TerritoryCore PUBLIC glfw glad glm)

add_subdirectory(examples)