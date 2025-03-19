#include "Cube.h"

unsigned short Cube::m_globalCubeNum = 1;

Cube::Cube() {
	m_type = "cube";
	std::ostringstream os;
	os << "cube_" << m_globalCubeNum;
	name = os.str();
	m_globalCubeNum += 1;
	RecalculateGeometry();
}

void Cube::RecalculateGeometry() {
	m_vertices = {
		{ gmod::vector3<double>(-0.5f, -0.5f,  0.5f) },
		{ gmod::vector3<double>(0.5f, -0.5f,  0.5f) },
		{ gmod::vector3<double>(0.5f, -0.5f, -0.5f) },
		{ gmod::vector3<double>(-0.5f, -0.5f, -0.5f) },
		{ gmod::vector3<double>(-0.5f,  0.5f,  0.5f) },
		{ gmod::vector3<double>(0.5f,  0.5f,  0.5f) },
		{ gmod::vector3<double>(0.5f,  0.5f, -0.5f) },
		{ gmod::vector3<double>(-0.5f,  0.5f, -0.5f) }
	};
	m_edges = {
		{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
		{ 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
		{ 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 }
	};
}