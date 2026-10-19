//-----------------------------------------------------------------------------
// Camera path made of keyframes (time, position, target) interpolated with
// Catmull-Rom splines: the camera passes exactly through every keyframe
// with a smooth (C1 continuous) trajectory.
//-----------------------------------------------------------------------------
#ifndef CAMERA_PATH_H
#define CAMERA_PATH_H

#include <vector>
#include "glm/glm.hpp"

struct CameraState
{
	glm::vec3 position{ 0.0f };
	glm::vec3 target{ 0.0f, 0.0f, -1.0f };
	float fov = 45.0f;
};

class CameraPath
{
public:
	struct Key
	{
		float time;
		glm::vec3 position;
		glm::vec3 target;
	};

	void addKey(float time, const glm::vec3& position, const glm::vec3& target);
	CameraState evaluate(float time) const;
	float duration() const { return mKeys.empty() ? 0.0f : mKeys.back().time; }

private:
	std::vector<Key> mKeys;
};

#endif // CAMERA_PATH_H
