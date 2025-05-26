#pragma once

#include <vector>

#include "camera.hpp"
#include "spotlight.hpp"
#include "framebuffer.hpp"
#include "shadowmap_framebuffer.hpp"
#include "ogl_material_factory.hpp"
#include "ogl_geometry_factory.hpp"
#include <random>
#include <renderer.hpp>

class QuadRenderer {
public:
	QuadRenderer()
		: mQuad(generateQuadTex())
	{}

	void render(const OGLShaderProgram &aShaderProgram, MaterialParameterValues &aParameters) const {
		aShaderProgram.use();
		aShaderProgram.setMaterialParameters(aParameters, MaterialParameterValues());
		
		GL_CHECK(glBindVertexArray(mQuad.vao.get()));
		GL_CHECK(glDrawElements(mQuad.mode, mQuad.indexCount, GL_UNSIGNED_INT, reinterpret_cast<void*>(0)));
	}
protected:

	IndexedBuffer mQuad;
};

inline std::vector<CADescription> getColorNormalPositionAttachments() {
	return {
		{ GL_RGBA, GL_FLOAT, GL_RGBA },
		{ GL_RGBA, GL_FLOAT, GL_RGBA32F },
		{ GL_RGBA, GL_FLOAT, GL_RGBA32F },
	};
}

inline std::vector<CADescription> getSingleColorAttachment() {
	return {
		{ GL_RGBA, GL_FLOAT, GL_RGBA32F },
	};
}


class Renderer {
public:
	Renderer(OGLMaterialFactory &aMaterialFactory)
		: mMaterialFactory(aMaterialFactory)
	{
		mCompositingShader = std::static_pointer_cast<OGLShaderProgram>(
				mMaterialFactory.getShaderProgram("compositing"));
		mShadowMapShader = std::static_pointer_cast<OGLShaderProgram>(
			mMaterialFactory.getShaderProgram("shadowmap"));
		mSSAOShader = std::static_pointer_cast<OGLShaderProgram>(
			mMaterialFactory.getShaderProgram("ssao"));
		mSSAOBlurShader = std::static_pointer_cast<OGLShaderProgram>(
			mMaterialFactory.getShaderProgram("ssao_blur"));
	}

