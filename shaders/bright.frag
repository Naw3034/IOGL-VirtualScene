#version 330 core

in vec2 uv;

uniform sampler2D image;
uniform float threshold;

out vec4 frag_color;

void main()
{
	vec4 src = texture(image, uv);
	vec3 c = src.rgb;
	float luma = dot(c, vec3(0.2126, 0.7152, 0.0722));
	float k = smoothstep(threshold, threshold + 0.8, luma) * clamp(src.a, 0.0, 1.0);
	frag_color = vec4(min(c * k, vec3(50.0)), 1.0);
}
