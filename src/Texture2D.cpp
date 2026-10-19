#include "Texture2D.h"

#include <cassert>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

Texture2D::Texture2D()
	: mTexture(0)
{
}

Texture2D::~Texture2D()
{
	if (mTexture) glDeleteTextures(1, &mTexture);
}

bool Texture2D::loadTexture(const std::string& fileName, bool generateMipMaps)
{
	int width, height, components;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* imageData = stbi_load(fileName.c_str(), &width, &height, &components, STBI_rgb_alpha);
	if (imageData == nullptr)
	{
		std::cerr << "Error loading texture '" << fileName << "'" << std::endl;
		return false;
	}

	if (!mTexture) glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipMaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (GLEW_EXT_texture_filter_anisotropic)
	{
		GLfloat maxAniso = 1.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso < 8.0f ? maxAniso : 8.0f);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
	if (generateMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(imageData);
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

void Texture2D::createSolidColor(const glm::vec4& color)
{
	unsigned char px[4];
	for (int i = 0; i < 4; ++i)
		px[i] = (unsigned char)(glm::clamp(color[i], 0.0f, 1.0f) * 255.0f + 0.5f);

	if (!mTexture) glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::bind(GLuint texUnit) const
{
	assert(texUnit < 32);
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, mTexture);
}

void Texture2D::unbind(GLuint texUnit) const
{
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
}