	void initialize(int aWidth, int aHeight) {
		mWidth = aWidth;
		mHeight = aHeight;
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));

		mFramebuffer = std::make_unique<Framebuffer>(aWidth, aHeight, getColorNormalPositionAttachments());

		initialize_ssao(aWidth, aHeight);

		mShadowmapFramebuffer = std::make_unique<Framebuffer>(600, 600, getSingleColorAttachment());
		mCompositingParameters = {
			{ "u_diffuse", TextureInfo("diffuse", mFramebuffer->getColorAttachment(0)) },
			{ "u_normal", TextureInfo("diffuse", mFramebuffer->getColorAttachment(1)) },
			{ "u_position", TextureInfo("diffuse", mFramebuffer->getColorAttachment(2)) },
			{ "u_shadowMap", TextureInfo("shadowMap", mShadowmapFramebuffer->getColorAttachment(0)) },
			{ "u_ssao", TextureInfo("ssao", mSSAOBlurTexture) },
		};
	}

	void clear() {
		mFramebuffer->bind();
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	void setSSAOParameters(float radius, float bias) {
		mSSAORadius = radius;
		mSSAOBias = bias;
	}

	void ssaoPass(const Camera& aCamera)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mSSAOFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		mSSAOShader->use();

		GLint samplesLoc = glGetUniformLocation(mSSAOShader->program.get(), "u_samples");
		if (samplesLoc != -1)
		{
			std::vector<float> kernelData;
			kernelData.reserve(64 * 3);
			for (const auto& sample : mSSAOKernel)
			{
				kernelData.push_back(sample.x);
				kernelData.push_back(sample.y);
				kernelData.push_back(sample.z);
			}
			glUniform3fv(samplesLoc, 64, kernelData.data());
		}
		else
		{
			std::cout << "Failed to get uniform location for u_samples" << std::endl;
		}

		MaterialParameterValues ssaoParams;

		ssaoParams["u_proj"] = aCamera.getProjectionMatrix();
		ssaoParams["u_view"] = aCamera.getViewMatrix();
		ssaoParams["u_radius"] = mSSAORadius;
		ssaoParams["u_bias"] = mSSAOBias;
		ssaoParams["u_intensity"] = mSSAOIntensity;
		ssaoParams["u_noiseScale"] = glm::vec2(
			mWidth / 4.0f,
			mHeight / 4.0f
		);

		// Get and set uniform locations
		GLint posLoc = glGetUniformLocation(mSSAOShader->program.get(), "u_position");
		GLint normLoc = glGetUniformLocation(mSSAOShader->program.get(), "u_normal");
		GLint noiseLoc = glGetUniformLocation(mSSAOShader->program.get(), "u_noise");

		if (posLoc != -1) glUniform1i(posLoc, 0);
		if (normLoc != -1) glUniform1i(normLoc, 1);
		if (noiseLoc != -1) glUniform1i(noiseLoc, 2);

		GLuint posTex = mFramebuffer->getColorAttachment(2)->texture.get();
		GLuint normTex = mFramebuffer->getColorAttachment(1)->texture.get();
		GLuint noiseTex = mNoiseTexture->texture.get();

		// Bind textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, posTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normTex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, noiseTex);

		mQuadRenderer.render(*mSSAOShader, ssaoParams);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ssaoBlurPass()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mSSAOBlurFBO);
		glViewport(0, 0, mWidth, mHeight);
		glClear(GL_COLOR_BUFFER_BIT);
		mSSAOBlurShader->use();

		// Create parameters with texture info
		MaterialParameterValues blurParams;
		blurParams["u_ssaoInput"] = TextureInfo("ssao", mSSAOTexture);

		mQuadRenderer.render(*mSSAOBlurShader, blurParams);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	template<typename TScene, typename TCamera>
	void geometryPass(const TScene &aScene, const TCamera &aCamera, RenderOptions aRenderOptions) {
		GL_CHECK(glEnable(GL_DEPTH_TEST));
		GL_CHECK(glViewport(0, 0, mWidth, mHeight));
		mFramebuffer->bind();
		mFramebuffer->setDrawBuffers();
		auto projection = aCamera.getProjectionMatrix();
		auto view = aCamera.getViewMatrix();

		std::vector<RenderData> renderData;
		for (const auto &object : aScene.getObjects()) {
			auto data = object.getRenderData(aRenderOptions);
			if (data) {
				renderData.push_back(data.value());
			}
		}

		MaterialParameterValues fallbackParameters;
		fallbackParameters["u_projMat"] = projection;
		fallbackParameters["u_viewMat"] = view;
		fallbackParameters["u_solidColor"] = glm::vec4(0,0,0,1);
		fallbackParameters["u_viewPos"] = aCamera.getPosition();
		fallbackParameters["u_near"] = aCamera.near();
		fallbackParameters["u_far"] = aCamera.far();
		for (const auto &data: renderData) {
			const glm::mat4 modelMat = data.modelMat;
			const MaterialParameters &params = data.mMaterialParams;
			const OGLShaderProgram &shaderProgram = static_cast<const OGLShaderProgram &>(data.mShaderProgram);
			const OGLGeometry &geometry = static_cast<const OGLGeometry&>(data.mGeometry);

			fallbackParameters["u_modelMat"] = modelMat;
			fallbackParameters["u_normalMat"] = glm::mat3(modelMat);

			shaderProgram.use();
			shaderProgram.setMaterialParameters(params.mParameterValues, fallbackParameters);

			geometry.bind();
			geometry.draw();
		}
		mFramebuffer->unbind();
	}

	template<typename TLight>
	void compositingPass(const TLight &aLight) {
		GL_CHECK(glDisable(GL_DEPTH_TEST));
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		
		mCompositingParameters["u_lightPos"] = aLight.getPosition();
		mCompositingParameters["u_lightMat"] = aLight.getViewMatrix();
		mCompositingParameters["u_lightProjMat"] = aLight.getProjectionMatrix();
		mCompositingParameters["u_useSSAO"] = mSSAOEnabled;
		mCompositingParameters["u_useShadows"] = mShadowsEnabled;

		// If shadows are disabled, clear the shadow map texture
		if (!mShadowsEnabled) {
			mShadowmapFramebuffer->bind();
			GL_CHECK(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
			GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
			mShadowmapFramebuffer->unbind();
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mFramebuffer->getColorAttachment(0)->texture.get());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mFramebuffer->getColorAttachment(1)->texture.get());
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mFramebuffer->getColorAttachment(2)->texture.get());
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, mShadowmapFramebuffer->getColorAttachment(0)->texture.get());
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, mSSAOBlurTexture->texture.get());

		mQuadRenderer.render(*mCompositingShader, mCompositingParameters);
	}

	template<typename TScene, typename TLight>
	void shadowMapPass(const TScene &aScene, const TLight &aLight) {
		GL_CHECK(glEnable(GL_DEPTH_TEST));
		mShadowmapFramebuffer->bind();
		GL_CHECK(glViewport(0, 0, 600, 600));
		GL_CHECK(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		mShadowmapFramebuffer->setDrawBuffers();
		auto projection = aLight.getProjectionMatrix();
		auto view = aLight.getViewMatrix();

		MaterialParameterValues fallbackParameters;
		fallbackParameters["u_projMat"] = projection;
		fallbackParameters["u_viewMat"] = view;
		fallbackParameters["u_viewPos"] = aLight.getPosition();

		std::vector<RenderData> renderData;
		RenderOptions renderOptions = {"solid"};
		for (const auto &object : aScene.getObjects()) {
			auto data = object.getRenderData(renderOptions);
			if (data) {
				renderData.push_back(data.value());
			}
		}
		mShadowMapShader->use();
		for (const auto &data: renderData) {
			const glm::mat4 modelMat = data.modelMat;
			const MaterialParameters &params = data.mMaterialParams;
			const OGLShaderProgram &shaderProgram = static_cast<const OGLShaderProgram &>(data.mShaderProgram);
			const OGLGeometry &geometry = static_cast<const OGLGeometry&>(data.mGeometry);

			fallbackParameters["u_modelMat"] = modelMat;
			fallbackParameters["u_normalMat"] = glm::mat3(modelMat);

			mShadowMapShader->setMaterialParameters(fallbackParameters, {});

			geometry.bind();
			geometry.draw();
		}



		mShadowmapFramebuffer->unbind();
	}

	void initialize_ssao(int aWidth, int aHeight)
	{
		glGenFramebuffers(1, &mSSAOFBO);  glGenFramebuffers(1, &mSSAOBlurFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, mSSAOFBO);

		// Create SSAO texture
		OpenGLResource ssaoTexResource = createTexture();
		GLuint ssaoColorBuffer = ssaoTexResource.get();
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, aWidth, aHeight, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSAO Framebuffer not complete!" << std::endl;

		mSSAOTexture = std::make_shared<OGLTexture>(std::move(ssaoTexResource), GL_TEXTURE_2D);

		// Create SSAO blur texture
		glBindFramebuffer(GL_FRAMEBUFFER, mSSAOBlurFBO);
		OpenGLResource ssaoBlurTexResource = createTexture();
		GLuint ssaoColorBufferBlur = ssaoBlurTexResource.get();
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, aWidth, aHeight, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		mSSAOBlurTexture = std::make_shared<OGLTexture>(std::move(ssaoBlurTexResource), GL_TEXTURE_2D);

		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
		std::default_random_engine generator;
		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			float scale = float(i) / 64.0;

			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			mSSAOKernel.push_back(sample);
		}

		// Generate noise texture
		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
			ssaoNoise.push_back(noise);
		}

		// Create and configure noise texture
		OpenGLResource noiseTexResource = createTexture();
		GLuint noiseTexture = noiseTexResource.get();
		
		// Set texture parameters
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);

		mNoiseTexture = std::make_shared<OGLTexture>(std::move(noiseTexResource), GL_TEXTURE_2D);

		//std::cout << "Noise texture ID: " << mNoiseTexture->texture.get() << std::endl;
	}

	inline float lerp(float a, float b, float t)
	{
		return a + t * (b - a);
	}

	void setSSAOEnabled(bool enabled) {
		mSSAOEnabled = enabled;
	}

	void setShadowsEnabled(bool enabled) {
		mShadowsEnabled = enabled;
	}

