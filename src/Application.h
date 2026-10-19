//-----------------------------------------------------------------------------
// Application: window, input, camera modes and the rendering pipeline
//
//   shadow pass  ->  HDR pass (sky, lit scene, particles)  ->  bloom + tone mapping
//-----------------------------------------------------------------------------
#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "Camera.h"
#include "CameraPath.h"
#include "DemoScript.h"
#include "Fireflies.h"
#include "PostProcessor.h"
#include "RenderSettings.h"
#include "Scene.h"
#include "ShaderProgram.h"
#include "ShadowMap.h"
#include "Sky.h"
#include "VideoRecorder.h"

struct AppOptions
{
	bool demo = false;				// play the scripted presentation in real time
	std::string recordFile;			// non empty = offline rendering to a video
	int width = 1280;
	int height = 720;
	int fps = 30;
	float startTime = 0.0f;			// part of the script to render (seconds)
	float endTime = -1.0f;			// -1 = until the end of the script
	bool fullscreen = false;
};

class Application
{
public:
	Application() = default;
	~Application();

	bool init(const AppOptions& options);
	int run();

private:
	enum class CameraMode { Free, Orbit, Cinematic };

	bool initWindow();
	void printHelp() const;

	// GLFW callbacks (forwarded to the instance through the window user pointer)
	static void onKey(GLFWwindow* w, int key, int scancode, int action, int mods);
	static void onScroll(GLFWwindow* w, double dx, double dy);
	static void onFramebufferSize(GLFWwindow* w, int width, int height);
	void handleKey(int key, int action);

	void processInput(float dt);
	void update(float dt);
	void render();
	void updateTitle(float dt);

	CameraState currentCamera() const;
	void setCameraMode(CameraMode mode);

	AppOptions mOptions;
	GLFWwindow* mWindow = nullptr;
	int mFbWidth = 0, mFbHeight = 0;

	// Cameras
	CameraMode mCameraMode = CameraMode::Free;
	FPSCamera mFreeCamera{ glm::vec3(0.0f, 2.5f, 14.0f) };
	OrbitCamera mOrbitCamera;
	float mOrbitYaw = 30.0f, mOrbitPitch = 20.0f;
	float mTourTime = 0.0f;
	CameraState mScriptCamera;
	bool mFirstMouse = true;
	double mLastMouseX = 0.0, mLastMouseY = 0.0;

	// World
	Scene mScene;
	Sky mSky;
	Fireflies mFireflies;
	DemoScript mScript;
	RenderSettings mSettings;
	float mAnimTime = 0.0f;
	float mScriptTime = 0.0f;

	// Rendering
	ShaderProgram mLightingShader, mDepthShader;
	ShadowMap mShadowMap{ 2048 };
	PostProcessor mPost;
	VideoRecorder mRecorder;
};

#endif // APPLICATION_H
