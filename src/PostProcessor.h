//-----------------------------------------------------------------------------
// HDR rendering + post-processing
//  1. the scene is rendered into a floating point (RGBA16F) framebuffer
//  2. bright pass: keeps only the pixels brighter than 1.0 (half resolution)
//  3. separable gaussian blur (ping-pong between two buffers) = bloom
//  4. composite: scene + bloom, exposure, ACES tone mapping, gamma,
//     vignette, fades, optional shadow map debug view
//-----------------------------------------------------------------------------
#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "ShaderProgram.h"

struct PostSettings
{
	bool bloom = true;
	float bloomStrength = 0.35f;
	float exposure = 1.0f;
	float fade = 1.0f;
	bool showShadowMap = false;
	GLuint shadowMapTexture = 0;
	float time = 0.0f;
};

class PostProcessor
{
public:
	PostProcessor() = default;
	~PostProcessor();
	PostProcessor(const PostProcessor&) = delete;
	PostProcessor& operator=(const PostProcessor&) = delete;

	bool init(int width, int height);
	void resize(int width, int height);

	void beginScene();							// render target = HDR buffer
	void endScene(const PostSettings& s);		// bloom + composite to screen

	void drawFullscreenTriangle() const;		// also used by the sky

private:
	void createTargets();
	void destroyTargets();

	int mWidth = 0, mHeight = 0;
	GLuint mHdrFBO = 0, mHdrColor = 0, mHdrDepth = 0;
	GLuint mBloomFBO[2] = { 0, 0 }, mBloomTex[2] = { 0, 0 };
	GLuint mEmptyVAO = 0;

	ShaderProgram mBright, mBlur, mComposite;
};

#endif // POST_PROCESSOR_H
