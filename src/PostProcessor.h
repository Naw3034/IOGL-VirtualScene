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

	void beginScene();
	void endScene(const PostSettings& s);

	void drawFullscreenTriangle() const;

private:
	void createTargets();
	void destroyTargets();

	int mWidth = 0, mHeight = 0;
	GLuint mHdrFBO = 0, mHdrColor = 0, mHdrDepth = 0;
	GLuint mBloomFBO[2] = { 0, 0 }, mBloomTex[2] = { 0, 0 };
	GLuint mEmptyVAO = 0;

	ShaderProgram mBright, mBlur, mComposite;
};

#endif
