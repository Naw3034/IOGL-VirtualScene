#version 330 core

in vec2 uv;

uniform sampler2D image;
uniform vec2 direction;

out vec4 frag_color;

const float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
	vec3 result = texture(image, uv).rgb * weights[0];
	for (int i = 1; i < 5; ++i)
	{
		result += texture(image, uv + direction * float(i)).rgb * weights[i];
		result += texture(image, uv - direction * float(i)).rgb * weights[i];
	}
	frag_color = vec4(result, 1.0);
}
