//-----------------------------------------------------------------------------
// Final pass: HDR scene + bloom -> exposure -> ACES tone mapping -> gamma
//             + vignette, fades and the shadow map debug view
//-----------------------------------------------------------------------------
#version 330 core

in vec2 uv;

uniform sampler2D sceneTex;
uniform sampler2D bloomTex;
uniform sampler2D shadowTex;
uniform int bloomOn;
uniform float bloomStrength;
uniform float exposure;
uniform float fade;
uniform int showShadowMap;
uniform float aspect;
uniform float time;

out vec4 frag_color;

// Filmic tone mapping curve (Narkowicz' fit of ACES)
vec3 aces(vec3 x)
{
	return clamp((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14), 0.0, 1.0);
}

void main()
{
	vec3 hdr = texture(sceneTex, uv).rgb;
	if (bloomOn == 1)
		hdr += texture(bloomTex, uv).rgb * bloomStrength;

	vec3 color = aces(hdr * exposure);
	color = pow(color, vec3(1.0 / 2.2));	// linear -> sRGB

	// Vignette
	vec2 d = uv - 0.5;
	d.x *= aspect;
	color *= mix(1.0, 0.72, smoothstep(0.35, 1.0, length(d)));

	// Debug: shadow map in the bottom-right corner
	if (showShadowMap == 1)
	{
		vec2 size = vec2(0.32 / aspect, 0.32) * 1.0;
		vec2 origin = vec2(1.0 - size.x - 0.02, 0.03);
		vec2 local = (uv - origin) / size;
		if (all(greaterThanEqual(local, vec2(0.0))) && all(lessThanEqual(local, vec2(1.0))))
		{
			float depth = texture(shadowTex, local).r;
			color = vec3(clamp((depth - 0.2) * 1.6, 0.0, 1.0));	// stretch the useful depth range
		}
		else if (all(greaterThanEqual(local, vec2(-0.012))) && all(lessThanEqual(local, vec2(1.012))))
			color = vec3(1.0);
	}

	// Tiny noise against banding in the dark gradients
	float n = fract(sin(dot(uv * (1.0 + fract(time)), vec2(12.9898, 78.233))) * 43758.5453);
	color += (n - 0.5) / 255.0;

	frag_color = vec4(color * fade, 1.0);
}
