//-----------------------------------------------------------------------------
// Scene: loads the models / textures / materials, places the objects,
// animates them and drives the lights (day/night cycle, lanterns, wisps).
//-----------------------------------------------------------------------------
#ifndef SCENE_H
#define SCENE_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "Lights.h"
#include "Mesh.h"
#include "RenderSettings.h"
#include "ShaderProgram.h"
#include "Texture2D.h"

// Surface description used by the Blinn-Phong shader
struct Material
{
	const Texture2D* diffuseMap = nullptr;
	const Texture2D* specularMap = nullptr;	// red channel = specular mask
	const Texture2D* emissiveMap = nullptr;
	glm::vec3 tint{ 1.0f };					// multiplies the diffuse texture
	glm::vec3 specular{ 0.5f };
	float shininess = 32.0f;
	glm::vec3 emissive{ 0.0f };				// multiplies the emissive map (HDR)
	glm::vec2 uvScale{ 1.0f };
};

struct SceneObject
{
	std::string name;
	const Mesh* mesh = nullptr;
	Material* material = nullptr;
	glm::mat4 transform{ 1.0f };
	bool castShadow = true;
	bool visible = true;
};

// Sky / fog / ambient values computed from the time of day
struct Environment
{
	glm::vec3 sunDir{ 0, 1, 0 };	// direction TOWARDS the sun
	glm::vec3 moonDir{ 0, -1, 0 };
	glm::vec3 sunColor{ 1.0f };
	glm::vec3 zenithColor{ 0.2f, 0.4f, 0.8f };
	glm::vec3 horizonColor{ 0.6f, 0.7f, 0.9f };
	glm::vec3 ambient{ 0.2f };
	glm::vec3 fogColor{ 0.5f };
	float fogDensity = 0.02f;
	float dayFactor = 1.0f;			// 1 = full day
	float nightFactor = 0.0f;		// 1 = full night
	float lampsOn = 0.0f;			// street lights intensity 0..1
	float exposure = 1.0f;
};

class Scene
{
public:
	static const int MAX_POINT_LIGHTS = 8;

	bool load();
	void update(float animTime, const RenderSettings& settings);

	// Binds the material and draws every object. depthOnly = shadow pass.
	void draw(ShaderProgram& shader, bool depthOnly) const;
	void applyLights(ShaderProgram& shader) const;

	const Environment& environment() const { return mEnv; }
	const DirectionalLight& mainLight() const { return mMainLight; }
	SpotLight& flashlight() { return mFlashlight; }
	size_t triangleCount() const;

	glm::vec3 robotPosition() const { return glm::vec3(0.0f, 2.0f, 0.0f); }
	glm::vec3 foxPosition() const { return mFoxPos; }

private:
	Mesh* mesh(const std::string& file);
	Texture2D* texture(const std::string& file);
	Material* newMaterial();
	SceneObject& add(const std::string& name, const std::string& meshFile, Material* mat, const glm::mat4& transform);

	void updateEnvironment(float timeOfDay);
	void animateObjects(float t);

	std::map<std::string, std::unique_ptr<Mesh>> mMeshes;
	std::map<std::string, std::unique_ptr<Texture2D>> mTextures;
	std::vector<std::unique_ptr<Material>> mMaterials;
	std::vector<SceneObject> mObjects;
	Mesh mGround;

	Texture2D mWhite, mBlack;

	// Indices of animated objects
	int mRobot = -1, mBunny = -1, mFox = -1, mBall = -1;
	int mWisps[2] = { -1, -1 };
	std::vector<int> mPins;
	std::vector<glm::vec3> mPinBase;
	std::vector<int> mLanternObjects;
	std::vector<int> mLampBulbs;
	Material* mLanternMat = nullptr;
	Material* mBulbMat = nullptr;
	Material* mWispMat[2] = { nullptr, nullptr };

	// Lights
	Environment mEnv;
	DirectionalLight mMainLight;
	std::vector<PointLight> mPointLights;
	std::vector<glm::vec3> mLanternLightPos, mLampLightPos;
	SpotLight mFlashlight;
	glm::vec3 mFoxPos{ 0.0f };
};

#endif // SCENE_H
