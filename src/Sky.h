#ifndef SKY_H
#define SKY_H

#include "glm/glm.hpp"
#include "ShaderProgram.h"

struct Environment;
class PostProcessor;

class Sky
{
public:
	bool init();
	void draw(const PostProcessor& post, const Environment& env, const glm::mat4& view, const glm::mat4& projection, float time);

private:
	ShaderProgram mShader;
};

#endif
