#ifndef RENDER_SETTINGS_H
#define RENDER_SETTINGS_H

struct RenderSettings
{
	bool shadows = true;
	bool bloom = true;
	bool fog = true;
	bool fireflies = true;
	bool wireframe = false;
	bool showShadowMap = false;
	bool flashlight = false;

	float timeOfDay = 18.0f;
	float timeSpeed = 0.05f;
	bool timePaused = false;

	bool animationPaused = false;

	float exposureBias = 1.0f;
	float fade = 1.0f;
};

#endif
