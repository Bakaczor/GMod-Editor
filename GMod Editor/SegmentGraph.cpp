#include "SegmentGraph.h"
#include <set>
#include <stack>

using namespace app;

SegmentGraph::SegmentGraph(const std::vector<std::pair<float, std::vector<Segment2>>>& verticalSegments,
	const std::vector<Segment2>& contourSegments, int vertNum) {

	vertices2 = std::vector<Vertex2>(vertNum);

	// add contour edges
	for (const auto& seg : contourSegments) {
		float diffX = seg.p1.x - seg.p2.x;
		float diffZ = seg.p1.z - seg.p2.z;
		float dist = std::sqrt(diffX * diffX + diffZ * diffZ);

		int v1 = seg.p1.id;
		int v2 = seg.p2.id;

		vertices2[v1].segEnd = seg.p1;
		vertices2[v2].segEnd = seg.p2;

		vertices2[v1].neighbours.push_back(std::make_pair(v2, Edge2{ seg, v1, v2, dist, false, false }));
		vertices2[v2].neighbours.push_back(std::make_pair(v1, Edge2{ seg, v2, v1, dist, false, false }));
	}

	// add vertical edges
	for (const auto& [xVal, segments] : verticalSegments) {
		for (const auto& seg : segments) {
			float diffZ = seg.p1.z - seg.p2.z;

			int v1 = seg.p1.id;
			int v2 = seg.p2.id;

			vertices2[v1].segEnd = seg.p1;
			vertices2[v2].segEnd = seg.p2;

			vertices2[v1].neighbours.push_back(std::make_pair(v2, Edge2{ seg, v1, v2, diffZ, true, false }));
			vertices2[v2].neighbours.push_back(std::make_pair(v1, Edge2{ seg, v2, v1, diffZ, true, false }));
		}
	}

	// add horizontal edges at the top and bottom
	for (int i = 0; i < verticalSegments.size() - 1; i++) {
		auto& leftTop = verticalSegments[i].second.front().p1;
		auto& rightTop = verticalSegments[i + 1].second.front().p1;

		float diffX = leftTop.x - rightTop.x;

		int v1Top = leftTop.id;
		int v2Top = rightTop.id;
		Segment2 topSeg(leftTop, rightTop);

		vertices2[v1Top].segEnd = leftTop;
		vertices2[v2Top].segEnd = rightTop;

		vertices2[v1Top].neighbours.push_back(std::make_pair(v2Top, Edge2{ topSeg, v1Top, v2Top, diffX, false, true }));
		vertices2[v2Top].neighbours.push_back(std::make_pair(v1Top, Edge2{ topSeg, v2Top, v1Top, diffX, false, true }));

		auto& leftBottom = verticalSegments[i].second.back().p2;
		auto& rightBottom = verticalSegments[i + 1].second.back().p2;

		int v1Bottom = leftBottom.id;
		int v2Bottom = rightBottom.id;
		Segment2 bottomSeg(leftBottom, rightBottom);

		vertices2[v1Bottom].segEnd = leftBottom;
		vertices2[v2Bottom].segEnd = rightBottom;

		vertices2[v1Bottom].neighbours.push_back(std::make_pair(v2Bottom, Edge2{ bottomSeg, v1Bottom, v2Bottom, diffX, false, true }));
		vertices2[v2Bottom].neighbours.push_back(std::make_pair(v1Bottom, Edge2{ bottomSeg, v2Bottom, v1Bottom, diffX, false, true }));
	}
}

SegmentGraph::SegmentGraph(const std::vector<Segment3>& innerSegments, const std::vector<Segment3>& contourSegments) {
	// TODO
}

std::vector<int> SegmentGraph::SpecialDFS2(int startVertex) const {
	std::set<std::pair<int, int>> uncoveredVerticalEdges;
	for (const auto& vertex : vertices2) {
		for (const auto& neighbour : vertex.neighbours) {
			if (neighbour.second.isVertical) {
				uncoveredVerticalEdges.insert(std::make_pair(
					std::min(neighbour.second.v1, neighbour.second.v2),
					std::max(neighbour.second.v1, neighbour.second.v2)
				));
			}
		}
	}
    std::vector<int> path = { startVertex };

    std::stack<int> stack;
    stack.push(startVertex);

	std::vector<bool> visited(vertices2.size(), false);
    visited[startVertex] = true;

    while (!stack.empty() && !uncoveredVerticalEdges.empty()) {
        int u = stack.top();
		auto& neighbours = vertices2[u].neighbours;

        // find best neighbor (uncovered vertical > shorter edge > any neighbor)
		int bestNB = -1;
        int bestNeighbourIdx = -1;
        float bestScore = -1.0f;
		for (int i = 0; i < neighbours.size(); i++) {
			const auto& [nb, edge] = vertices2[u].neighbours[i];

			// TODO : possibly we should prioritize visiting vertical edges over not visiting already visited vertices
			if (visited[nb]) { continue; }

            float score = 0.0f;
            if (edge.isVertical) {
                auto edgeKey = std::make_pair(std::min(u, nb), std::max(u, nb));
                if (uncoveredVerticalEdges.count(edgeKey) == 1) {
                    score += 1000.0f; // big bonus
                }
            }
            // prefer shorter
            score += 1.0f / (edge.dist + 0.1f);

            if (score > bestScore) {
                bestScore = score;
				bestNB = nb;
				bestNeighbourIdx = i;
            }
        }

        if (bestNB != -1) {
			const auto& edge = neighbours[bestNeighbourIdx].second;
            if (edge.isVertical) {
                auto edgeKey = std::make_pair(std::min(u, bestNB), std::max(u, bestNB));
				uncoveredVerticalEdges.erase(edgeKey);
            }

            visited[bestNB] = true;
            path.push_back(bestNB);
            stack.push(bestNB);
        } else {
            stack.pop();
			if (!stack.empty()) {
				// moving back
				path.push_back(stack.top());
			}
        }
    }

	return path;
}

std::vector<int> SegmentGraph::SpecialDFS3(int startVertex) const {
	// TODO
	return std::vector<int>();
}
