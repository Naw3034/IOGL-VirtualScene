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

	float tourDuration() const { return mTour.duration(); }
	CameraState tourCamera(float t, const glm::vec3& foxPos) const;

	float duration() const { return 100.0f; }
	void apply(float t, RenderSettings& settings, CameraState& camera, const glm::vec3& foxPos) const;

	struct Caption { float start, end; std::string text; };
	const std::vector<Caption>& captions() const { return mCaptions; }
	bool writeSubtitles(const std::string& file) const;

private:
	CameraState orbit(float t) const;

	CameraPath mTour;
	std::vector<Caption> mCaptions;
};

#endif
