#include "StageOne.h"
#include "Surface.h"
#include "IGeometrical.h"
#include "Debug.h";

using namespace app;

std::vector<gmod::vector3<float>> StageOne::GeneratePath(const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection) const {
	std::vector<std::vector<float>> heightmap = CreateHeightMap(sceneObjects, intersection);

	// calculate boundaries 
	const float xLeft = topLeftCorner.x();
	const float xRight = topLeftCorner.x() + width;

	// with respect to radius
	const float zTop = topLeftCorner.z() - m_radius - 1.0f;
	const float zBottom = topLeftCorner.z() + length + m_radius + 1.0f;

	// calcualte milling heights
	const float heightBottom = baseY + epsilon;
	const float heightTop = heightBottom + (totalHeight - baseY) / 2;

	const float separation = diameter / 1.5f;

	// calculate x values
	std::vector<float> xValues;
	float xCurr = xLeft;
	while (xCurr < xRight) {
		xValues.push_back(xCurr);
		xCurr += separation;
	}
	xValues.push_back(xCurr); // last additional

	// going from left to right
	float moveToRight = separation;

	gmod::vector3<float> currPos(xLeft, baseY, zTop);
	int direction = 1; // top-bottom -> 1, bottom-top -> -1

	const float stepX = width / m_resX;
	const float stepZ = length / m_resZ;

	std::vector<gmod::vector3<float>> topPath;
	std::vector<gmod::vector3<float>> bottomPath;
	for (const float& xVal : xValues) {
		topPath.push_back(gmod::vector3<float>(currPos.x(), heightTop, currPos.z()));
		bottomPath.push_back(gmod::vector3<float>(currPos.x(), heightBottom, currPos.z()));

		bool dir = direction == 1;

		auto nextPos = currPos;
		nextPos.z() = dir ? zBottom : zTop;

		const int x = static_cast<int>((xVal - topLeftCorner.x()) / width * m_resX);
		
		for (int z = dir ? 0 : m_resZ; dir ? z <= m_resZ : z >= 0; z += direction) {
			const float zVal = z * stepZ + topLeftCorner.z();
			const float yVal = CheckInRange(heightmap, x, z) + epsilon;

			topPath.push_back(gmod::vector3<float>(xVal, std::max(yVal, heightTop), zVal));
			bottomPath.push_back(gmod::vector3<float>(xVal, std::max(yVal, heightBottom), zVal));
		}

		topPath.push_back(gmod::vector3<float>(nextPos.x(), heightTop, nextPos.z()));
		bottomPath.push_back(gmod::vector3<float>(nextPos.x(), heightBottom, nextPos.z()));

		currPos = nextPos;
		currPos.x() = currPos.x() + moveToRight;
		direction *= -1;
	}

	topPath = MakeSmooth(topPath);
	bottomPath = MakeSmooth(bottomPath);

	// filter
	std::vector<gmod::vector3<float>> topPathFiltered;
	topPathFiltered.push_back(topPath.front());
	for (int i = 1; i < topPath.size() - 1; i++) {
		float currY = topPath[i].y();

		if (std::abs(topPath[i].z() - zTop) < FZERO || std::abs(topPath[i].z() - zBottom) < FZERO) {
			topPathFiltered.push_back(topPath[i]);
		}

		if (std::abs(topPath[i - 1].y() - currY) < FZERO && std::abs(topPath[i + 1].y() - currY) < FZERO) {
			continue;
		}
		topPathFiltered.push_back(topPath[i]);
	}
	topPathFiltered.push_back(topPath.back());

	std::vector<gmod::vector3<float>> bottomPathFiltered;
	bottomPathFiltered.push_back(bottomPath.front());
	for (int i = 1; i < bottomPath.size() - 1; i++) {
		float currY = bottomPath[i].y();

		if (std::abs(bottomPath[i].z() - zTop) < FZERO || std::abs(bottomPath[i].z() - zBottom) < FZERO) {
			bottomPathFiltered.push_back(bottomPath[i]);
		}
		if (std::abs(bottomPath[i - 1].y() - currY) < FZERO && std::abs(bottomPath[i + 1].y() - currY) < FZERO) {
			continue;
		} 
		bottomPathFiltered.push_back(bottomPath[i]);
	}
	bottomPathFiltered.push_back(bottomPath.back());

	// special points
	gmod::vector3<float> startPoint(0, totalHeight + 1.0f, 0);
	gmod::vector3<float> overMillingStart(xLeft, totalHeight + 1.0f, zTop);

	std::vector<gmod::vector3<float>> path;
	path.reserve(1 + 1 + topPath.size() + 1 + 1 + bottomPath.size() + 1 + 1);

	// combine
	path.push_back(startPoint);

	path.push_back(overMillingStart);
	for (const auto& pos : topPathFiltered) {
		path.push_back(pos);
	}
	const auto& lastTop = topPathFiltered.back();
	path.push_back(gmod::vector3<float>(lastTop.x(), totalHeight + 1.0f, lastTop.z()));

	path.push_back(overMillingStart);
	for (const auto& pos : bottomPathFiltered) {
		path.push_back(pos);
	}
	const auto& lastBottom = bottomPathFiltered.back();
	path.push_back(gmod::vector3<float>(lastBottom.x(), totalHeight + 1.0f, lastBottom.z()));

	path.push_back(startPoint);

	return path;
}

