//-----------------------------------------------------------------------------
// Application: window, input, camera modes and rendering pipeline
//-----------------------------------------------------------------------------
#include "Application.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "glm/gtc/matrix_transform.hpp"
#include "Paths.h"

namespace
{
	const char* APP_TITLE = "IOGL - La petite place";
	const float MOVE_SPEED = 5.0f;			// units per second
	const float MOUSE_SENSITIVITY = 0.1f;
	const double ZOOM_SENSITIVITY = -3.0;
	const char* MODE_NAMES[] = { "Libre (FPS)", "Orbite", "Visite cinematique" };
}

Application::~Application()
{
	mRecorder.close();
	if (mWindow) glfwTerminate();
}

//-----------------------------------------------------------------------------
// Initialisation
//-----------------------------------------------------------------------------
bool Application::init(const AppOptions& options)
{
	mOptions = options;
	if (!initWindow()) return false;

	bool ok = mLightingShader.loadShaders(resourcePath("shaders/scene.vert").c_str(), resourcePath("shaders/scene.frag").c_str());
	ok = mDepthShader.loadShaders(resourcePath("shaders/shadow_depth.vert").c_str(), resourcePath("shaders/shadow_depth.frag").c_str()) && ok;
	ok = mShadowMap.init() && ok;
	ok = mPost.init(mFbWidth, mFbHeight) && ok;
	ok = mSky.init() && ok;
	ok = mFireflies.init() && ok;
	ok = mScene.load() && ok;
	if (!ok)
	{
		std::cerr << "Initialisation failed (check that the 'shaders' and 'assets' folders are reachable)" << std::endl;
		return false;
	}

	mFreeCamera.lookAt(glm::vec3(0.0f, 1.8f, 0.0f));
	mOrbitCamera.setLookAt(glm::vec3(0.0f, 1.8f, 0.0f));
	mOrbitCamera.setRadius(12.0f);
	mOrbitCamera.rotate(mOrbitYaw, mOrbitPitch);

	if (!mOptions.recordFile.empty())
	{
		if (!mRecorder.open(mOptions.recordFile, mFbWidth, mFbHeight, mOptions.fps)) return false;
		std::string srt = mOptions.recordFile.substr(0, mOptions.recordFile.find_last_of('.')) + ".srt";
		mScript.writeSubtitles(srt);
	}
	if (mOptions.demo || mRecorder.isOpen())
		mScriptTime = mOptions.startTime;
	else
		printHelp();

	return true;
}

bool Application::initWindow()
{
	if (!glfwInit())
	{
		std::cerr << "GLFW initialization failed" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 0);
	const bool recording = !mOptions.recordFile.empty();
	if (recording) glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWmonitor* monitor = nullptr;
	int w = mOptions.width, h = mOptions.height;
	if (mOptions.fullscreen && !recording)
	{
		monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		w = mode->width;
		h = mode->height;
	}

	mWindow = glfwCreateWindow(w, h, APP_TITLE, monitor, nullptr);
	if (!mWindow)
	{
		std::cerr << "Failed to create an OpenGL 3.3 window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(mWindow);
	glfwSwapInterval(recording ? 0 : 1);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return false;
	}
	glGetError();	// glewInit can leave a harmless GL_INVALID_ENUM

	std::cout << "OpenGL " << glGetString(GL_VERSION) << " - " << glGetString(GL_RENDERER) << std::endl;

	glfwSetWindowUserPointer(mWindow, this);
	glfwSetKeyCallback(mWindow, onKey);
	glfwSetScrollCallback(mWindow, onScroll);
	glfwSetFramebufferSizeCallback(mWindow, onFramebufferSize);

	if (!recording && !mOptions.demo)
		glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwGetFramebufferSize(mWindow, &mFbWidth, &mFbHeight);
	glViewport(0, 0, mFbWidth, mFbHeight);
	glEnable(GL_DEPTH_TEST);
	return true;
}

