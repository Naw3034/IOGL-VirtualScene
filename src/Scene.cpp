#include "Scene.h"

#include <cmath>
#include <iostream>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/constants.hpp"
#include "Paths.h"

namespace
{
	const float PI = glm::pi<float>();

	glm::mat4 T(const glm::vec3& p) { return glm::translate(glm::mat4(1.0f), p); }
	glm::mat4 S(const glm::vec3& s) { return glm::scale(glm::mat4(1.0f), s); }
	glm::mat4 S(float s) { return S(glm::vec3(s)); }
	glm::mat4 Ry(float deg) { return glm::rotate(glm::mat4(1.0f), glm::radians(deg), glm::vec3(0, 1, 0)); }
	glm::mat4 Rx(float deg) { return glm::rotate(glm::mat4(1.0f), glm::radians(deg), glm::vec3(1, 0, 0)); }
	glm::mat4 Rz(float deg) { return glm::rotate(glm::mat4(1.0f), glm::radians(deg), glm::vec3(0, 0, 1)); }

	float smooth(float e0, float e1, float x)
	{
		float t = glm::clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
		return t * t * (3.0f - 2.0f * t);
	}

	float flicker(float t, float seed)
	{
		return 0.5f + 0.25f * sinf(t * 7.3f + seed * 13.1f) + 0.15f * sinf(t * 17.9f + seed * 5.7f)
			+ 0.1f * sinf(t * 31.3f + seed * 2.3f);
	}

	const glm::vec3 LANTERN_COLOR(1.0f, 0.62f, 0.28f);
	const glm::vec3 LAMP_COLOR(0.55f, 0.75f, 1.0f);
	const glm::vec3 WISP_COLORS[2] = { glm::vec3(1.0f, 0.25f, 0.85f), glm::vec3(0.2f, 0.95f, 1.0f) };
	const glm::vec3 PIN_LANE(5.0f, 0.0f, -3.0f);
	const float BALL_START_Z = 3.5f;
}

Mesh* Scene::mesh(const std::string& file)
{
	auto it = mMeshes.find(file);
	if (it != mMeshes.end()) return it->second.get();
	auto m = std::make_unique<Mesh>();
	m->loadOBJ(resourcePath("assets/models/" + file));
	Mesh* ptr = m.get();
	mMeshes[file] = std::move(m);
	return ptr;
}

Texture2D* Scene::texture(const std::string& file)
{
	auto it = mTextures.find(file);
	if (it != mTextures.end()) return it->second.get();
	auto t = std::make_unique<Texture2D>();
	if (!t->loadTexture(resourcePath("assets/textures/" + file)))
		t->createSolidColor(glm::vec4(1, 0, 1, 1));
	Texture2D* ptr = t.get();
	mTextures[file] = std::move(t);
	return ptr;
}

Material* Scene::newMaterial()
{
	mMaterials.push_back(std::make_unique<Material>());
	Material* m = mMaterials.back().get();
	m->diffuseMap = &mWhite;
	m->specularMap = &mWhite;
	m->emissiveMap = &mBlack;
	return m;
}

SceneObject& Scene::add(const std::string& name, const std::string& meshFile, Material* mat, const glm::mat4& transform)
{
	SceneObject o;
	o.name = name;
	o.mesh = meshFile.empty() ? &mGround : mesh(meshFile);
	o.material = mat;
	o.transform = transform;
	mObjects.push_back(o);
	return mObjects.back();
}

