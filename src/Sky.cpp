//-----------------------------------------------------------------------------
// Procedural sky
//-----------------------------------------------------------------------------
#include "Sky.h"

#include "Paths.h"
#include "PostProcessor.h"
#include "Scene.h"

bool Sky::init()
{
	return mShader.loadShaders(resourcePath("shaders/sky.vert").c_str(), resourcePath("shaders/sky.frag").c_str());
}

void Sky::draw(const PostProcessor& post, const Environment& env, const glm::mat4& view, const glm::mat4& projection, float time)
{
	// Remove the translation from the view matrix: the sky is infinitely far away
	glm::mat4 rotationOnly = glm::mat4(glm::mat3(view));
	glm::mat4 invViewProj = glm::inverse(projection * rotationOnly);

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	mShader.use();
	mShader.setUniform("invViewProj", invViewProj);
	mShader.setUniform("zenithColor", env.zenithColor);
	mShader.setUniform("horizonColor", env.horizonColor);
	mShader.setUniform("sunDir", env.sunDir);
	mShader.setUniform("sunColor", env.sunColor);
	mShader.setUniform("moonDir", env.moonDir);
	mShader.setUniform("nightFactor", env.nightFactor);
	mShader.setUniform("time", time);
	post.drawFullscreenTriangle();
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}
