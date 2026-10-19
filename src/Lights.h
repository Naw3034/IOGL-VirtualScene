//-----------------------------------------------------------------------------
// Light sources (Blinn-Phong model, same structures as in the GLSL shader)
//-----------------------------------------------------------------------------
#ifndef LIGHTS_H
#define LIGHTS_H

#include <string>
#include "glm/glm.hpp"
#include "ShaderProgram.h"

struct DirectionalLight
{
	glm::vec3 direction{ 0.0f, -1.0f, 0.0f };	// direction the light travels
	glm::vec3 diffuse{ 1.0f };
	glm::vec3 specular{ 1.0f };

	void apply(ShaderProgram& shader, const std::string& name) const
	{
		shader.setUniform((name + ".direction").c_str(), direction);
		shader.setUniform((name + ".diffuse").c_str(), diffuse);
		shader.setUniform((name + ".specular").c_str(), specular);
	}
};

struct PointLight
{
	glm::vec3 position{ 0.0f };
	glm::vec3 diffuse{ 1.0f };
	glm::vec3 specular{ 1.0f };
	float constant = 1.0f;		// attenuation = 1 / (Kc + Kl*d + Kq*d^2)
	float linear = 0.14f;
	float exponent = 0.07f;

	void apply(ShaderProgram& shader, const std::string& name) const
	{
		shader.setUniform((name + ".position").c_str(), position);
		shader.setUniform((name + ".diffuse").c_str(), diffuse);
		shader.setUniform((name + ".specular").c_str(), specular);
		shader.setUniform((name + ".constant").c_str(), constant);
		shader.setUniform((name + ".linear").c_str(), linear);
		shader.setUniform((name + ".exponent").c_str(), exponent);
	}
};

struct SpotLight
{
	glm::vec3 position{ 0.0f };
	glm::vec3 direction{ 0.0f, 0.0f, -1.0f };
	glm::vec3 diffuse{ 1.0f };
	glm::vec3 specular{ 1.0f };
	float innerAngle = 12.0f;	// degrees
	float outerAngle = 18.0f;
	float constant = 1.0f;
	float linear = 0.07f;
	float exponent = 0.017f;
	bool on = false;

	void apply(ShaderProgram& shader, const std::string& name) const
	{
		shader.setUniform((name + ".position").c_str(), position);
		shader.setUniform((name + ".direction").c_str(), direction);
		shader.setUniform((name + ".diffuse").c_str(), diffuse);
		shader.setUniform((name + ".specular").c_str(), specular);
		shader.setUniform((name + ".cosInnerCone").c_str(), glm::cos(glm::radians(innerAngle)));
		shader.setUniform((name + ".cosOuterCone").c_str(), glm::cos(glm::radians(outerAngle)));
		shader.setUniform((name + ".constant").c_str(), constant);
		shader.setUniform((name + ".linear").c_str(), linear);
		shader.setUniform((name + ".exponent").c_str(), exponent);
		shader.setUniform((name + ".on").c_str(), (GLint)(on ? 1 : 0));
	}
};

#endif // LIGHTS_H
