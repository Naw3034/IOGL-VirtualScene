#include "ShadowMap.h"

#include <cmath>
#include <iostream>
#include "glm/gtc/matrix_transform.hpp"

ShadowMap::ShadowMap(int size)
	: mSize(size)
{
}

ShadowMap::~ShadowMap()
{
	if (mDepthTex) glDeleteTextures(1, &mDepthTex);
	if (mFBO) glDeleteFramebuffers(1, &mFBO);
}

bool ShadowMap::init()
{
	glGenTextures(1, &mDepthTex);
	glBindTexture(GL_TEXTURE_2D, mDepthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, mSize, mSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	const float border[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

	glGenFramebuffers(1, &mFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	bool ok = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (!ok) std::cerr << "Shadow map framebuffer is incomplete" << std::endl;
	return ok;
}

void ShadowMap::update(const glm::vec3& lightDir, const glm::vec3& center, float radius)
{
	glm::vec3 dir = glm::normalize(lightDir);
	glm::vec3 eye = center - dir * (radius * 2.0f);
	glm::vec3 up = std::fabs(dir.y) > 0.99f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
	glm::mat4 view = glm::lookAt(eye, center, up);

	float texel = (2.0f * radius) / (float)mSize;
	glm::vec4 origin = view * glm::vec4(0, 0, 0, 1);
	glm::vec2 snap(std::floor(origin.x / texel) * texel - origin.x, std::floor(origin.y / texel) * texel - origin.y);
	glm::mat4 proj = glm::ortho(-radius + snap.x, radius + snap.x, -radius + snap.y, radius + snap.y, 0.1f, radius * 4.0f);
	mLightSpace = proj * view;
}

void ShadowMap::begin()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glViewport(0, 0, mSize, mSize);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0f, 4.0f);
}

void ShadowMap::end()
{
	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::bindTexture(GLuint unit) const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, mDepthTex);
}
