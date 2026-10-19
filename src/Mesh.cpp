#include "Mesh.h"

#include <cstdio>
#include <fstream>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace
{
	struct FaceIndex { int v = 0, vt = 0, vn = 0; };

	FaceIndex parseFaceToken(const std::string& token)
	{
		FaceIndex idx;
		std::string parts[3];
		int p = 0;
		for (char c : token)
		{
			if (c == '/') { if (++p > 2) break; }
			else parts[p] += c;
		}
		if (!parts[0].empty()) idx.v = std::atoi(parts[0].c_str());
		if (!parts[1].empty()) idx.vt = std::atoi(parts[1].c_str());
		if (!parts[2].empty()) idx.vn = std::atoi(parts[2].c_str());
		return idx;
	}

	int resolve(int index, size_t count)
	{
		if (index > 0) return index - 1;
		if (index < 0) return static_cast<int>(count) + index;
		return -1;
	}
}

Mesh::Mesh()
	: mLoaded(false), mVertexCount(0), mVBO(0), mVAO(0), mMin(0.0f), mMax(0.0f)
{
}

Mesh::~Mesh()
{
	if (mVAO) glDeleteVertexArrays(1, &mVAO);
	if (mVBO) glDeleteBuffers(1, &mVBO);
}

bool Mesh::loadOBJ(const std::string& filename)
{
	std::ifstream fin(filename);
	if (!fin)
	{
		std::cerr << "Cannot open " << filename << std::endl;
		return false;
	}

	std::vector<glm::vec3> positions, normals;
	std::vector<glm::vec2> uvs;
	mVertices.clear();

	std::string line;
	while (std::getline(fin, line))
	{
		std::istringstream ss(line);
		std::string cmd;
		ss >> cmd;

		if (cmd == "v")
		{
			glm::vec3 p(0.0f);
			ss >> p.x >> p.y >> p.z;
			positions.push_back(p);
		}
		else if (cmd == "vt")
		{
			glm::vec2 uv(0.0f);
			ss >> uv.x >> uv.y;
			uvs.push_back(uv);
		}
		else if (cmd == "vn")
		{
			glm::vec3 n(0.0f, 1.0f, 0.0f);
			ss >> n.x >> n.y >> n.z;
			normals.push_back(glm::length(n) > 0.0f ? glm::normalize(n) : n);
		}
		else if (cmd == "f")
		{
			std::vector<Vertex> polygon;
			std::vector<bool> hasNormal;
			std::string token;
			while (ss >> token)
			{
				FaceIndex fi = parseFaceToken(token);
				Vertex v{};
				int iv = resolve(fi.v, positions.size());
				int it = resolve(fi.vt, uvs.size());
				int in = resolve(fi.vn, normals.size());
				if (iv < 0 || iv >= (int)positions.size()) continue;
				v.position = positions[iv];
				v.texCoords = (it >= 0 && it < (int)uvs.size()) ? uvs[it] : glm::vec2(0.0f);
				bool okNormal = (in >= 0 && in < (int)normals.size());
				v.normal = okNormal ? normals[in] : glm::vec3(0.0f);
				polygon.push_back(v);
				hasNormal.push_back(okNormal);
			}

			for (size_t i = 1; i + 1 < polygon.size(); ++i)
			{
				Vertex tri[3] = { polygon[0], polygon[i], polygon[i + 1] };
				if (!hasNormal[0] || !hasNormal[i] || !hasNormal[i + 1])
				{
					glm::vec3 n = glm::cross(tri[1].position - tri[0].position, tri[2].position - tri[0].position);
					n = glm::length(n) > 0.0f ? glm::normalize(n) : glm::vec3(0, 1, 0);
					for (Vertex& v : tri) v.normal = n;
				}
				mVertices.insert(mVertices.end(), tri, tri + 3);
			}
		}
	}

	if (mVertices.empty())
	{
		std::cerr << "OBJ file " << filename << " contains no faces" << std::endl;
		return false;
	}

	initBuffers();
	std::cout << "Loaded " << filename << " (" << triangleCount() << " triangles)" << std::endl;
	return (mLoaded = true);
}

void Mesh::createPlane(float size, float uvRepeat)
{
	const float h = size * 0.5f;
	const glm::vec3 n(0.0f, 1.0f, 0.0f);
	Vertex a{ {-h, 0, -h}, n, {0, uvRepeat} };
	Vertex b{ {-h, 0,  h}, n, {0, 0} };
	Vertex c{ { h, 0,  h}, n, {uvRepeat, 0} };
	Vertex d{ { h, 0, -h}, n, {uvRepeat, uvRepeat} };
	mVertices = { a, b, c, a, c, d };
	initBuffers();
	mLoaded = true;
}

void Mesh::initBuffers()
{
	mMin = mMax = mVertices[0].position;
	for (const Vertex& v : mVertices)
	{
		mMin = glm::min(mMin, v.position);
		mMax = glm::max(mMax, v.position);
	}

	if (!mVAO) glGenVertexArrays(1, &mVAO);
	if (!mVBO) glGenBuffers(1, &mVBO);

	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex), mVertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texCoords));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	mVertexCount = mVertices.size();
	mVertices.clear();
	mVertices.shrink_to_fit();
}

void Mesh::draw() const
{
	if (!mLoaded) return;
	glBindVertexArray(mVAO);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)mVertexCount);
	glBindVertexArray(0);
}