void Application::printHelp() const
{
	std::cout <<
		"\n=== Controles ==========================================\n"
		"  Souris            : regarder        Molette : zoom (FOV) / distance (orbite)\n"
		"  W A S D (Z Q S D) : se deplacer     Espace / Shift : monter / descendre\n"
		"  C                 : mode camera (libre -> orbite -> visite cinematique)\n"
		"  F                 : lampe torche    F1 : fil de fer\n"
		"  1 ombres | 2 bloom | 3 brouillard | 4 lucioles | 5 shadow map\n"
		"  Fleches G/D       : heure -/+ 1h    Fleches H/B : vitesse du temps\n"
		"  N                 : pause du cycle jour/nuit\n"
		"  P                 : pause des animations\n"
		"  H                 : cette aide      Echap : quitter\n"
		"========================================================\n" << std::endl;
}

//-----------------------------------------------------------------------------
// Input
//-----------------------------------------------------------------------------
void Application::onKey(GLFWwindow* w, int key, int, int action, int)
{
	static_cast<Application*>(glfwGetWindowUserPointer(w))->handleKey(key, action);
}

void Application::onScroll(GLFWwindow* w, double, double dy)
{
	Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
	if (app->mCameraMode == CameraMode::Orbit)
	{
		app->mOrbitCamera.setRadius(app->mOrbitCamera.getRadius() - (float)dy);
		app->mOrbitCamera.rotate(app->mOrbitYaw, app->mOrbitPitch);
	}
	else
	{
		double fov = glm::clamp(app->mFreeCamera.getFOV() + dy * ZOOM_SENSITIVITY, 5.0, 100.0);
		app->mFreeCamera.setFOV((float)fov);
	}
}

void Application::onFramebufferSize(GLFWwindow* w, int width, int height)
{
	Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
	app->mFbWidth = width;
	app->mFbHeight = height;
	app->mPost.resize(width, height);
}

void Application::handleKey(int key, int action)
{
	if (action != GLFW_PRESS) return;
	RenderSettings& s = mSettings;

	switch (key)
	{
	case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(mWindow, GL_TRUE); break;
	case GLFW_KEY_F1: s.wireframe = !s.wireframe; break;
	case GLFW_KEY_F: s.flashlight = !s.flashlight; break;
	case GLFW_KEY_1: s.shadows = !s.shadows; break;
	case GLFW_KEY_2: s.bloom = !s.bloom; break;
	case GLFW_KEY_3: s.fog = !s.fog; break;
	case GLFW_KEY_4: s.fireflies = !s.fireflies; break;
	case GLFW_KEY_5: s.showShadowMap = !s.showShadowMap; break;
	case GLFW_KEY_N: s.timePaused = !s.timePaused; break;
	case GLFW_KEY_P: s.animationPaused = !s.animationPaused; break;
	case GLFW_KEY_H: printHelp(); break;
	case GLFW_KEY_LEFT: s.timeOfDay = std::fmod(s.timeOfDay + 23.0f, 24.0f); break;
	case GLFW_KEY_RIGHT: s.timeOfDay = std::fmod(s.timeOfDay + 1.0f, 24.0f); break;
	case GLFW_KEY_UP: s.timeSpeed = glm::min(s.timeSpeed * 2.0f, 3.2f); break;
	case GLFW_KEY_DOWN: s.timeSpeed = glm::max(s.timeSpeed * 0.5f, 0.0125f); break;
	case GLFW_KEY_C:
		setCameraMode(mCameraMode == CameraMode::Free ? CameraMode::Orbit
			: mCameraMode == CameraMode::Orbit ? CameraMode::Cinematic : CameraMode::Free);
		break;
	default: break;
	}
}

void Application::setCameraMode(CameraMode mode)
{
	// Leaving the cinematic/orbit view: the free camera continues from there
	CameraState current = currentCamera();
	if (mode == CameraMode::Free)
	{
		mFreeCamera.setPosition(current.position);
		mFreeCamera.lookAt(current.target);
	}
	else if (mode == CameraMode::Cinematic)
		mTourTime = 0.0f;
	mCameraMode = mode;
	std::cout << "Camera: " << MODE_NAMES[(int)mode] << std::endl;
}

