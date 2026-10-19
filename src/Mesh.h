//-----------------------------------------------------------------------------
// Mesh class
// Based on the course "Introduction to Modern OpenGL" mesh class, extended:
//  - OBJ faces with any number of vertices (triangulated as a fan)
//  - all index formats: v, v/vt, v//vn, v/vt/vn, negative (relative) indices
//  - flat normals computed when the file has none
//  - procedural plane generation (used for the ground)
//-----------------------------------------------------------------------------
#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>

#include "GL/glew.h"
#include "glm/glm.hpp"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

class Mesh
{
public:
	Mesh();
	~Mesh();
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	bool loadOBJ(const std::string& filename);
	void createPlane(float size, float uvRepeat);
	void draw() const;

	size_t triangleCount() const { return mVertexCount / 3; }
	const glm::vec3& boundsMin() const { return mMin; }
	const glm::vec3& boundsMax() const { return mMax; }

private:
	void initBuffers();

	bool mLoaded;
	std::vector<Vertex> mVertices;	// CPU copy, released once uploaded to the GPU
	size_t mVertexCount;
	GLuint mVBO, mVAO;
	glm::vec3 mMin, mMax;
};

#endif // MESH_H
