//-----------------------------------------------------------------------------
// Sky: full-screen triangle, each pixel computes its own view ray
//-----------------------------------------------------------------------------
#version 330 core

out vec2 uv;
out vec2 ndc;

void main()
{
	vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);	// (0,0) (2,0) (0,2)
	uv = p;
	ndc = p * 2.0 - 1.0;
	gl_Position = vec4(ndc, 0.0, 1.0);
}
