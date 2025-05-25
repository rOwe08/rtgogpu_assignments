#include <iostream>
#include <cassert>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <array>

#include "ogl_resource.hpp"
#include "error_handling.hpp"
#include "window.hpp"
#include "shader.hpp"


#include "scene_definition.hpp"
#include "renderer.hpp"

#include "ogl_geometry_factory.hpp"
#include "ogl_material_factory.hpp"

#include <glm/gtx/string_cast.hpp>

void toggle(const std::string &aToggleName, bool &aToggleValue) {

	aToggleValue = !aToggleValue;
	std::cout << aToggleName << ": " << (aToggleValue ? "ON\n" : "OFF\n");
}

struct Config {
	int currentSceneIdx = 0;
	bool showSolid = true;
	bool showWireframe = false;
	bool useZOffset = false;
	bool useSSAO = true;
};

int main() {
	// Initialize GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	try {
		auto window = Window();
		MouseTracking mouseTracking;
		Config config;
		Camera camera(window.aspectRatio());
		camera.setPosition(glm::vec3(0.0f, 10.0f, 20.0f));
		camera.lookAt(glm::vec3());
		SpotLight light;
		light.setPosition(glm::vec3(25.0f, 40.0f, 30.0f));
		light.lookAt(glm::vec3());

		window.onCheckInput([&camera, &mouseTracking](GLFWwindow *aWin) {
				mouseTracking.update(aWin);
				if (glfwGetMouseButton(aWin, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
					camera.orbit(-0.4f * mouseTracking.offset(), glm::vec3());
				}
			});
		window.setKeyCallback([&config, &camera](GLFWwindow *aWin, int key, int scancode, int action, int mods) {
				if (action == GLFW_PRESS) {
					switch (key) {
					case GLFW_KEY_ENTER:
						camera.setPosition(glm::vec3(0.0f, -10.0f, -50.0f));
						camera.lookAt(glm::vec3());
						break;

					case GLFW_KEY_O:
						toggle("SSAO", config.useSSAO);
						break;
					}
				}
			});

		OGLMaterialFactory materialFactory;

		materialFactory.loadShadersFromDir("D:\\School\\GitHub\\gl_tutorials\\build\\04_deffered\\shaders");
		materialFactory.loadTexturesFromDir("D:\\School\\GitHub\\gl_tutorials\\build\\data\\textures");

		OGLGeometryFactory geometryFactory;

		std::array<SimpleScene, 1> scenes {
			createCottageScene(materialFactory, geometryFactory),
		};

		Renderer renderer(materialFactory);
		window.onResize([&camera, &window, &renderer](int width, int height) {
				camera.setAspectRatio(window.aspectRatio());
				renderer.initialize(width, height);
			});


		renderer.initialize(window.size()[0], window.size()[1]);
		window.runLoop([&] 
			{
			renderer.clear();

			renderer.shadowMapPass(scenes[config.currentSceneIdx], light);
			renderer.geometryPass(scenes[config.currentSceneIdx], camera, RenderOptions{"solid"});
			
			renderer.setSSAOEnabled(config.useSSAO);
			if (config.useSSAO) {
				renderer.ssaoPass(camera);
				renderer.ssaoBlurPass();
			}
			
			renderer.compositingPass(light);
		});
	} catch (ShaderCompilationError &exc) {
		std::cerr
			<< "Shader compilation error!\n"
			<< "Shader type: " << exc.shaderTypeName()
			<<"\nError: " << exc.what() << "\n";
		return -3;
	} catch (OpenGLError &exc) {
		std::cerr << "OpenGL error: " << exc.what() << "\n";
		return -2;
	} catch (std::exception &exc) {
		std::cerr << "Error: " << exc.what() << "\n";
		return -1;
	}

	glfwTerminate();
	return 0;
}
