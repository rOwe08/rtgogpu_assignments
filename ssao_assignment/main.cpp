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
	bool useShadows = true;
	
	// SSAO parameters
	float ssaoRadius = 0.5f;
	const float ssaoRadiusMin = 0.1f;
	const float ssaoRadiusMax = 2.0f;
	const float ssaoRadiusStep = 0.1f;

	float ssaoBias = 0.005f;
	const float ssaoBiasMin = 0.001f;
	const float ssaoBiasMax = 0.03f;
	const float ssaoBiasStep = 0.0025f;
};

int main() {
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

					case GLFW_KEY_S:
						toggle("Shadows", config.useShadows);
						break;

					case GLFW_KEY_R:
						config.ssaoRadius -= config.ssaoRadiusStep;
						if (config.ssaoRadius < config.ssaoRadiusMin) {
							config.ssaoRadius = config.ssaoRadiusMax;
						}
						std::cout << "SSAO Radius: " << config.ssaoRadius << std::endl;
						break;
					case GLFW_KEY_F:
						config.ssaoRadius += config.ssaoRadiusStep;
						if (config.ssaoRadius > config.ssaoRadiusMax) {
							config.ssaoRadius = config.ssaoRadiusMin;
						}
						std::cout << "SSAO Radius: " << config.ssaoRadius << std::endl;
						break;

					case GLFW_KEY_T:
						config.ssaoBias -= config.ssaoBiasStep;
						if (config.ssaoBias < config.ssaoBiasMin) {
							config.ssaoBias = config.ssaoBiasMax;
						}
						std::cout << "SSAO Bias: " << config.ssaoBias << std::endl;
						break;
					case GLFW_KEY_G:
						config.ssaoBias += config.ssaoBiasStep;
						if (config.ssaoBias > config.ssaoBiasMax) {
							config.ssaoBias = config.ssaoBiasMin;
						}
						std::cout << "SSAO Bias: " << config.ssaoBias << std::endl;
						break;

					case GLFW_KEY_1:
						config.currentSceneIdx = 0;
						break;
					case GLFW_KEY_2:
						config.currentSceneIdx = 1;
						break;
					}
				}
			});

		OGLMaterialFactory materialFactory;

		materialFactory.loadShadersFromDir("./shaders/");
		materialFactory.loadTexturesFromDir("./textures/");

		OGLGeometryFactory geometryFactory;

		std::array<SimpleScene, 2> scenes {
			createCottageScene(materialFactory, geometryFactory),
			createMonkeyScene(materialFactory, geometryFactory)
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

			if (config.useShadows) {
				renderer.shadowMapPass(scenes[config.currentSceneIdx], light);
			}
			renderer.geometryPass(scenes[config.currentSceneIdx], camera, RenderOptions{"solid"});
			
			renderer.setSSAOEnabled(config.useSSAO);
			renderer.setShadowsEnabled(config.useShadows);
			if (config.useSSAO) {
				renderer.setSSAOParameters(config.ssaoRadius, config.ssaoBias);
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
