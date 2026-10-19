#ifndef SHADOW_MAP_H
#define SHADOW_MAP_H

#include "GL/glew.h"
#include "glm/glm.hpp"

class ShadowMap
{
public:
	explicit ShadowMap(int size = 2048);
	~ShadowMap();
	ShadowMap(const ShadowMap&) = delete;
	ShadowMap& operator=(const ShadowMap&) = delete;

	bool init();

	void update(const glm::vec3& lightDir, const glm::vec3& center, float radius);

	void begin();
	void end();

	void bindTexture(GLuint unit) const;
	const glm::mat4& lightSpaceMatrix() const { return mLightSpace; }
	GLuint texture() const { return mDepthTex; }

private:
	int mSize;
	GLuint mFBO = 0, mDepthTex = 0;
	glm::mat4 mLightSpace{ 1.0f };
};

#endif
