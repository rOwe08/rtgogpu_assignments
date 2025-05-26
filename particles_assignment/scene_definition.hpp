#pragma once

#include <memory>
#include <vector>
#include <ranges>
#include <iostream>

#include "vertex.hpp"
#include "scene_object.hpp"
#include "cube.hpp"
#include "instanced_cube.hpp"

#include "material_factory.hpp"
#include "geometry_factory.hpp"
#include "simple_scene.hpp"
#include "particle_system.h"

constexpr unsigned int DIFFUSE = 1;
constexpr unsigned int SPECULAR = 1 << 1;
constexpr unsigned int BUMP = 1 << 2;
constexpr unsigned int PARALLAX = 1 << 3;
constexpr unsigned int AMBIENT_OCC = 1 << 4;
constexpr unsigned int SHADOW = 1 << 5;
constexpr unsigned int DEBUG = 1 << 7;


inline SimpleScene createCubeScene(MaterialFactory &aMaterialFactory, GeometryFactory &aGeometryFactory) {
	SimpleScene scene;
	{
		auto cube = std::make_shared<Cube>();

		cube->setName("CUBE1");
		cube->setPosition(glm::vec3(0.75f, 0.0f, 0.0f));
		cube->addMaterial(
			"solid",
			MaterialParameters(
				"material",
				RenderStyle::Solid,
				{
					{ "configuration", static_cast<unsigned int>(DIFFUSE) },
					{ "u_diffuseTexture", TextureInfo("brick_wall/Brick_Wall_012_COLOR.jpg") },
					{ "u_specularTexture", TextureInfo("brick_wall/Brick_Wall_012_ROUGH.jpg") },
					{ "u_normalTexture", TextureInfo("brick_wall/Brick_Wall_012_NORM.jpg") },
					{ "u_displacementTexture", TextureInfo("brick_wall/Brick_Wall_012_DISP.png") },
					{ "u_ambientOccTexture", TextureInfo("brick_wall/Brick_Wall_012_OCC.jpg") },
				}
				)
			);
		cube->addMaterial(
			"wireframe",
			MaterialParameters(
				"solid_color",
				RenderStyle::Wireframe,
				{}
				)
			);
		cube->prepareRenderData(aMaterialFactory, aGeometryFactory);
		scene.addObject(cube);
	}
	{
		auto cube = std::make_shared<Cube>();

		cube->setName("CUBE2");
		cube->setPosition(glm::vec3(-0.75f, 0.0f, 0.0f));
		cube->addMaterial(
			"solid",
			MaterialParameters(
				"material",
				RenderStyle::Solid,
				{
					{ "configuration", static_cast<unsigned int>(DIFFUSE | SPECULAR | BUMP) },
					{ "u_diffuseTexture", TextureInfo("brick_wall/Brick_Wall_012_COLOR.jpg") },
					{ "u_specularTexture", TextureInfo("brick_wall/Brick_Wall_012_ROUGH.jpg") },
					{ "u_normalTexture", TextureInfo("brick_wall/Brick_Wall_012_NORM.jpg") },
					{ "u_displacementTexture", TextureInfo("brick_wall/Brick_Wall_012_DISP.png") },
					{ "u_ambientOccTexture", TextureInfo("brick_wall/Brick_Wall_012_OCC.jpg") },
				}
				)
			);
		cube->addMaterial(
			"wireframe",
			MaterialParameters(
				"solid_color",
				RenderStyle::Wireframe,
				{}
				)
			);
		cube->prepareRenderData(aMaterialFactory, aGeometryFactory);
		scene.addObject(cube);
	}
	return scene;
}

inline SimpleScene createInstancedCubesScene(MaterialFactory &aMaterialFactory, GeometryFactory &aGeometryFactory) {
	SimpleScene scene;
	std::vector<VertexColor> instanceAttributes;
	for (float x = -6.0f; x <= 6.0f; x += 1.5f) {
		for (float y = -6.0f; y <= 6.0f; y += 1.5f) {
			for (float z = -6.0f; z <= 6.0f; z += 1.5f) {
				float red = (((instanceAttributes.size() + 31415) * 325) % 255) / 255.0f;
				float green = (((instanceAttributes.size() + 81812) * 17) % 255) / 255.0f;
				float blue = (((instanceAttributes.size() + 563) * 999) % 255) / 255.0f;
				instanceAttributes.emplace_back(glm::vec3(x, y, z), glm::vec3(red, green, blue));
			}
		}
	}
	auto instancedCube = std::make_shared<InstancedCube>(std::move(instanceAttributes));
	instancedCube->setScale(glm::vec3(0.1, 0.1, 0.1));
	instancedCube->addMaterial(
		"solid",
		MaterialParameters(
			"instanced",
			RenderStyle::Solid,
			{
				{"u_solidColor", glm::vec4(0,0.5,0.7,1)}
			}
			)
		);
	instancedCube->addMaterial(
		"wireframe",
		MaterialParameters(
			"solid_color",
			RenderStyle::Wireframe,
			{}
			)
		);
	instancedCube->prepareRenderData(aMaterialFactory, aGeometryFactory);

	scene.addObject(instancedCube);
	return scene;
}


