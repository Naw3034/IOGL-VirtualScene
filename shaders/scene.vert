//-----------------------------------------------------------------------------
// Vertex shader - lit scene
//-----------------------------------------------------------------------------
#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 model;
uniform mat3 normalMatrix;		// transpose(inverse(model)), computed on the CPU
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;	// world -> shadow map
uniform vec2 uvScale;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec4 FragPosLightSpace;

void main()
{
	vec4 worldPos = model * vec4(pos, 1.0);
	FragPos = worldPos.xyz;
	Normal = normalMatrix * normal;
	TexCoord = texCoord * uvScale;
	FragPosLightSpace = lightSpaceMatrix * worldPos;
	gl_Position = projection * view * worldPos;
}
