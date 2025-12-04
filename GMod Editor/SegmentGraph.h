#pragma once
#include <unordered_map>
#include "../gmod/vector3.h"

namespace app {
	struct SegmentEnd2 {
		float x, z;
		bool isTopOrBottom;
		bool isOnContour; // !isTopOrBottom
		int id = -1;

		// positive if isOnContour == true
		long long prevIdx = -1;
		long long nextIdx = -1;
	};

	struct Segment2 {
		SegmentEnd2 p1;
		SegmentEnd2 p2;

		// positive only for contour segements
		long long interStartIdx = -1;
		long long interEndIdx = -1;
	};

	struct SegmentEnd3 {
		int id = -1;
		long contourIdx = -1;
	};

	struct Segment3 {
		SegmentEnd3 p1;
		SegmentEnd3 p2;

		bool contourSegment;
		std::vector<gmod::vector3<float>> p1p2; // only populated if contourSegment == false
	};

	class SegmentGraph {
	public:
		struct Edge2 {
			Segment2 seg;

			int v1, v2;
			float dist;
			bool isVertical;
			bool isHorizontal;
		};
		struct Vertex2 {
			SegmentEnd2 segEnd;

			std::vector<std::pair<int, Edge2>> neighbours;
		};

		struct Edge3 {
			Segment3 seg;

			int v1, v2;
			bool contourEdge;
		};
		struct Vertex3 {
			SegmentEnd3 segEnd;

			std::vector<std::pair<int, Edge3>> neighbours;
		};

		std::vector<Vertex2> vertices2;
		std::vector<Vertex3> vertices3;

		// for stage two
		SegmentGraph(const std::vector<std::pair<float, std::vector<Segment2>>>& verticalSegments,
			const std::vector<Segment2>& contourSegments, int vertNum);

		// for stage three
		SegmentGraph(const std::vector<Segment3>& innerSegments, const std::vector<Segment3>& contourSegments);

		std::vector<int> SpecialDFS2(int startVertex) const;
		std::vector<int> SpecialDFS3(int startVertex) const;
	};
}