void Application::processInput(float dt)
{
	double mx, my;
	glfwGetCursorPos(mWindow, &mx, &my);
	if (mFirstMouse) { mLastMouseX = mx; mLastMouseY = my; mFirstMouse = false; }
	float dx = (float)(mLastMouseX - mx), dy = (float)(mLastMouseY - my);
	mLastMouseX = mx;
	mLastMouseY = my;

	if (mCameraMode == CameraMode::Free)
	{
		mFreeCamera.rotate(dx * MOUSE_SENSITIVITY, dy * MOUSE_SENSITIVITY);

		glm::vec3 move(0.0f);
		if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS) move += mFreeCamera.getLook();
		if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS) move -= mFreeCamera.getLook();
		if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS) move += mFreeCamera.getRight();
		if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS) move -= mFreeCamera.getRight();
		if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS) move += glm::vec3(0, 1, 0);
		if (glfwGetKey(mWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) move -= glm::vec3(0, 1, 0);
		if (glm::length(move) > 0.0f)
		{
			mFreeCamera.move(glm::normalize(move) * MOVE_SPEED * dt);
			// keep the eye above the ground
			glm::vec3 p = mFreeCamera.getPosition();
			if (p.y < 0.3f) mFreeCamera.move(glm::vec3(0.0f, 0.3f - p.y, 0.0f));
		}
	}
	else if (mCameraMode == CameraMode::Orbit)
	{
		mOrbitYaw -= dx * MOUSE_SENSITIVITY;
		mOrbitPitch = glm::clamp(mOrbitPitch - dy * MOUSE_SENSITIVITY, 2.0f, 85.0f);
		mOrbitCamera.rotate(mOrbitYaw, mOrbitPitch);
	}
}

//-----------------------------------------------------------------------------
// Simulation
//-----------------------------------------------------------------------------
void Application::update(float dt)
{
	const bool scripted = mOptions.demo || mRecorder.isOpen();

	if (scripted)
	{
		// The script drives everything: animations are synchronised with it
		mScript.apply(mScriptTime, mSettings, mScriptCamera, mScene.foxPosition());
		mAnimTime = mScriptTime;
	}
	else
	{
		if (!mSettings.animationPaused) mAnimTime += dt;
		if (!mSettings.timePaused)
			mSettings.timeOfDay = std::fmod(mSettings.timeOfDay + mSettings.timeSpeed * dt, 24.0f);
		if (mCameraMode == CameraMode::Cinematic)
			mTourTime = std::fmod(mTourTime + dt, mScript.tourDuration());
	}

	mScene.update(mAnimTime, mSettings);
	mFireflies.update(mAnimTime);
}

CameraState Application::currentCamera() const
{
	CameraState s;
	if (mOptions.demo || mRecorder.isOpen())
		return mScriptCamera;

	switch (mCameraMode)
	{
	case CameraMode::Free:
		s.position = mFreeCamera.getPosition();
		s.target = s.position + mFreeCamera.getLook();
		s.fov = mFreeCamera.getFOV();
		break;
	case CameraMode::Orbit:
		s.position = mOrbitCamera.getPosition();
		s.target = mOrbitCamera.getTarget();
		break;
	case CameraMode::Cinematic:
		s = mScript.tourCamera(mTourTime, mScene.foxPosition());
		break;
	}
	return s;
}

