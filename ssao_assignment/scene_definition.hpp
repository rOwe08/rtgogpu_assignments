#pragma once

#include <memory>
#include <vector>
#include <ranges>

#include "scene_object.hpp"
#include "cube.hpp"

#include "material_factory.hpp"
#include "geometry_factory.hpp"
#include "simple_scene.hpp"

inline SimpleScene createCottageScene(MaterialFactory& aMaterialFactory, GeometryFactory& aGeometryFactory) {
	SimpleScene scene;
	{
		auto cottage = std::make_shared<LoadedMeshObject>("D:\\School\\GitHub\\gl_tutorials\\build\\data/geometry/cottage.obj");
		cottage->addMaterial(
			"solid",
			MaterialParameters(
				"material_deffered",
				RenderStyle::Solid,
				{
					{ "u_diffuseTexture", TextureInfo("cottage/cottageDif.jpg") },
				}
				)
		);
		cottage->prepareRenderData(aMaterialFactory, aGeometryFactory);
		scene.addObject(cottage);
	}
	{
		auto ground = std::make_shared<LoadedMeshObject>("D:\\School\\GitHub\\gl_tutorials\\build\\data/geometry/ground.obj");
		ground->addMaterial(
			"solid",
			MaterialParameters(
				"material_deffered",
				RenderStyle::Solid,
				{
					{ "u_diffuseTexture", TextureInfo("cottage/groundDif.png") },
				}
				)
		);
		ground->prepareRenderData(aMaterialFactory, aGeometryFactory);
		scene.addObject(ground);
	}
	{
		auto oak = std::make_shared<LoadedMeshObject>("D:\\School\\GitHub\\gl_tutorials\\build\\data/geometry/oak.obj");
		oak->addMaterial(
			"solid",
			MaterialParameters(
				"material_deffered",
				RenderStyle::Solid,
				{
					{ "u_diffuseTexture", TextureInfo("cottage/OakDif.png") },
				}
				)
		);
		oak->prepareRenderData(aMaterialFactory, aGeometryFactory);
		scene.addObject(oak);
	}
	//{
	//	// Add rocket
	//	auto rocket = std::make_shared<LoadedMeshObject>("D:\\School\\GitHub\\gl_tutorials\\build\\data/geometry/rocket.obj");
	//	rocket->setName("ROCKET");
	//	rocket->setPosition(glm::vec3(0.0f, -0.4f, 0.0f));
	//	rocket->setScale(glm::vec3(0.03f));
	//	rocket->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	//	rocket->addMaterial(
	//		"solid",
	//		MaterialParameters(
	//			"material_deffered",
	//			RenderStyle::Solid,
	//			{
	//				{ "u_diffuseTexture", TextureInfo("rocket/10475_Rocket_Ship_v1_Diffuse.jpg") },
	//			}
	//			)
	//	);

	//	rocket->prepareRenderData(aMaterialFactory, aGeometryFactory);
	//	scene.addObject(rocket);
	//}


	return scene;
}
