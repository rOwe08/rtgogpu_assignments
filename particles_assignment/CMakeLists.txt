# Set the minimum CMake version
cmake_minimum_required(VERSION 3.10)

project(particles_assignment)

add_executable(particles_assignment 
	main.cpp
	particle_system.cpp
	../utils/error_handling.hpp
	../utils/ogl_resource.hpp
	../utils/shader.hpp
	../utils/window.hpp
)
target_sources(particles_assignment PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/../glad/src/glad.c
)
target_link_libraries(particles_assignment utils glm::glm glfw OpenGL::GL)
target_include_directories(particles_assignment PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../glad/include
	${CMAKE_CURRENT_SOURCE_DIR}/../utils
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}
)
