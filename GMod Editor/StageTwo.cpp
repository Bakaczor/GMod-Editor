#include "StageTwo.h"

using namespace app;

std::vector<gmod::vector3<float>> StageTwo::GeneratePath(const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection) const {
	// create base plane
	// for every surface, find intersection with base (may need to use specific parameters for each surface)
	// combine into one intersection, using winding (intersection has uv-s so it is possible to calculate normals) - hard
	// when found, calculate normals and get outer rim (distance of radius)
	// need to check and solve loops if they happen - harder
	// that's one part of the path
	// the second one is crated similarly to before
	// you just need to aproach each xVal from two sides if there is the same xVal in the path (two times precisely) - easy
	// these will be the turning points

	return std::vector<gmod::vector3<float>>();
}