bool Scene::load()
{
	mWhite.createSolidColor(glm::vec4(1.0f));
	mBlack.createSolidColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	Material* ground = newMaterial();
	ground->diffuseMap = texture("tile_floor.jpg");
	ground->specular = glm::vec3(0.15f);
	ground->shininess = 64.0f;
	ground->tint = glm::vec3(0.68f, 0.64f, 0.6f);

	Material* pedestal = newMaterial();
	pedestal->diffuseMap = texture("crate.jpg");
	pedestal->specular = glm::vec3(0.15f);
	pedestal->shininess = 16.0f;

	Material* robot = newMaterial();
	robot->diffuseMap = texture("robot_diffuse.jpg");
	robot->specular = glm::vec3(0.9f);
	robot->shininess = 64.0f;

	Material* barrel = newMaterial();
	barrel->diffuseMap = texture("barrel_diffuse.png");
	barrel->specularMap = texture("barrel_spec.png");
	barrel->specular = glm::vec3(1.2f);
	barrel->shininess = 64.0f;

	Material* woodcrate = newMaterial();
	woodcrate->diffuseMap = texture("woodcrate_diffuse.jpg");
	woodcrate->specular = glm::vec3(0.1f);
	woodcrate->shininess = 8.0f;

	Material* pin = newMaterial();
	pin->diffuseMap = texture("AMF.tga");
	pin->specular = glm::vec3(1.0f);
	pin->shininess = 96.0f;

	Material* ball = newMaterial();
	ball->tint = glm::vec3(0.05f, 0.07f, 0.3f);
	ball->specular = glm::vec3(1.5f);
	ball->shininess = 128.0f;

	Material* bunny = newMaterial();
	bunny->diffuseMap = texture("bunny_diffuse.jpg");
	bunny->specular = glm::vec3(0.3f);
	bunny->shininess = 24.0f;

	Material* fox = newMaterial();
	fox->diffuseMap = texture("fox_diffuse.jpg");
	fox->specular = glm::vec3(0.05f);
	fox->shininess = 8.0f;

	Material* lampPost = newMaterial();
	lampPost->diffuseMap = texture("lamp_post_diffuse.png");
	lampPost->specular = glm::vec3(0.6f);
	lampPost->shininess = 48.0f;

	mLanternMat = newMaterial();
	mLanternMat->diffuseMap = texture("lantern_diffuse.jpg");
	mLanternMat->emissiveMap = texture("lantern_emissive.jpg");
	mLanternMat->specular = glm::vec3(0.3f);
	mLanternMat->shininess = 32.0f;

	mBulbMat = newMaterial();
	mBulbMat->diffuseMap = texture("light_diffuse.jpg");
	mBulbMat->emissiveMap = &mWhite;

	for (int i = 0; i < 2; ++i)
	{
		mWispMat[i] = newMaterial();
		mWispMat[i]->tint = glm::vec3(0.0f);
		mWispMat[i]->emissiveMap = &mWhite;
		mWispMat[i]->emissive = WISP_COLORS[i] * 6.0f;
	}

	mGround.createPlane(240.0f, 120.0f);
	add("ground", "", ground, glm::mat4(1.0f)).castShadow = false;

	add("pedestal", "crate.obj", pedestal, S(glm::vec3(1.1f, 0.35f, 1.1f)));
	mRobot = (int)mObjects.size();
	add("robot", "robot.obj", robot, glm::mat4(1.0f));

	for (int i = 0; i < 2; ++i)
	{
		mWisps[i] = (int)mObjects.size();
		add("wisp", "light.obj", mWispMat[i], glm::mat4(1.0f)).castShadow = false;
	}

	const glm::vec3 lanternPos[4] = { {9, 0, 9}, {-9, 0, 9}, {-9, 0, -9}, {9, 0, -9} };
	for (const glm::vec3& p : lanternPos)
	{
		glm::vec3 d = glm::normalize(-p);
		float angle = glm::degrees(atan2f(-d.z, d.x));
		glm::mat4 m = T(p) * Ry(angle);
		mLanternObjects.push_back((int)mObjects.size());
		add("lantern", "lantern.obj", mLanternMat, m);
		mLanternLightPos.push_back(glm::vec3(m * glm::vec4(1.92f, 3.55f, 0.0f, 1.0f)));
	}

	const glm::vec3 lampPos[2] = { {-14, 0, 0}, {14, 0, 0} };
	for (const glm::vec3& p : lampPos)
	{
		add("lampPost", "lampPost.obj", lampPost, T(p));
		mLampBulbs.push_back((int)mObjects.size());
		add("bulb", "light.obj", mBulbMat, T(p + glm::vec3(0, 3.85f, 0)) * S(0.45f)).castShadow = false;
		mLampLightPos.push_back(p + glm::vec3(0, 3.85f, 0));
	}

	add("barrel", "barrel.obj", barrel, T({ -6.0f, 0, -4.6f }) * Ry(20) * S(0.3f));
	add("barrel", "barrel.obj", barrel, T({ -7.0f, 0, -5.3f }) * Ry(75) * S(0.3f));
	add("barrel", "barrel.obj", barrel, T({ -6.3f, 0, -6.1f }) * Ry(140) * S(0.3f));
	add("barrel", "barrel.obj", barrel, T({ -4.6f, 0.39f, -6.0f }) * Ry(35) * Rz(90) * S(0.3f) * T({ 0, -1.675f, 0 }));
	add("crate", "woodcrate.obj", woodcrate, T({ -8.2f, 0, -1.4f }) * Ry(5) * S(0.4f));
	add("crate", "woodcrate.obj", woodcrate, T({ -8.1f, 0, -2.3f }) * Ry(-8) * S(0.4f));
	add("crate", "woodcrate.obj", woodcrate, T({ -8.15f, 0.8f, -1.85f }) * Ry(30) * S(0.4f));
	add("crate", "woodcrate.obj", woodcrate, T({ 7.5f, 0, 5.0f }) * Ry(15) * S(0.5f));
	add("crate", "woodcrate.obj", woodcrate, T({ 8.4f, 0, 4.2f }) * Ry(-20) * S(0.35f));
	add("barrel", "barrel.obj", barrel, T({ 6.6f, 0, 6.0f }) * Ry(-50) * S(0.3f));

	for (int row = 0; row < 4; ++row)
	{
		for (int i = 0; i <= row; ++i)
		{
			glm::vec3 p = PIN_LANE + glm::vec3((i - row * 0.5f) * 0.32f, 0.0f, -row * 0.3f);
			mPins.push_back((int)mObjects.size());
			mPinBase.push_back(p);
			add("pin", "bowling_pin.obj", pin, T(p) * S(0.025f));
		}
	}
	mBall = (int)mObjects.size();
	add("ball", "light.obj", ball, glm::mat4(1.0f));

	mBunny = (int)mObjects.size();
	add("bunny", "bunny.obj", bunny, glm::mat4(1.0f));
	mFox = (int)mObjects.size();
	add("fox", "fox.obj", fox, glm::mat4(1.0f));

	mPointLights.resize(MAX_POINT_LIGHTS);
	for (int i = 0; i < 4; ++i)
	{
		mPointLights[i].position = mLanternLightPos[i];
		mPointLights[i].linear = 0.09f;
		mPointLights[i].exponent = 0.032f;
	}
	for (int i = 0; i < 2; ++i)
	{
		mPointLights[4 + i].position = mLampLightPos[i];
		mPointLights[4 + i].linear = 0.14f;
		mPointLights[4 + i].exponent = 0.07f;
	}
	for (int i = 0; i < 2; ++i)
	{
		mPointLights[6 + i].linear = 0.35f;
		mPointLights[6 + i].exponent = 0.44f;
	}

	mFlashlight.diffuse = glm::vec3(1.6f, 1.5f, 1.3f);
	mFlashlight.specular = glm::vec3(1.0f);

	std::cout << "Scene ready: " << mObjects.size() << " objects, " << triangleCount() << " triangles" << std::endl;
	return true;
}