//-----------------------------------------------------------------------------
// Rendering
//-----------------------------------------------------------------------------
void Application::render()
{
	if (mFbWidth <= 0 || mFbHeight <= 0) return;	// minimised

	const Environment& env = mScene.environment();
	CameraState cam = currentCamera();
	glm::vec3 look = glm::normalize(cam.target - cam.position);
	glm::mat4 view = glm::lookAt(cam.position, cam.target, glm::vec3(0, 1, 0));
	glm::mat4 projection = glm::perspective(glm::radians(cam.fov), (float)mFbWidth / (float)mFbHeight, 0.1f, 200.0f);

	// The flashlight is held a bit below and to the right of the eye
	SpotLight& torch = mScene.flashlight();
	glm::vec3 right = glm::normalize(glm::cross(look, glm::vec3(0, 1, 0)));
	torch.position = cam.position - glm::vec3(0.0f, 0.3f, 0.0f) + right * 0.2f;
	torch.direction = look;

	// 1) Shadow pass ------------------------------------------------------------
	mShadowMap.update(mScene.mainLight().direction, glm::vec3(0.0f, 0.0f, 0.0f), 19.0f);
	if (mSettings.shadows)
	{
		mShadowMap.begin();
		mDepthShader.use();
		mDepthShader.setUniform("lightSpaceMatrix", mShadowMap.lightSpaceMatrix());
		mScene.draw(mDepthShader, true);
		mShadowMap.end();
	}

	// 2) HDR pass ---------------------------------------------------------------
	mPost.beginScene();
	mSky.draw(mPost, env, view, projection, mAnimTime);

	if (mSettings.wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	mLightingShader.use();
	mLightingShader.setUniform("view", view);
	mLightingShader.setUniform("projection", projection);
	mLightingShader.setUniform("viewPos", cam.position);
	mLightingShader.setUniform("lightSpaceMatrix", mShadowMap.lightSpaceMatrix());
	mLightingShader.setUniform("shadowsOn", (GLint)(mSettings.shadows ? 1 : 0));
	mLightingShader.setUniformSampler("shadowMap", 3);
	mShadowMap.bindTexture(3);
	mLightingShader.setUniform("fogOn", (GLint)(mSettings.fog ? 1 : 0));
	mLightingShader.setUniform("fogColor", env.fogColor);
	mLightingShader.setUniform("fogDensity", env.fogDensity);
	mScene.applyLights(mLightingShader);
	mScene.draw(mLightingShader, false);

	if (mSettings.fireflies && !mSettings.wireframe)
		mFireflies.draw(view, projection, (float)mFbHeight, env.nightFactor);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// 3) Post-processing --------------------------------------------------------
	PostSettings post;
	post.bloom = mSettings.bloom;
	post.exposure = env.exposure * mSettings.exposureBias;
	post.fade = mSettings.fade;
	post.showShadowMap = mSettings.showShadowMap;
	post.shadowMapTexture = mShadowMap.texture();
	post.time = mAnimTime;
	mPost.endScene(post);
}

void Application::updateTitle(float dt)
{
	static float acc = 0.0f;
	static int frames = 0;
	acc += dt;
	frames++;
	if (acc < 0.25f) return;

	int hours = (int)mSettings.timeOfDay;
	int minutes = (int)((mSettings.timeOfDay - hours) * 60.0f);
	std::ostringstream title;
	title << APP_TITLE << "  |  " << (mOptions.demo ? "Demo" : MODE_NAMES[(int)mCameraMode])
		<< "  |  " << std::setfill('0') << std::setw(2) << hours << "h" << std::setw(2) << minutes
		<< std::setfill(' ') << "  |  " << std::fixed << std::setprecision(1) << frames / acc << " FPS";
	glfwSetWindowTitle(mWindow, title.str().c_str());
	acc = 0.0f;
	frames = 0;
}

//-----------------------------------------------------------------------------
// Main loop
//-----------------------------------------------------------------------------
int Application::run()
{
	if (mRecorder.isOpen())
	{
		// Offline rendering with a fixed time step
		const float dt = 1.0f / (float)mOptions.fps;
		const float end = mOptions.endTime > 0.0f ? mOptions.endTime : mScript.duration();
		const int total = (int)std::ceil((end - mOptions.startTime) * mOptions.fps);
		for (int frame = 0; frame < total && !glfwWindowShouldClose(mWindow); ++frame)
		{
			mScriptTime = mOptions.startTime + frame * dt;
			glfwPollEvents();
			update(dt);
			render();
			mRecorder.captureFrame();
			glfwSwapBuffers(mWindow);
			if (frame % mOptions.fps == 0)
				std::cout << "\rFrame " << frame << " / " << total << std::flush;
		}
		std::cout << std::endl;
		mRecorder.close();
		return 0;
	}

	double last = glfwGetTime();
	while (!glfwWindowShouldClose(mWindow))
	{
		double now = glfwGetTime();
		float dt = (float)glm::min(now - last, 0.1);	// clamp after a hiccup
		last = now;

		glfwPollEvents();
		if (mOptions.demo)
		{
			mScriptTime += dt;
			if (mScriptTime > mScript.duration()) mScriptTime = 0.0f;
		}
		else
			processInput(dt);

		update(dt);
		render();
		glfwSwapBuffers(mWindow);
		updateTitle(dt);
	}
	return 0;
}
