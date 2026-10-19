//-----------------------------------------------------------------------------
// Catmull-Rom camera path
//-----------------------------------------------------------------------------
#include "CameraPath.h"

#include <algorithm>

namespace
{
	glm::vec3 catmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t)
	{
		float t2 = t * t, t3 = t2 * t;
		return 0.5f * ((2.0f * p1) + (-p0 + p2) * t + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2
			+ (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
	}
}

void CameraPath::addKey(float time, const glm::vec3& position, const glm::vec3& target)
{
	mKeys.push_back({ time, position, target });
	std::sort(mKeys.begin(), mKeys.end(), [](const Key& a, const Key& b) { return a.time < b.time; });
}

CameraState CameraPath::evaluate(float time) const
{
	CameraState s;
	if (mKeys.empty()) return s;
	if (time <= mKeys.front().time) { s.position = mKeys.front().position; s.target = mKeys.front().target; return s; }
	if (time >= mKeys.back().time) { s.position = mKeys.back().position; s.target = mKeys.back().target; return s; }

	size_t i = 0;
	while (i + 1 < mKeys.size() && mKeys[i + 1].time < time) ++i;

	const Key& k1 = mKeys[i];
	const Key& k2 = mKeys[i + 1];
	const Key& k0 = mKeys[i > 0 ? i - 1 : i];
	const Key& k3 = mKeys[i + 2 < mKeys.size() ? i + 2 : i + 1];

	float u = (time - k1.time) / (k2.time - k1.time);
	s.position = catmullRom(k0.position, k1.position, k2.position, k3.position, u);
	s.target = catmullRom(k0.target, k1.target, k2.target, k3.target, u);
	return s;
}
