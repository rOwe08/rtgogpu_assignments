#pragma once

#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>

#include "camera.hpp"
#include "ogl_material_factory.hpp"
#include "ogl_geometry_factory.hpp"


class Renderer
{
public:

	Renderer(OGLMaterialFactory& aMaterialFactory)
		: mMaterialFactory(aMaterialFactory)
	{
		mShowNormalsShader = std::static_pointer_cast<OGLShaderProgram>(
			mMaterialFactory.getShaderProgram("generate_normals"));
	}

	void initialize()
	{
		GL_CHECK(glEnable(GL_DEPTH_TEST));
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void clear()
	{
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	template<typename TScene, typename TCamera>
	void renderScene(const TScene& aScene, const TCamera& aCamera, RenderOptions aRenderOptions)
	{
		auto projection = aCamera.getProjectionMatrix();
		auto view = aCamera.getViewMatrix();

		std::vector<RenderData> renderData;
		std::vector<RenderData> transparentData;

		for (const auto& object : aScene.getObjects())
		{
			auto data = object.getRenderData(aRenderOptions);
			if (data)
			{
				if (data->mMaterialParams.mMaterialName == "particle") {
					transparentData.push_back(data.value());
				} else {
					renderData.push_back(data.value());
				}
			}
		}

		MaterialParameterValues fallbackParameters;
		fallbackParameters["u_projMat"] = projection;
		fallbackParameters["u_viewMat"] = view;
		fallbackParameters["u_solidColor"] = glm::vec4(1.0f, 0.6f, 0.1f, 1.0f);
		fallbackParameters["u_viewPos"] = aCamera.getPosition();
		fallbackParameters["u_near"] = aCamera.near();
		fallbackParameters["u_far"] = aCamera.far();

		GL_CHECK(glDisable(GL_BLEND));
		GL_CHECK(glEnable(GL_DEPTH_TEST));
		GL_CHECK(glDepthMask(GL_TRUE));

		GL_CHECK(glPatchParameteri(GL_PATCH_VERTICES, 3));
		for (const auto& data : renderData)
		{
			const glm::mat4& modelMat = data.modelMat;
			const MaterialParameters& params = data.mMaterialParams;
			const OGLShaderProgram& shaderProgram = static_cast<const OGLShaderProgram&>(data.mShaderProgram);
			const OGLGeometry& geometry = static_cast<const OGLGeometry&>(data.mGeometry);

			fallbackParameters["u_modelMat"] = modelMat;
			fallbackParameters["u_normalMat"] = glm::mat3(modelMat);

			shaderProgram.use();
			shaderProgram.setMaterialParameters(params.mParameterValues, fallbackParameters);
			geometry.bind();
			if (params.mIsTesselation)
			{
				geometry.draw(GL_PATCHES);
			}
			else
			{
				geometry.draw();
			}
		}

		GL_CHECK(glEnable(GL_BLEND));
		GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
		GL_CHECK(glEnable(GL_DEPTH_TEST));
		GL_CHECK(glDepthMask(GL_FALSE)); 

		std::vector<size_t> indices(transparentData.size());
		std::iota(indices.begin(), indices.end(), 0);

		// Sort indices by distance from camera
		std::sort(indices.begin(), indices.end(),
			[&view, &transparentData](size_t a, size_t b) {
				glm::vec3 posA = glm::vec3(transparentData[a].modelMat[3]);
				glm::vec3 posB = glm::vec3(transparentData[b].modelMat[3]);
				float distA = glm::length(glm::vec3(view * glm::vec4(posA, 1.0f)));
				float distB = glm::length(glm::vec3(view * glm::vec4(posB, 1.0f)));
				return distA > distB;
			});

		// Render in sorted order
		for (size_t idx : indices)
		{
			const auto& data = transparentData[idx];
			const glm::mat4& modelMat = data.modelMat;
			const MaterialParameters& params = data.mMaterialParams;
			const OGLShaderProgram& shaderProgram = static_cast<const OGLShaderProgram&>(data.mShaderProgram);
			const OGLGeometry& geometry = static_cast<const OGLGeometry&>(data.mGeometry);

			fallbackParameters["u_modelMat"] = modelMat;
			fallbackParameters["u_normalMat"] = glm::mat3(modelMat);

			shaderProgram.use();
			shaderProgram.setMaterialParameters(params.mParameterValues, fallbackParameters);
			geometry.bind();
			geometry.draw();
		}

		GL_CHECK(glDisable(GL_BLEND));
		GL_CHECK(glDepthMask(GL_TRUE));
	}

	template<typename TScene, typename TCamera>
	void renderSceneNormals(const TScene& aScene, const TCamera& aCamera, RenderOptions aRenderOptions)
	{
		auto projection = aCamera.getProjectionMatrix();
		auto view = aCamera.getViewMatrix();

		std::vector<RenderData> renderData;
		for (const auto& object : aScene.getObjects())
		{
			auto data = object.getRenderData(aRenderOptions);
			if (data)
			{
				renderData.push_back(data.value());
			}
		}

		MaterialParameterValues fallbackParameters;
		fallbackParameters["u_projMat"] = projection;
		fallbackParameters["u_viewMat"] = view;
		fallbackParameters["u_solidColor"] = glm::vec4(0, 0, 0, 1);
		fallbackParameters["u_viewPos"] = aCamera.getPosition();
		fallbackParameters["u_near"] = aCamera.near();
		fallbackParameters["u_far"] = aCamera.far();
		for (const auto& data : renderData)
		{
			const glm::mat4& modelMat = data.modelMat;
			const OGLGeometry& geometry = static_cast<const OGLGeometry&>(data.mGeometry);

			fallbackParameters["u_modelMat"] = modelMat;
			fallbackParameters["u_normalMat"] = glm::mat3(modelMat);

			mShowNormalsShader->use();
			mShowNormalsShader->setMaterialParameters(fallbackParameters, {});

			geometry.bind();
			geometry.draw(GL_POINTS);
		}
	}
protected:
	std::shared_ptr<OGLShaderProgram> mShowNormalsShader;

	OGLMaterialFactory& mMaterialFactory;
};