inline SimpleScene createMonkeyScene(MaterialFactory &aMaterialFactory, GeometryFactory &aGeometryFactory) {
	SimpleScene scene;
	{
		auto mesh = std::make_shared<LoadedMeshObject>("./data/geometry/monkey.obj");
		mesh->setScale(glm::vec3(0.5));
		mesh->setPosition(glm::vec3(-0.7, 0.0f, 0.0f));
		mesh->setRotation(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
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
		mesh->prepareRenderData(aMaterialFactory, aGeometryFactory);

		scene.addObject(mesh);
	}
	{
		int innerFactor = 3;
		int outerFactor = 3;
		auto mesh = std::make_shared<LoadedMeshObject>("./data/geometry/monkey.obj");
		mesh->setScale(glm::vec3(0.5));
		mesh->setPosition(glm::vec3(0.7, 0.0f, 0.0f));
		mesh->setRotation(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
		mesh->setName("MONKEY2");
		mesh->addMaterial(
				"solid",
				MaterialParameters(
					"pn_triangles",
					RenderStyle::Solid,
					{
						{"u_inner", innerFactor },
						{"u_outer", outerFactor },
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
						{"u_inner", innerFactor },
						{"u_outer", outerFactor },
					},
					true)
				);
		mesh->prepareRenderData(aMaterialFactory, aGeometryFactory);
		scene.addObject(mesh);
	}
	return scene;
}

inline SimpleScene createParticleScene(MaterialFactory& aMaterialFactory, GeometryFactory& aGeometryFactory)
{
	SimpleScene scene;

	// Adding rocket
	auto rocket = std::make_shared<LoadedMeshObject>("./data/geometry/rocket.obj");
	rocket->setName("ROCKET");
	rocket->setPosition(glm::vec3(0.0f, -0.4f, 0.0f));
	rocket->setScale(glm::vec3(0.01f));
	rocket->setRotation(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));
	rocket->addMaterial(
		"solid",
		MaterialParameters(
			"material",
			RenderStyle::Solid,
			{
				{ "configuration", static_cast<unsigned int>(DIFFUSE | SPECULAR | BUMP) },
				{ "u_diffuseTexture", TextureInfo("rocket/10475_Rocket_Ship_v1_Diffuse.jpg") },
				{ "u_specularTexture", TextureInfo("rocket/10475_Rocket_Ship_v1_Diffuse.jpg") },
				{ "u_normalTexture", TextureInfo("rocket/10475_Rocket_Ship_v1_Diffuse.jpg") }
			}
		)
	);
	rocket->addMaterial(
		"wireframe",
		MaterialParameters(
			"solid_color",
			RenderStyle::Wireframe,
			{}
		)
	);
	rocket->prepareRenderData(aMaterialFactory, aGeometryFactory);
	scene.addObject(rocket);

	// Particle system
	auto particleSystem = std::make_shared<ParticleSystem>(1000);
	particleSystem->setName("FIRE_PARTICLES");
	particleSystem->setPosition(glm::vec3(0.0f, -0.45f, 0.0f));

	particleSystem->addMaterial(
		"solid",
		MaterialParameters(
			"particle",
			RenderStyle::Solid,
			{
				{"u_particleTexture", TextureInfo("particle.png")},
				{"u_solidColor", glm::vec4(1.0f, 0.6f, 0.1f, 1.0f)},
				{"u_lightPos", glm::vec3(2.0f, 2.0f, 2.0f)},
				{"u_lightColor", glm::vec3(1.0f, 0.9f, 0.8f)},
				{"u_lightIntensity", 1.0f}
			}
		)
	);

	particleSystem->prepareRenderData(aMaterialFactory, aGeometryFactory);
	scene.addObject(particleSystem);

	return scene;
}
