//-----------------------------------------------------------------------------
// Simple 2D texture class (stb_image loader, mipmaps, anisotropic filtering)
//-----------------------------------------------------------------------------
#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include <string>
#include "GL/glew.h"
#include "glm/glm.hpp"

class Texture2D
{
public:
	Texture2D();
	~Texture2D();
	Texture2D(const Texture2D&) = delete;
	Texture2D& operator=(const Texture2D&) = delete;

	bool loadTexture(const std::string& fileName, bool generateMipMaps = true);
	void createSolidColor(const glm::vec4& color);	// 1x1 texture (default maps)

	void bind(GLuint texUnit = 0) const;
	void unbind(GLuint texUnit = 0) const;

private:
	GLuint mTexture;
};

#endif // TEXTURE2D_H