size_t Scene::triangleCount() const
{
	size_t n = 0;
	for (const SceneObject& o : mObjects) n += o.mesh->triangleCount();
	return n;
}

void Scene::update(float animTime, const RenderSettings& settings)
{
	updateEnvironment(settings.timeOfDay);
	animateObjects(animTime);

	for (int i = 0; i < 4; ++i)
	{
		float f = 0.85f + 0.3f * flicker(animTime, (float)i);
		mPointLights[i].diffuse = LANTERN_COLOR * 3.0f * f * mEnv.lampsOn;
		mPointLights[i].specular = mPointLights[i].diffuse;
	}
	mLanternMat->emissive = LANTERN_COLOR * 5.0f * mEnv.lampsOn * (0.9f + 0.2f * flicker(animTime, 1.5f));

	for (int i = 0; i < 2; ++i)
	{
		mPointLights[4 + i].diffuse = LAMP_COLOR * 2.2f * mEnv.lampsOn;
		mPointLights[4 + i].specular = mPointLights[4 + i].diffuse;
	}
	mBulbMat->emissive = LAMP_COLOR * 6.0f * mEnv.lampsOn;

	for (int i = 0; i < 2; ++i)
	{
		mPointLights[6 + i].position = glm::vec3(mObjects[mWisps[i]].transform[3]);
		float pulse = 0.8f + 0.2f * sinf(animTime * 3.0f + i * PI);
		mPointLights[6 + i].diffuse = WISP_COLORS[i] * 2.5f * pulse;
		mPointLights[6 + i].specular = mPointLights[6 + i].diffuse;
		mWispMat[i]->emissive = WISP_COLORS[i] * 6.0f * pulse;
	}

	mFlashlight.on = settings.flashlight;
}

