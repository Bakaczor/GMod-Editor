#pragma once
#include <unordered_map>

namespace app {
	struct SegmentEnd {
		float x, z;
		bool isTopOrBottom;
		bool isOnContour; // !isTopOrBottom
		int id = -1;

		// positive if isOnContour == true
		long long prevIdx = -1;
		long long nextIdx = -1;
	};

	struct Segment {
		SegmentEnd p1;
		SegmentEnd p2;

		// positive only for contour segements
		long long interStartIdx = -1;
		long long interEndIdx = -1;
	};

	class SegmentGraph {
	public:
		struct Edge {
			Segment seg;

			int v1, v2;
			float dist;
			bool isVertical;
			bool isHorizontal;
		};
		struct Vertex {
			SegmentEnd segEnd;

			std::vector<std::pair<int, Edge>> neighbours;
		};

		std::vector<Vertex> vertices;

		SegmentGraph(const std::vector<std::pair<float, std::vector<Segment>>>& verticalSegments,
			const std::vector<Segment>& contourSegments, int vertNum);

		std::vector<int> SpecialDFS(int startVertex) const;
	};
}