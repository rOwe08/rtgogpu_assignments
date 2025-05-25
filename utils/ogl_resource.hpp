#pragma once

#include <glad/glad.h>
#include <functional>
#include <utility>

#include "error_handling.hpp"

/**
 * @brief A move-only RAII wrapper for an OpenGL resource.
 *		Automatically creates and deletes the underlying resource.
 */
class OpenGLResource {
public:
	/**
	 * @brief Default constructor creates an empty wrapper (no resource).
	 */
	OpenGLResource()
		: mId(0)
	{}

	/**
	 * @brief Constructs a resource using creation and deletion functions.
	 * @param aCreateFunc  Function that returns a newly created GL resource ID.
	 * @param aDeleteFunc  Function that deletes a GL resource given its ID.
	 */
	OpenGLResource(
		std::function<GLuint()> aCreateFunc,
		std::function<void(GLuint)> aDeleteFunc)
		: mCreateFunc(aCreateFunc)
		, mDeleteFunc(aDeleteFunc)
	{
		mId = mCreateFunc();
	}

	/**
	 * @brief Invokes the deletion function if one is set.
	 */
	~OpenGLResource() {
		if (mDeleteFunc) {
			mDeleteFunc(mId);
		}
	}

	// Disable copy operations
	OpenGLResource(const OpenGLResource&) = delete;
	OpenGLResource& operator=(const OpenGLResource&) = delete;

	// Enable move operations

	/**
	 * @brief Move constructor transfers ownership from another resource.
	 */
	OpenGLResource(OpenGLResource&& other) noexcept
		: mId(std::exchange(other.mId, 0))
		, mCreateFunc(std::move(other.mCreateFunc))
		, mDeleteFunc(std::move(other.mDeleteFunc))
	{}

	/**
	 * @brief Move assignment operator transfers ownership and frees current resource if any.
	 */
	OpenGLResource& operator=(OpenGLResource&& other) noexcept {
		if (this != &other) {
			if (mDeleteFunc) {
				mDeleteFunc(mId);
			}
			mId = std::exchange(other.mId, 0);
			mCreateFunc = std::move(other.mCreateFunc);
			mDeleteFunc = std::move(other.mDeleteFunc);
		}
		return *this;
	}

	/**
	 * @return The underlying OpenGL resource ID.
	 */
	GLuint get() const { return mId; }

	/**
	 * @brief Allows checking if the resource is valid (non-zero).
	 *
	 * Example usage:
	 * \code
	 * OpenGLResource resource = createBuffer();
	 * if (resource) {
	 *	 // Use the resource
	 * }
	 * \endcode
	 */
	explicit operator bool() const {
		return mId != 0;
	}

private:
	GLuint mId = 0; ///< The OpenGL resource ID.
	std::function<GLuint(void)> mCreateFunc;
	std::function<void(GLuint)> mDeleteFunc;
};

/**
 * @brief Creates an RAII-managed Vertex Array Object (VAO).
 */
inline OpenGLResource createVertexArray() {
	return OpenGLResource(
		[]{
			GLuint id = 0;
			GL_CHECK(glGenVertexArrays(1, &id));
			return id;
		},
		[](GLuint id){
			glDeleteVertexArrays(1, &id);
		});
}

/**
 * @brief Creates an RAII-managed Buffer Object.
 */
inline OpenGLResource createBuffer() {
	return OpenGLResource(
		[]{
			GLuint id = 0;
			GL_CHECK(glGenBuffers(1, &id));
			return id;
		},
		[](GLuint id){
			glDeleteBuffers(1, &id);
		});
}

/**
 * @brief Creates an RAII-managed Query Object.
 */
inline OpenGLResource createQuery() {
	return OpenGLResource(
		[]{
			GLuint id = 0;
			GL_CHECK(glGenQueries(1, &id));
			return id;
		},
		[](GLuint id){
			glDeleteQueries(1, &id);
		});
}

/**
 * @brief Creates an RAII-managed Renderbuffer Object.
 */
inline OpenGLResource createRenderBuffer() {
	return OpenGLResource(
		[]{
			GLuint id = 0;
			GL_CHECK(glGenRenderbuffers(1, &id));
			return id;
		},
		[](GLuint id){
			glDeleteRenderbuffers(1, &id);
		});
}

/**
 * @brief Creates an RAII-managed Framebuffer Object.
 */
inline OpenGLResource createFramebuffer() {
	return OpenGLResource(
		[]{
			GLuint id = 0;
			GL_CHECK(glGenFramebuffers(1, &id));
			return id;
		},
		[](GLuint id){
			glDeleteFramebuffers(1, &id);
		});
}

/**
 * @brief Creates an RAII-managed Shader Object.
 */
inline OpenGLResource createShader(GLenum aShaderType) {
	return OpenGLResource(
		[aShaderType]{
			return glCreateShader(aShaderType);
		},
		[](GLuint id){
			glDeleteShader(id);
		});
}

/**
 * @brief Creates an RAII-managed Shader Program.
 */
inline OpenGLResource createShaderProgram() {
	return OpenGLResource(
		[]{
			return glCreateProgram();
		},
		[](GLuint id){
			glDeleteProgram(id);
		});
}

/**
 * @brief Creates an RAII-managed Texture Object.
 */
inline OpenGLResource createTexture() {
	return OpenGLResource(
		[]{
			GLuint id = 0;
			GL_CHECK(glGenTextures(1, &id));
			return id;
		},
		[](GLuint id){
			glDeleteTextures(1, &id);
		});
}

/**
 * @brief Creates an RAII-managed Sampler Object.
 */
inline OpenGLResource createSampler() {
	return OpenGLResource(
		[]{
			GLuint id = 0;
			GL_CHECK(glGenSamplers(1, &id));
			return id;
		},
		[](GLuint id){
			glDeleteSamplers(1, &id);
		});
}