void Scene::updateEnvironment(float tod)
{
	float angle = (tod - 6.0f) / 24.0f * 2.0f * PI;
	mEnv.sunDir = glm::normalize(glm::vec3(cosf(angle), sinf(angle) * 0.9f, 0.45f));
	mEnv.moonDir = glm::normalize(glm::vec3(-mEnv.sunDir.x, -mEnv.sunDir.y, -0.35f));

	float e = mEnv.sunDir.y;
	float day = smooth(-0.05f, 0.3f, e);
	float night = 1.0f - smooth(-0.25f, 0.0f, e);
	float sunset = expf(-(e * e) / (0.22f * 0.22f));
	mEnv.dayFactor = day;
	mEnv.nightFactor = night;
	mEnv.lampsOn = 1.0f - smooth(-0.04f, 0.12f, e);

	mEnv.zenithColor = glm::mix(glm::vec3(0.004f, 0.007f, 0.02f), glm::vec3(0.06f, 0.18f, 0.55f), day)
		+ glm::vec3(0.12f, 0.06f, 0.12f) * sunset;
	mEnv.horizonColor = glm::mix(glm::vec3(0.015f, 0.02f, 0.045f), glm::vec3(0.4f, 0.55f, 0.8f), day)
		+ glm::vec3(0.7f, 0.27f, 0.07f) * sunset;
	mEnv.fogColor = mEnv.horizonColor * 0.85f;
	mEnv.fogDensity = glm::mix(0.009f, 0.026f, night);
	mEnv.ambient = glm::mix(glm::vec3(0.025f, 0.03f, 0.055f), glm::vec3(0.17f, 0.19f, 0.24f), day)
		+ glm::vec3(0.08f, 0.04f, 0.02f) * sunset;
	mEnv.exposure = glm::mix(1.3f, 1.8f, night);

	glm::vec3 sunColor = glm::mix(glm::vec3(1.0f, 0.45f, 0.18f), glm::vec3(1.0f, 0.95f, 0.88f), smooth(0.0f, 0.45f, e));
	mEnv.sunColor = sunColor;
	if (e > 0.0f)
	{
		mMainLight.direction = -mEnv.sunDir;
		mMainLight.diffuse = sunColor * 3.2f * smooth(0.0f, 0.12f, e);
	}
	else
	{
		mMainLight.direction = -mEnv.moonDir;
		mMainLight.diffuse = glm::vec3(0.3f, 0.4f, 0.75f) * 0.35f * smooth(0.0f, 0.12f, -e);
	}
	mMainLight.specular = mMainLight.diffuse;
}

