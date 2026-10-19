//-----------------------------------------------------------------------------
// Fragment shader - Blinn-Phong lighting with
//   1 directional light (sun / moon) with shadow mapping (3x3 PCF)
//   up to 8 point lights (lanterns, lamp posts, wisps)
//   1 spotlight (flashlight)
//   diffuse / specular / emissive maps, distance fog
// Lighting is computed in linear space; gamma is applied in the post pass.
//-----------------------------------------------------------------------------
#version 330 core

#define MAX_POINT_LIGHTS 8

struct Material
{
	sampler2D diffuseMap;
	sampler2D specularMap;
	sampler2D emissiveMap;
	vec3 tint;
	vec3 specular;
	float shininess;
	vec3 emissive;
};

struct DirectionalLight
{
	vec3 direction;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	float constant;
	float linear;
	float exponent;
};

struct SpotLight
{
	vec3 position;
	vec3 direction;
	float cosInnerCone;
	float cosOuterCone;
	vec3 diffuse;
	vec3 specular;
	int on;
	float constant;
	float linear;
	float exponent;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

uniform DirectionalLight sunLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;
uniform SpotLight spotLight;
uniform Material material;
uniform vec3 viewPos;
uniform vec3 ambientColor;

uniform sampler2D shadowMap;
uniform int shadowsOn;

uniform int fogOn;
uniform vec3 fogColor;
uniform float fogDensity;

out vec4 frag_color;

vec3 albedo;
float specMask;

//-----------------------------------------------------------------------------
// Blinn-Phong for one light: diffuse (N.L) + specular (N.H)^shininess
//-----------------------------------------------------------------------------
vec3 blinnPhong(vec3 lightDiffuse, vec3 lightSpecular, vec3 L, vec3 N, vec3 V)
{
	float NdotL = max(dot(N, L), 0.0);
	vec3 H = normalize(L + V);
	float spec = pow(max(dot(N, H), 0.0), material.shininess) * step(0.0, NdotL);
	return lightDiffuse * NdotL * albedo + lightSpecular * material.specular * specMask * spec;
}

//-----------------------------------------------------------------------------
// 1.0 = lit, 0.0 = in shadow. Percentage Closer Filtering on a 3x3 kernel
// gives soft shadow edges.
//-----------------------------------------------------------------------------
float shadowFactor(vec3 N, vec3 L)
{
	if (shadowsOn == 0) return 1.0;
	vec3 p = FragPosLightSpace.xyz / FragPosLightSpace.w * 0.5 + 0.5;
	if (p.z > 1.0) return 1.0;

	float bias = max(0.0015 * (1.0 - dot(N, L)), 0.0004);
	vec2 texel = 1.0 / vec2(textureSize(shadowMap, 0));
	float lit = 0.0;
	for (int x = -1; x <= 1; ++x)
		for (int y = -1; y <= 1; ++y)
		{
			float closest = texture(shadowMap, p.xy + vec2(x, y) * texel).r;
			lit += (p.z - bias > closest) ? 0.0 : 1.0;
		}
	return lit / 9.0;
}

float attenuation(vec3 lightPos, float kc, float kl, float kq)
{
	float d = length(lightPos - FragPos);
	return 1.0 / (kc + kl * d + kq * d * d);
}

void main()
{
	vec3 N = normalize(Normal);
	if (!gl_FrontFacing) N = -N;		// thin, double-sided geometry
	vec3 V = normalize(viewPos - FragPos);

	// sRGB textures -> linear space
	albedo = pow(texture(material.diffuseMap, TexCoord).rgb, vec3(2.2)) * material.tint;
	specMask = texture(material.specularMap, TexCoord).r;

	// Ambient (sky light)
	vec3 color = ambientColor * albedo;

	// Directional light (sun or moon) with shadows
	vec3 Ld = normalize(-sunLight.direction);
	color += blinnPhong(sunLight.diffuse, sunLight.specular, Ld, N, V) * shadowFactor(N, Ld);

	// Point lights
	for (int i = 0; i < numPointLights; ++i)
	{
		vec3 L = normalize(pointLights[i].position - FragPos);
		float att = attenuation(pointLights[i].position, pointLights[i].constant, pointLights[i].linear, pointLights[i].exponent);
		color += blinnPhong(pointLights[i].diffuse, pointLights[i].specular, L, N, V) * att;
	}

	// Spotlight (flashlight) with a smooth cone edge
	if (spotLight.on == 1)
	{
		vec3 L = normalize(spotLight.position - FragPos);
		float cosDir = dot(-L, normalize(spotLight.direction));
		float cone = smoothstep(spotLight.cosOuterCone, spotLight.cosInnerCone, cosDir);
		float att = attenuation(spotLight.position, spotLight.constant, spotLight.linear, spotLight.exponent);
		color += blinnPhong(spotLight.diffuse, spotLight.specular, L, N, V) * att * cone;
	}

	// Self-illumination (lantern glass, bulbs, wisps): HDR values > 1 feed the bloom
	color += pow(texture(material.emissiveMap, TexCoord).rgb, vec3(2.2)) * material.emissive;

	// Exponential squared fog, blended with the sky horizon colour
	if (fogOn == 1)
	{
		float d = length(viewPos - FragPos);
		float f = 1.0 - exp(-pow(d * fogDensity, 2.0));
		color = mix(color, fogColor, clamp(f, 0.0, 1.0));
	}

	frag_color = vec4(color, 1.0);
}
