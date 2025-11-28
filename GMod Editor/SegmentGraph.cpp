#include "SegmentGraph.h"
#include <set>
#include <stack>

using namespace app;

SegmentGraph::SegmentGraph(const std::vector<std::pair<float, std::vector<Segment>>>& verticalSegments,
	const std::vector<Segment>& contourSegments, int vertNum) {

	vertices = std::vector<Vertex>(vertNum);

	// add contour edges
	for (const auto& seg : contourSegments) {
		float diffX = seg.p1.x - seg.p2.x;
		float diffZ = seg.p1.z - seg.p2.z;
		float dist = std::sqrt(diffX * diffX + diffZ * diffZ);

		int v1 = seg.p1.id;
		int v2 = seg.p2.id;

		vertices[v1].segEnd = seg.p1;
		vertices[v2].segEnd = seg.p2;

		vertices[v1].neighbours.push_back(std::make_pair(v2, Edge{ seg, v1, v2, dist, false, false }));
		vertices[v2].neighbours.push_back(std::make_pair(v1, Edge{ seg, v2, v1, dist, false, false }));
	}

	// add vertical edges
	for (const auto& [xVal, segments] : verticalSegments) {
		for (const auto& seg : segments) {
			float diffZ = seg.p1.z - seg.p2.z;

			int v1 = seg.p1.id;
			int v2 = seg.p2.id;

			vertices[v1].segEnd = seg.p1;
			vertices[v2].segEnd = seg.p2;

			vertices[v1].neighbours.push_back(std::make_pair(v2, Edge{ seg, v1, v2, diffZ, true, false }));
			vertices[v2].neighbours.push_back(std::make_pair(v1, Edge{ seg, v2, v1, diffZ, true, false }));
		}
	}

	// add horizontal edges at the top and bottom
	for (int i = 0; i < verticalSegments.size() - 1; i++) {
		auto& leftTop = verticalSegments[i].second.front().p1;
		auto& rightTop = verticalSegments[i + 1].second.front().p1;

		float diffX = leftTop.x - rightTop.x;

		int v1Top = leftTop.id;
		int v2Top = rightTop.id;
		Segment topSeg(leftTop, rightTop);

		vertices[v1Top].segEnd = leftTop;
		vertices[v2Top].segEnd = rightTop;

		vertices[v1Top].neighbours.push_back(std::make_pair(v2Top, Edge{ topSeg, v1Top, v2Top, diffX, false, true }));
		vertices[v2Top].neighbours.push_back(std::make_pair(v1Top, Edge{ topSeg, v2Top, v1Top, diffX, false, true }));

		auto& leftBottom = verticalSegments[i].second.back().p2;
		auto& rightBottom = verticalSegments[i + 1].second.back().p2;

		int v1Bottom = leftBottom.id;
		int v2Bottom = rightBottom.id;
		Segment bottomSeg(leftBottom, rightBottom);

		vertices[v1Bottom].segEnd = leftBottom;
		vertices[v2Bottom].segEnd = rightBottom;

		vertices[v1Bottom].neighbours.push_back(std::make_pair(v2Bottom, Edge{ bottomSeg, v1Bottom, v2Bottom, diffX, false }));
		vertices[v2Bottom].neighbours.push_back(std::make_pair(v1Bottom, Edge{ bottomSeg, v2Bottom, v1Bottom, diffX, false }));
	}
}

std::vector<int> SegmentGraph::SpecialDFS(int startVertex) const {
	std::set<std::pair<int, int>> uncoveredVerticalEdges;
	for (const auto& vertex : vertices) {
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

	std::vector<bool> visited(vertices.size(), false);
    visited[startVertex] = true;

    while (!stack.empty() && !uncoveredVerticalEdges.empty()) {
        int u = stack.top();
		auto& neighbours = vertices[u].neighbours;

        // find best neighbor (uncovered vertical > shorter edge > any neighbor)
		int bestNB = -1;
        int bestNeighbourIdx = -1;
        float bestScore = -1.0f;
		for (int i = 0; i < neighbours.size(); i++) {
			const auto& [nb, edge] = vertices[u].neighbours[i];

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
        }
    }

	return path;
}
