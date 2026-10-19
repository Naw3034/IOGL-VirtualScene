//-----------------------------------------------------------------------------
// HDR rendering + bloom + tone mapping
//-----------------------------------------------------------------------------
#include "PostProcessor.h"

#include <iostream>
#include "Paths.h"

namespace
{
	GLuint makeColorTexture(int w, int h)
	{
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		return tex;
	}
}

PostProcessor::~PostProcessor()
{
	destroyTargets();
	if (mEmptyVAO) glDeleteVertexArrays(1, &mEmptyVAO);
}

bool PostProcessor::init(int width, int height)
{
	bool ok = mBright.loadShaders(resourcePath("shaders/fullscreen.vert").c_str(), resourcePath("shaders/bright.frag").c_str());
	ok = mBlur.loadShaders(resourcePath("shaders/fullscreen.vert").c_str(), resourcePath("shaders/blur.frag").c_str()) && ok;
	ok = mComposite.loadShaders(resourcePath("shaders/fullscreen.vert").c_str(), resourcePath("shaders/composite.frag").c_str()) && ok;

	glGenVertexArrays(1, &mEmptyVAO);	// a core profile needs a VAO even without attributes
	mWidth = width;
	mHeight = height;
	createTargets();
	return ok;
}

void PostProcessor::resize(int width, int height)
{
	if (width == mWidth && height == mHeight) return;
	if (width <= 0 || height <= 0) return;	// minimised window
	mWidth = width;
	mHeight = height;
	destroyTargets();
	createTargets();
}

void PostProcessor::createTargets()
{
	// HDR scene buffer
	mHdrColor = makeColorTexture(mWidth, mHeight);
	glGenRenderbuffers(1, &mHdrDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, mHdrDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mWidth, mHeight);

	glGenFramebuffers(1, &mHdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mHdrFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mHdrColor, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mHdrDepth);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "HDR framebuffer is incomplete" << std::endl;

	// Half resolution bloom buffers
	for (int i = 0; i < 2; ++i)
	{
		mBloomTex[i] = makeColorTexture(mWidth / 2, mHeight / 2);
		glGenFramebuffers(1, &mBloomFBO[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, mBloomFBO[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mBloomTex[i], 0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::destroyTargets()
{
	if (mHdrFBO) glDeleteFramebuffers(1, &mHdrFBO);
	if (mHdrColor) glDeleteTextures(1, &mHdrColor);
	if (mHdrDepth) glDeleteRenderbuffers(1, &mHdrDepth);
	glDeleteFramebuffers(2, mBloomFBO);
	glDeleteTextures(2, mBloomTex);
	mHdrFBO = mHdrColor = mHdrDepth = 0;
	mBloomFBO[0] = mBloomFBO[1] = mBloomTex[0] = mBloomTex[1] = 0;
}

void PostProcessor::beginScene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mHdrFBO);
	glViewport(0, 0, mWidth, mHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessor::drawFullscreenTriangle() const
{
	glBindVertexArray(mEmptyVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
}

void PostProcessor::endScene(const PostSettings& s)
{
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// wireframe must not affect the post passes

	if (s.bloom)
	{
		// Bright pass (downsample to half resolution)
		glViewport(0, 0, mWidth / 2, mHeight / 2);
		glBindFramebuffer(GL_FRAMEBUFFER, mBloomFBO[0]);
		mBright.use();
		mBright.setUniform("threshold", 1.4f);
		mBright.setUniformSampler("image", 0);
		glBindTexture(GL_TEXTURE_2D, mHdrColor);
		drawFullscreenTriangle();

		// Gaussian blur, alternating horizontal and vertical passes
		mBlur.use();
		mBlur.setUniformSampler("image", 0);
		const int passes = 8;
		for (int i = 0; i < passes; ++i)
		{
			int src = i % 2, dst = 1 - src;
			glBindFramebuffer(GL_FRAMEBUFFER, mBloomFBO[dst]);
			glm::vec2 dir = (i % 2 == 0) ? glm::vec2(1.0f / (mWidth / 2), 0.0f) : glm::vec2(0.0f, 1.0f / (mHeight / 2));
			mBlur.setUniform("direction", dir * (1.0f + (float)(i / 2)));	// growing radius
			glBindTexture(GL_TEXTURE_2D, mBloomTex[src]);
			drawFullscreenTriangle();
		}
	}

	// Composite to the screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, mWidth, mHeight);
	mComposite.use();
	mComposite.setUniformSampler("sceneTex", 0);
	glBindTexture(GL_TEXTURE_2D, mHdrColor);
	mComposite.setUniformSampler("bloomTex", 1);
	glBindTexture(GL_TEXTURE_2D, mBloomTex[0]);
	mComposite.setUniformSampler("shadowTex", 2);
	glBindTexture(GL_TEXTURE_2D, s.shadowMapTexture);
	mComposite.setUniform("bloomOn", (GLint)(s.bloom ? 1 : 0));
	mComposite.setUniform("bloomStrength", s.bloomStrength);
	mComposite.setUniform("exposure", s.exposure);
	mComposite.setUniform("fade", s.fade);
	mComposite.setUniform("showShadowMap", (GLint)(s.showShadowMap ? 1 : 0));
	mComposite.setUniform("aspect", (float)mWidth / (float)mHeight);
	mComposite.setUniform("time", s.time);
	drawFullscreenTriangle();

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_DEPTH_TEST);
}