float StageOne::CheckInRange(const std::vector<std::vector<float>>& heightmap, int currX, int currZ) const {
	const int rangeX = m_radius / width * m_resX;
	const int rangeZ = m_radius / length * m_resZ;

	const int startX = std::max(0, currX - rangeX);
	const int startZ = std::max(0, currZ - rangeZ);

	const int endX = std::min(m_resX, currX + rangeX);
	const int endZ = std::min(m_resZ, currZ + rangeZ);

	float y = baseY;
	for (int x = startX; x <= endX; x++) {
		for (int z = startZ; z <= endZ; z++) {
			if (heightmap[x][z] > y) {
				y = heightmap[x][z];
			}
		}
	}

	return y;
}

std::vector<std::vector<float>> StageOne::CreateHeightMap(const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection) const {
	intersection.SetIntersectionParameters(m_interParams);

	// get all surfaces on scene
	std::vector<Intersection::IDIG> sceneSurfaces;

	for (const auto& so : sceneObjects) {
		IGeometrical* g = dynamic_cast<IGeometrical*>(so.get());
		if (g != nullptr) {
			sceneSurfaces.push_back({ so->id, g });
		}
	}

	// sort based on y value
	std::sort(sceneSurfaces.begin(), sceneSurfaces.end(), [](const Intersection::IDIG& a, const Intersection::IDIG& b) {
		return a.s->WorldBounds().max.y() > b.s->WorldBounds().max.y();
	});

	// create "ray"
	const float rayWidth = 0.1f;
	const float rayLenght = totalHeight - baseY;
	const float rayY = rayLenght / 2 + baseY;
	const float stepX = width / m_resX;
	const float stepZ = length / m_resZ;

	Surface::Plane ray = Surface::MakePlane(centre, rayLenght, rayWidth, false, -69);

	// complete heigtmap
	std::vector<std::vector<float>> heightmap(m_resX + 1, std::vector<float>(m_resZ + 1, baseY));
	for (int x = 0; x <= m_resX; x++) {
		const float rayX = x * stepX + topLeftCorner.x();
		for (int z = 0; z <= m_resZ; z++) {
			const float rayZ = z * stepZ + topLeftCorner.z();

			ray.surface->SetTranslation(rayX, rayY, rayZ);

			Intersection::IDIG rayIDIG = { ray.surface->id, dynamic_cast<IGeometrical*>(ray.surface.get()) };
			for (auto& surf : sceneSurfaces) {
				if (!IGeometrical::XYZBoundsIntersect(surf.s->WorldBounds(), rayIDIG.s->WorldBounds())) { continue; }
				unsigned int res = intersection.FindIntersection(std::make_pair(surf, rayIDIG));
				if (res != 0) { continue; }

				float dist = std::max(width, length);
				float y = baseY;

				for (const auto& poi : intersection.GetPointsOfIntersection()) {
					float diffX = rayX - poi.pos.x();
					float diffZ = rayZ - poi.pos.z();
					float currDist = std::sqrt(diffX * diffX + diffZ * diffZ);

					if (currDist < dist) {
						dist = currDist;
						y = poi.pos.y();
					}
				}

				if (y > heightmap[x][z]) {
					heightmap[x][z] = y;
				}

			}
		}
	}
	return heightmap;
}

std::vector<gmod::vector3<float>> StageOne::MakeSmooth(const std::vector<gmod::vector3<float>>& path) const {
	if (path.size() < 3) { return path; }

	std::vector<gmod::vector3<float>> smoothed = path;
	std::vector<bool> smooth(smoothed.size(), false);
	const float maxSlope = std::tan(maxSlopeInDeg * std::numbers::pi / 180.f);
	const int maxIterations = 100;

	bool changed = false;
	int iter = 0;

	do {
		changed = false;
		for (int i = 1; i < smoothed.size() - 1; i++) {
			if (smooth[i]) { 
				continue;
			}
			const auto& prev = smoothed[i - 1];
			const auto& curr = smoothed[i];
			const auto& next = smoothed[i + 1];

			const float diffXprev = curr.x() - prev.x();
			const float diffZprev = curr.z() - prev.z();
			const float diffXnext = curr.x() - next.x();
			const float diffZnext = curr.z() - next.z();

			float distToPrev = std::sqrt(diffXprev * diffXprev + diffZprev * diffZprev);
			float distToNext = std::sqrt(diffXnext * diffXnext + diffZnext * diffZnext);

			// skip vertical movements or duplicate points
			if (distToPrev < FZERO || distToNext < FZERO) {
				continue;
			}

			// calculate slopes
			float slopeToPrev = std::abs(curr.y() - prev.y()) / distToPrev;
			float slopeToNext = std::abs(curr.y() - next.y()) / distToNext;

			if (slopeToPrev > maxSlope || slopeToNext > maxSlope) {
				float targetY = (prev.y() + next.y()) / 2.0f;
				float minAllowedY = curr.y(); 

				// apply smoothing with constraint
				float newY = std::max(targetY, minAllowedY);

				// only update if actually changing and it's an improvement
				if (std::abs(newY - smoothed[i].y()) > FZERO) {
					smoothed[i].y() = newY;
					changed = true;
				} else {
					smooth[i] = true;
				}
			}
		}
		iter++;
	} while (changed && iter < maxIterations);

	return smoothed;
}
