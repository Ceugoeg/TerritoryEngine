# third_party/CMakeLists.txt

# ------------------------------------------------------------------------
# 1. GLM
# ------------------------------------------------------------------------
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glm)

# ------------------------------------------------------------------------
# 2. GLFW
# ------------------------------------------------------------------------
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# ------------------------------------------------------------------------
# 3. GLAD
# ------------------------------------------------------------------------
add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)