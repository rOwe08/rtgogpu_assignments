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
		auto cottage = std::make_shared<LoadedMeshObject>("./data/geometry/cottage.obj");
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
		auto ground = std::make_shared<LoadedMeshObject>("./data/geometry/ground.obj");
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
		auto oak = std::make_shared<LoadedMeshObject>("./data/geometry/oak.obj");
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

	return scene;
}

inline SimpleScene createMonkeyScene(MaterialFactory& aMaterialFactory, GeometryFactory& aGeometryFactory)
{
	SimpleScene scene;
	{
		auto mesh = std::make_shared<LoadedMeshObject>("./data/geometry/monkey.obj");
		mesh->setScale(glm::vec3(3.5));
		mesh->setPosition(glm::vec3(-0.7, 0.0f, 15.0f));
		mesh->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		mesh->setName("MONKEY1");
		mesh->addMaterial(
			"solid",
			MaterialParameters(
				"pn_triangles",
				RenderStyle::Solid,
				{
					{"u_inner", 1 },
					{"u_outer", 1 },
					{"u_solidColor", glm::vec4(0,0.5,0.5,1)}
				},
				true)
		);
		mesh->addMaterial(
			"wireframe",
			MaterialParameters(
				"pn_triangles_solid",
				RenderStyle::Wireframe,
				{
					{"u_inner", 1 },
					{"u_outer", 1 },
				},
				true)
				);
		mesh->addMaterial(
			"solid",
			MaterialParameters(
				"material_deffered",
				RenderStyle::Solid,
				{
					{ "u_diffuseTexture", TextureInfo("white_texture.png") },
				}
				)
		);
		mesh->prepareRenderData(aMaterialFactory, aGeometryFactory);

		scene.addObject(mesh);
	}

	return scene;
}