protected:
	int mWidth = 100;
	int mHeight = 100;

	std::vector<glm::vec3> mSSAOKernel;

	// SSAO parameters
	float mSSAORadius = 0.5f;
	float mSSAOBias = 0.025f;
	float mSSAOIntensity = 1.0f;

	std::unique_ptr<Framebuffer> mFramebuffer;
	std::unique_ptr<Framebuffer> mShadowmapFramebuffer;

	unsigned int mSSAOFBO, mSSAOBlurFBO;
	std::shared_ptr<OGLTexture> mSSAOTexture; 
	std::shared_ptr<OGLTexture> mSSAOBlurTexture; 

	std::unique_ptr<Framebuffer> mSSAOFramebuffer;
	std::unique_ptr<Framebuffer> mSSAOBlurFramebuffer;

	MaterialParameterValues mCompositingParameters;
	QuadRenderer mQuadRenderer;
	std::shared_ptr<OGLShaderProgram> mCompositingShader;
	std::shared_ptr<OGLShaderProgram> mShadowMapShader;

	std::shared_ptr<OGLShaderProgram> mSSAOShader;
	std::shared_ptr<OGLShaderProgram> mSSAOBlurShader;

	OGLMaterialFactory &mMaterialFactory;

	std::shared_ptr<OGLTexture> mNoiseTexture;

	bool mSSAOEnabled = true;
	bool mShadowsEnabled = true; 
};
