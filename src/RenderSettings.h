//-----------------------------------------------------------------------------
// Everything the user (or the demo script) can toggle at runtime
//-----------------------------------------------------------------------------
#ifndef RENDER_SETTINGS_H
#define RENDER_SETTINGS_H

struct RenderSettings
{
	// Effects
	bool shadows = true;
	bool bloom = true;
	bool fog = true;
	bool fireflies = true;
	bool wireframe = false;
	bool showShadowMap = false;	// debug view of the depth map
	bool flashlight = false;

	// Time of day (hours, 0..24) and how fast it runs (hours per second)
	float timeOfDay = 18.0f;
	float timeSpeed = 0.05f;
	bool timePaused = false;

	// Animations
	bool animationPaused = false;

	// Post-processing
	float exposureBias = 1.0f;
	float fade = 1.0f;			// 0 = black screen, 1 = visible (video fades)
};

#endif // RENDER_SETTINGS_H
