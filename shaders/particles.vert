#version 330 core

layout (location = 0) in vec4 particle;

uniform mat4 view;
uniform mat4 projection;
uniform float viewportHeight;

out float brightness;

void main()
{
	gl_Position = projection * view * vec4(particle.xyz, 1.0);
	gl_PointSize = clamp(0.07 * viewportHeight / gl_Position.w, 1.5, 48.0);
	brightness = particle.w;
}
