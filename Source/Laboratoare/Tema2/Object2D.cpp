#include "Object2D.h"

#include <Core/Engine.h>

#define ANGLE 3600.0
#define PI 3.14

Mesh* Object2D::CreateSquare(std::string name, glm::vec3 leftBottomCorner, float length, glm::vec3 color, bool fill)
{
	glm::vec3 corner = leftBottomCorner;

	std::vector<VertexFormat> vertices =
	{
		VertexFormat(corner, color),
		VertexFormat(corner + glm::vec3(length, 0, 0), color),
		VertexFormat(corner + glm::vec3(length, length, 0), color),
		VertexFormat(corner + glm::vec3(0, length, 0), color)
	};

	Mesh* mesh = new Mesh(name);
	std::vector<unsigned short> indices = { 0, 1, 2, 3 };

	if (!fill) {
		mesh->SetDrawMode(GL_LINE_LOOP);
	}
	else {
		// draw 2 triangles. Add the remaining 2 indices
		indices.push_back(0);
		indices.push_back(2);
	}

	mesh->InitFromData(vertices, indices);
	return mesh;
}

Mesh* Object2D::CreateRectangle(std::string name, glm::vec3 leftBottomCorner, float height, float length, glm::vec3 color, bool fill)
{
	glm::vec3 corner = leftBottomCorner;

	std::vector<VertexFormat> vertices =
	{
		VertexFormat(corner, color),
		VertexFormat(corner + glm::vec3(height, 0, 0), color),
		VertexFormat(corner + glm::vec3(height, length, 0), color),
		VertexFormat(corner + glm::vec3(0, length, 0), color)
	};

	Mesh* mesh = new Mesh(name);
	std::vector<unsigned short> indices = { 0, 1, 2, 3 };

	if (!fill) {
		mesh->SetDrawMode(GL_LINE_LOOP);
	}
	else {
		// draw 2 triangles. Add the remaining 2 indices
		indices.push_back(0);
		indices.push_back(2);
	}

	mesh->InitFromData(vertices, indices);
	return mesh;
}

Mesh* Object2D::CreateSemicircle(std::string name, glm::vec3 leftBottomCorner, float length, glm::vec3 color, bool fill)
{
	glm::vec3 corner = leftBottomCorner;

	std::vector<VertexFormat> vertices;
	vertices.push_back(VertexFormat(corner, color));
	for (float i = 0; i < ANGLE / 4; i++)
		vertices.push_back(VertexFormat(corner + glm::vec3(cos(i * 2 / ANGLE * 2 * PI) * length, sin(i * 2 / ANGLE * 2 * PI) * length, 0), color));
	vertices.push_back(VertexFormat(corner + glm::vec3(0, length, 0), color));
	
	Mesh* mesh = new Mesh(name);
	std::vector<unsigned short> indices = { 0 };
	for (int i = 1; i <= ANGLE / 4; i++)
	{
		indices.push_back(i);
		// indices.push_back(i);
	}
	
	if (!fill) {
		mesh->SetDrawMode(GL_LINE_LOOP);
	}

	mesh->InitFromData(vertices, indices);
	return mesh;
}

Mesh* Object2D::CreateCircle(std::string name, glm::vec3 leftBottomCorner, float length, glm::vec3 color, bool fill)
{
	glm::vec3 corner = leftBottomCorner;

	std::vector<VertexFormat> vertices;

	vertices.push_back(VertexFormat(corner + glm::vec3(length, 0, 0), color));
	for (int i = 0; i < 3600; i++)
		vertices.push_back(VertexFormat(corner + glm::vec3(cos(i / ANGLE * 2 * PI) * length, sin(i / ANGLE * 2 * PI) * length, 0), color));

	Mesh* mesh = new Mesh(name);

	std::vector<unsigned short> indices;
	for (int i = 0; i <= 3600 - 1; i++)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	if (!fill)
		mesh->SetDrawMode(GL_LINE_LOOP);
	else {
		// draw 2 triangles. Add the remaining 2 indices
		indices.push_back(0);
		indices.push_back(2);
	}

	mesh->InitFromData(vertices, indices);
	return mesh;
}

Mesh* Object2D::CreateLine(std::string name, glm::vec3 leftBottomCorner, float length, glm::vec3 color, bool fill)
{
	glm::vec3 corner = leftBottomCorner;

	std::vector<VertexFormat> vertices =
	{
		VertexFormat(corner, color),
		VertexFormat(corner + glm::vec3(length, 0, 0), color),
	};

	Mesh* mesh = new Mesh(name);
	std::vector<unsigned short> indices = { 0, 1 };

	if (!fill) {
		mesh->SetDrawMode(GL_LINE_LOOP);
	}

	mesh->InitFromData(vertices, indices);
	return mesh;
}

Mesh* Object2D::CreateTriangle(std::string name, glm::vec3 leftBottomCorner, float length, glm::vec3 color, bool fill)
{
	glm::vec3 corner = leftBottomCorner;

	std::vector<VertexFormat> vertices =
	{
		VertexFormat(corner, color),
		VertexFormat(corner + glm::vec3(length, 0, 0), color),
		VertexFormat(corner + glm::vec3(length, length, 0), color),
		VertexFormat(corner + glm::vec3(0, length, 0), color)
	};

	Mesh* mesh = new Mesh(name);
	std::vector<unsigned short> indices = { 0, 1, 2 };

	if (!fill) {
		mesh->SetDrawMode(GL_LINE_LOOP);
	}

	mesh->InitFromData(vertices, indices);
	return mesh;
}

