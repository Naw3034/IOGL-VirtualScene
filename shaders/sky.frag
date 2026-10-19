//-----------------------------------------------------------------------------
// Procedural sky: gradient + sun + moon + twinkling stars (HDR output)
//-----------------------------------------------------------------------------
#version 330 core

in vec2 ndc;

uniform mat4 invViewProj;		// inverse(projection * view without translation)
uniform vec3 zenithColor;
uniform vec3 horizonColor;
uniform vec3 sunDir;
uniform vec3 sunColor;
uniform vec3 moonDir;
uniform float nightFactor;
uniform float time;

out vec4 frag_color;

float hash(vec3 p)
{
	p = fract(p * 0.3183099 + 0.1);
	p *= 17.0;
	return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

void main()
{
	vec4 world = invViewProj * vec4(ndc, 1.0, 1.0);
	vec3 dir = normalize(world.xyz / world.w);
	float h = dir.y;

	// Gradient: horizon -> zenith, and a darker "ground" below the horizon
	vec3 col = mix(horizonColor, zenithColor, pow(clamp(h, 0.0, 1.0), 0.45));
	col = mix(col, horizonColor * 0.35, smoothstep(0.0, -0.25, h));

	// Sun: sharp disc + wide halo
	float sd = max(dot(dir, normalize(sunDir)), 0.0);
	float sunVisible = smoothstep(-0.1, 0.02, sunDir.y);
	col += sunColor * (smoothstep(0.9993, 0.9996, sd) * 40.0 + pow(sd, 12.0) * 0.5 + pow(sd, 200.0) * 2.0) * sunVisible;

	// Moon
	float md = max(dot(dir, normalize(moonDir)), 0.0);
	col += vec3(0.75, 0.82, 1.0) * (smoothstep(0.99965, 0.9998, md) * 4.0 + pow(md, 60.0) * 0.12) * nightFactor;

	// Stars: one random point per cell of a 3D grid projected on the sphere
	vec3 p = dir * 180.0;
	vec3 cell = floor(p);
	float r = hash(cell);
	if (r > 0.92 && h > 0.0)
	{
		vec3 starPos = cell + 0.5 + 0.35 * (vec3(hash(cell + 1.3), hash(cell + 2.7), hash(cell + 4.1)) - 0.5);
		float d = length(p - starPos);
		float twinkle = 0.6 + 0.4 * sin(time * (2.0 + 4.0 * r) + r * 50.0);
		col += vec3(0.9, 0.95, 1.0) * smoothstep(0.22, 0.0, d) * twinkle * 2.5 * nightFactor * smoothstep(0.0, 0.2, h);
	}

	// alpha = how much this pixel may feed the bloom (the sky only a little)
	frag_color = vec4(col, 0.2);
}
