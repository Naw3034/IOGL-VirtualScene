#ifndef FIREFLIES_H
#define FIREFLIES_H

#include <vector>
#include "GL/glew.h"
#include "glm/glm.hpp"
#include "ShaderProgram.h"

class Fireflies
{
public:
	explicit Fireflies(int count = 220);
	~Fireflies();
	Fireflies(const Fireflies&) = delete;
	Fireflies& operator=(const Fireflies&) = delete;

	bool init();
	void update(float time);
	void draw(const glm::mat4& view, const glm::mat4& projection, float viewportHeight, float intensity);

private:
	struct Particle
	{
		glm::vec3 home;
		glm::vec3 freq;
		glm::vec3 phase;
		float blinkSpeed;
	};

	int mCount;
	std::vector<Particle> mParticles;
	std::vector<glm::vec4> mGpuData;
	GLuint mVAO = 0, mVBO = 0;
	ShaderProgram mShader;
};

#endif