void Scene::animateObjects(float t)
{
	float bob = 0.06f * sinf(t * 1.5f);
	mObjects[mRobot].transform = T({ 0.0f, 0.7f + 0.06f + bob, 0.0f }) * Ry(t * 25.0f) * S(0.33f);

	for (int i = 0; i < 2; ++i)
	{
		float a = t * 0.9f + i * PI;
		glm::vec3 p(2.1f * cosf(a), 2.4f + 0.5f * sinf(t * 1.3f + i * 2.0f), 2.1f * sinf(a));
		mObjects[mWisps[i]].transform = T(p) * S(0.35f);
	}

	{
		const float R = 4.2f, speed = -0.22f, hopPeriod = 0.55f;
		float a = t * speed;
		glm::vec3 p(R * cosf(a), 0.0f, R * sinf(a));
		float phase = fmodf(t, hopPeriod) / hopPeriod;
		p.y = 0.35f * sinf(phase * PI);
		glm::vec3 v(-sinf(a) * speed, 0.0f, cosf(a) * speed);
		float heading = glm::degrees(atan2f(v.x, v.z));
		float squash = 1.0f + 0.12f * sinf(phase * 2.0f * PI);
		mObjects[mBunny].transform = T(p) * Ry(heading) * Ry(90.0f) * S(glm::vec3(0.19f, 0.19f * squash, 0.19f));
	}

	{
		const float R = 7.0f, speed = 0.11f;
		float a = t * speed + 1.0f;
		glm::vec3 p(R * cosf(a), 0.03f * fabsf(sinf(t * 7.0f)), R * sinf(a));
		glm::vec3 v(-sinf(a), 0.0f, cosf(a));
		float heading = glm::degrees(atan2f(v.x, v.z));
		mObjects[mFox].transform = T(p) * Ry(heading) * Rz(2.0f * sinf(t * 3.5f)) * S(0.55f);
		mFoxPos = p;
	}

	{
		const float period = 12.0f, impact = 2.5f;
		float u = fmodf(t + 0.5f, period);
		const float r = 0.22f;
		float frontZ = PIN_LANE.z + 0.15f;
		float z, scale = 1.0f;
		if (u < impact)
			z = BALL_START_Z - (BALL_START_Z - frontZ) * (u / impact);
		else
		{
			float k = glm::min((u - impact) / 1.5f, 1.0f);
			z = frontZ - 1.6f * (1.0f - (1.0f - k) * (1.0f - k));
		}
		if (u > 9.0f && u < 10.5f) scale = 0.0f;
		else if (u >= 10.5f) { scale = smooth(10.5f, 11.0f, u); z = BALL_START_Z; }
		if (u > 8.5f && u <= 9.0f) scale = 1.0f - smooth(8.5f, 9.0f, u);
		float spin = glm::degrees((BALL_START_Z - z) / r);
		mObjects[mBall].transform = T({ PIN_LANE.x, r * scale, z }) * Rx(-spin) * S(scale);

		for (size_t i = 0; i < mPins.size(); ++i)
		{
			glm::vec3 base = mPinBase[i];
			int row = (int)(fabsf(base.z - PIN_LANE.z) / 0.3f + 0.5f);
			float start = impact + 0.07f * row + 0.03f * (float)(i % 3);
			float fall = smooth(start, start + 0.45f, u) * (1.0f - smooth(9.5f, 10.3f, u));
			glm::vec3 dir = glm::normalize(glm::vec3((base.x - PIN_LANE.x) * 2.5f + 0.15f * (float)((int)i % 2 * 2 - 1), 0.0f, -1.0f));
			glm::vec3 axis = glm::cross(glm::vec3(0, 1, 0), dir);
			glm::vec3 slide = dir * 0.25f * fall;
			glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(84.0f) * fall, axis);
			mObjects[mPins[i]].transform = T(base + slide + glm::vec3(0, 0.06f * fall, 0)) * rot * S(0.025f);
		}
	}
}

void Scene::applyLights(ShaderProgram& shader) const
{
	mMainLight.apply(shader, "sunLight");
	shader.setUniform("numPointLights", (GLint)mPointLights.size());
	for (size_t i = 0; i < mPointLights.size(); ++i)
		mPointLights[i].apply(shader, "pointLights[" + std::to_string(i) + "]");
	mFlashlight.apply(shader, "spotLight");
	shader.setUniform("ambientColor", mEnv.ambient);
}

void Scene::draw(ShaderProgram& shader, bool depthOnly) const
{
	if (!depthOnly)
	{
		shader.setUniformSampler("material.diffuseMap", 0);
		shader.setUniformSampler("material.specularMap", 1);
		shader.setUniformSampler("material.emissiveMap", 2);
	}

	for (const SceneObject& o : mObjects)
	{
		if (!o.visible || (depthOnly && !o.castShadow)) continue;
		if (o.transform[0][0] == 0.0f && o.transform[1][1] == 0.0f) continue;

		shader.setUniform("model", o.transform);
		if (!depthOnly)
		{
			shader.setUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(o.transform))));
			const Material& m = *o.material;
			shader.setUniform("material.tint", m.tint);
			shader.setUniform("material.specular", m.specular);
			shader.setUniform("material.shininess", m.shininess);
			shader.setUniform("material.emissive", m.emissive);
			shader.setUniform("uvScale", m.uvScale);
			m.diffuseMap->bind(0);
			m.specularMap->bind(1);
			m.emissiveMap->bind(2);
		}
		o.mesh->draw();
	}
}
