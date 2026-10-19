#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <map>
#include "GL/glew.h"
#include "glm/glm.hpp"
using std::string;

class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();
	ShaderProgram(const ShaderProgram&) = delete;
	ShaderProgram& operator=(const ShaderProgram&) = delete;

	enum ShaderType
	{
		VERTEX,
		FRAGMENT,
		PROGRAM
	};

	bool loadShaders(const char* vsFilename, const char* fsFilename);
	void use();

	GLuint getProgram() const;

	void setUniform(const GLchar* name, const glm::vec2& v);
	void setUniform(const GLchar* name, const glm::vec3& v);
	void setUniform(const GLchar* name, const glm::vec4& v);
	void setUniform(const GLchar* name, const glm::mat3& m);
	void setUniform(const GLchar* name, const glm::mat4& m);
	void setUniform(const GLchar* name, const GLfloat f);
	void setUniform(const GLchar* name, const GLint v);
	void setUniformSampler(const GLchar* name, const GLint& slot);

	GLint getUniformLocation(const GLchar * name);

private:

	string fileToString(const string& filename);
	bool checkCompileErrors(GLuint shader, ShaderType type, const string& label);

	GLuint mHandle;
	std::map<string, GLint> mUniformLocations;
};
#endif
