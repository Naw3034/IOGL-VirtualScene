#include "Fireflies.h"

#include <cmath>
#include <random>
#include "Paths.h"

Fireflies::Fireflies(int count)
	: mCount(count)
{
}

Fireflies::~Fireflies()
{
	if (mVBO) glDeleteBuffers(1, &mVBO);
	if (mVAO) glDeleteVertexArrays(1, &mVAO);
}

bool Fireflies::init()
{
	std::mt19937 rng(42);
	std::uniform_real_distribution<float> u01(0.0f, 1.0f);

	for (int i = 0; i < mCount; ++i)
	{
		Particle p;
		float a = u01(rng) * 6.2831853f;
		float r = 2.5f + 13.0f * std::sqrt(u01(rng));
		p.home = glm::vec3(r * std::cos(a), 0.3f + 2.6f * u01(rng), r * std::sin(a));
		p.freq = glm::vec3(0.15f + 0.35f * u01(rng), 0.3f + 0.5f * u01(rng), 0.15f + 0.35f * u01(rng));
		p.phase = glm::vec3(u01(rng), u01(rng), u01(rng)) * 6.2831853f;
		p.blinkSpeed = 0.6f + 1.8f * u01(rng);
		mParticles.push_back(p);
	}
	mGpuData.resize(mCount);

	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);
	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, mCount * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	return mShader.loadShaders(resourcePath("shaders/particles.vert").c_str(), resourcePath("shaders/particles.frag").c_str());
}

void Fireflies::update(float t)
{
	for (int i = 0; i < mCount; ++i)
	{
		const Particle& p = mParticles[i];
		glm::vec3 offset(
			1.6f * std::sin(t * p.freq.x + p.phase.x),
			0.45f * std::sin(t * p.freq.y + p.phase.y),
			1.6f * std::cos(t * p.freq.z + p.phase.z));
		float blink = 0.5f + 0.5f * std::sin(t * p.blinkSpeed + p.phase.x * 3.0f);
		blink = blink * blink * blink;
		mGpuData[i] = glm::vec4(p.home + offset, 0.05f + blink);
	}
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, mCount * sizeof(glm::vec4), mGpuData.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Fireflies::draw(const glm::mat4& view, const glm::mat4& projection, float viewportHeight, float intensity)
{
	if (intensity <= 0.01f) return;

	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(GL_FALSE);

	mShader.use();
	mShader.setUniform("view", view);
	mShader.setUniform("projection", projection);
	mShader.setUniform("viewportHeight", viewportHeight);
	mShader.setUniform("intensity", intensity);
	glBindVertexArray(mVAO);
	glDrawArrays(GL_POINTS, 0, mCount);
	glBindVertexArray(0);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}
