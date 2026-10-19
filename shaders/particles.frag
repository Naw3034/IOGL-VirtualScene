#version 330 core

in float brightness;

uniform float intensity;

out vec4 frag_color;

void main()
{
	vec2 c = gl_PointCoord * 2.0 - 1.0;
	float r2 = dot(c, c);
	if (r2 > 1.0) discard;
	float glow = exp(-r2 * 5.0);
	vec3 color = vec3(0.75, 1.0, 0.35) * glow * brightness * intensity * 6.0;
	frag_color = vec4(color, glow);
}
