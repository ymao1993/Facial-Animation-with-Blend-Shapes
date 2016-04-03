#ifndef XRSHADERUTILS_H
#define XRSHADERUTILS_H

#include <GL/glew.h>


/**
 * XRShaderUtils
 * Some utility functions for shader operations.
 *
 * @Author Yu Mao
 */
namespace XRShaderUtils
{
		GLuint loadShader(const char * filename,
			GLenum shader_type = GL_FRAGMENT_SHADER,
#ifdef _DEBUG
			bool check_errors = true);
#else
			bool check_errors = false);
#endif

		GLuint loadShaderFromSrc(const char * source,
			GLenum shader_type,
#ifdef _DEBUG
			bool check_errors = true);
#else
			bool check_errors = false);
#endif

		GLuint linkShaderProgram(const GLuint * shaders,
			int shader_count,
			bool delete_shaders,
#ifdef _DEBUG
			bool check_errors = true);
#else
			bool check_errors = false);
#endif

};

#endif