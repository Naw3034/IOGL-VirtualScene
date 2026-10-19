//-----------------------------------------------------------------------------
// Scripted presentation: the camera tour ("virtual visit") followed by a
// technical demonstration where effects are switched on and off.
// Used by the cinematic camera mode (key C) and by --demo / --record.
//-----------------------------------------------------------------------------
#ifndef DEMO_SCRIPT_H
#define DEMO_SCRIPT_H

#include <string>
#include <vector>
#include "CameraPath.h"
#include "RenderSettings.h"

class DemoScript
{
public:
	DemoScript();

	// Camera tour only (loops), for the interactive cinematic mode
	float tourDuration() const { return mTour.duration(); }
	CameraState tourCamera(float t, const glm::vec3& foxPos) const;

	// Full presentation: camera + effects + time of day
	float duration() const { return 100.0f; }
	void apply(float t, RenderSettings& settings, CameraState& camera, const glm::vec3& foxPos) const;

	// Subtitles of the video (exported as .srt by the recorder)
	struct Caption { float start, end; std::string text; };
	const std::vector<Caption>& captions() const { return mCaptions; }
	bool writeSubtitles(const std::string& file) const;

private:
	CameraState orbit(float t) const;

	CameraPath mTour;
	std::vector<Caption> mCaptions;
};

#endif // DEMO_SCRIPT_H
