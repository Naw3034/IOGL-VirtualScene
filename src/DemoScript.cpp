#include "DemoScript.h"

#include <cmath>
#include <cstdio>
#include <fstream>

namespace
{
	float smooth(float e0, float e1, float x)
	{
		float t = (x - e0) / (e1 - e0);
		t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
		return t * t * (3.0f - 2.0f * t);
	}
	bool between(float t, float a, float b) { return t >= a && t < b; }

	const float TOUR_END = 60.0f;
	const glm::vec3 ORBIT_TARGET(0.0f, 1.8f, 0.0f);
}

DemoScript::DemoScript()
{
	mTour.addKey(0.0f,  { 18.0f, 13.0f, 25.0f }, { 0.0f, 1.0f, 0.0f });
	mTour.addKey(5.0f,  { 10.0f, 8.0f, 17.0f },  { 0.0f, 1.5f, 0.0f });
	mTour.addKey(10.0f, { 3.0f, 3.2f, 9.5f },    { 0.0f, 2.2f, 0.0f });
	mTour.addKey(15.0f, { -1.0f, 2.6f, 6.5f },   { 0.0f, 2.4f, 0.0f });
	mTour.addKey(20.0f, { -6.0f, 2.3f, 3.5f },   { -1.0f, 1.6f, 0.0f });
	mTour.addKey(25.0f, { -3.5f, 1.7f, -1.5f },  { -6.5f, 0.7f, -5.5f });
	mTour.addKey(30.0f, { -4.5f, 1.6f, -3.2f },  { -8.2f, 0.9f, -1.9f });
	mTour.addKey(35.0f, { 2.2f, 1.1f, 0.8f },    { 5.0f, 0.3f, -3.4f });
	mTour.addKey(40.0f, { 3.8f, 0.9f, -0.4f },   { 5.0f, 0.3f, -3.7f });
	mTour.addKey(44.0f, { 8.5f, 2.0f, 0.5f },    { 7.6f, 3.4f, 7.6f });
	mTour.addKey(48.0f, { 11.0f, 1.6f, -3.5f },  { 7.0f, 0.5f, 0.0f });
	mTour.addKey(52.0f, { 11.0f, 1.5f, 3.5f },   { 6.0f, 0.5f, 3.0f });
	mTour.addKey(56.0f, { 3.0f, 7.0f, 14.0f },   { 0.0f, 1.5f, 0.0f });
	CameraState o = orbit(TOUR_END);
	mTour.addKey(TOUR_END, o.position, o.target);

	mCaptions = {
		{ 0.5f,  5.0f,  "IOGL - Scène virtuelle : « La petite place »" },
		{ 5.5f,  10.0f, "Chargement de modèles OBJ : robot, tonneaux, caisses, quilles, lapin…" },
		{ 10.5f, 15.0f, "Éclairage Blinn-Phong : soleil/lune + 8 lumières ponctuelles + spot" },
		{ 15.5f, 20.5f, "Cycle jour / nuit : ciel procédural, coucher de soleil, étoiles" },
		{ 21.0f, 25.5f, "Matériaux : texture diffuse + specular map (cerclages métalliques)" },
		{ 26.0f, 33.0f, "Lampe torche : spotlight attaché à la caméra" },
		{ 34.0f, 41.0f, "Animation : partie de bowling automatique" },
		{ 42.0f, 46.5f, "Lanternes (modèle glTF CC0 converti) : texture émissive + bloom HDR" },
		{ 47.0f, 54.0f, "Le renard (modèle glTF) fait sa ronde - caméra de suivi" },
		{ 54.5f, 59.5f, "Lucioles : système de particules additif" },
		{ 60.0f, 62.5f, "Démonstration technique (contrôles clavier)" },
		{ 62.5f, 65.8f, "F1 : mode fil de fer" },
		{ 66.0f, 69.3f, "Touche 2 : bloom désactivé" },
		{ 69.5f, 72.8f, "Touche 3 : brouillard désactivé" },
		{ 73.0f, 77.0f, "Touche F : lampe torche" },
		{ 77.5f, 87.0f, "Cycle jour / nuit accéléré (flèches ← → et N)" },
		{ 88.5f, 92.3f, "Touche 1 : ombres désactivées (shadow mapping + PCF)" },
		{ 92.5f, 96.0f, "Touche 5 : visualisation de la shadow map" },
		{ 96.5f, 99.8f, "Merci !" },
	};
}

CameraState DemoScript::orbit(float t) const
{
	float k = t - TOUR_END;
	float a = 2.4f + 0.16f * k;
	float grow = smooth(77.0f, 87.0f, t);
	float radius = 8.5f + 5.5f * grow;
	float height = 3.2f + 3.8f * grow + 5.0f * smooth(96.0f, 100.0f, t);
	CameraState s;
	s.position = ORBIT_TARGET + glm::vec3(radius * std::cos(a), height, radius * std::sin(a));
	s.target = ORBIT_TARGET;
	return s;
}

CameraState DemoScript::tourCamera(float t, const glm::vec3& foxPos) const
{
	CameraState s = mTour.evaluate(t);
	float w = smooth(45.5f, 47.5f, t) * (1.0f - smooth(52.5f, 54.5f, t));
	s.target = glm::mix(s.target, foxPos + glm::vec3(0.0f, 0.4f, 0.0f), w);
	return s;
}

void DemoScript::apply(float t, RenderSettings& s, CameraState& camera, const glm::vec3& foxPos) const
{
	camera = (t < TOUR_END) ? tourCamera(t, foxPos) : orbit(t);

	if (t < TOUR_END)
		s.timeOfDay = 16.3f + 0.11f * t;
	else if (t < 77.0f)
		s.timeOfDay = 16.3f + 0.11f * TOUR_END;
	else
	{
		float night = 16.3f + 0.11f * TOUR_END;
		s.timeOfDay = night + (33.5f - night) * smooth(77.0f, 87.0f, t) + 0.05f * std::fmax(0.0f, t - 87.0f);
	}
	s.timeOfDay = std::fmod(s.timeOfDay, 24.0f);
	s.timePaused = true;

	s.flashlight = between(t, 26.5f, 37.0f) || between(t, 73.0f, 77.0f);
	s.wireframe = between(t, 62.5f, 65.8f);
	s.bloom = !between(t, 66.0f, 69.3f);
	s.fog = !between(t, 69.5f, 72.8f);
	s.shadows = !between(t, 88.5f, 92.3f);
	s.showShadowMap = between(t, 92.5f, 96.0f);
	s.fireflies = true;

	s.fade = smooth(0.0f, 1.5f, t) * (1.0f - smooth(98.5f, 100.0f, t));
}

bool DemoScript::writeSubtitles(const std::string& file) const
{
	std::ofstream out(file);
	if (!out) return false;
	auto stamp = [](float sec) {
		int ms = (int)std::lround(sec * 1000.0f);
		char buf[32];
		std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d,%03d", ms / 3600000, (ms / 60000) % 60, (ms / 1000) % 60, ms % 1000);
		return std::string(buf);
	};
	int n = 1;
	for (const Caption& c : mCaptions)
		out << n++ << "\n" << stamp(c.start) << " --> " << stamp(c.end) << "\n" << c.text << "\n\n";
	return true;
}
