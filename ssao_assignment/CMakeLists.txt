# Set the minimum CMake version
cmake_minimum_required(VERSION 3.10)

project(ssao_assignment)

add_executable(ssao_assignment
	main.cpp
	renderer.hpp
	scene_definition.hpp
)
target_sources(ssao_assignment PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/../glad/src/glad.c
)
target_link_libraries(ssao_assignment utils glm::glm glfw OpenGL::GL)
target_include_directories(ssao_assignment PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../glad/include
	${CMAKE_CURRENT_SOURCE_DIR}/../utils
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}
)
