#pragma once
#include "Object.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace app {
	struct NetPoint {
		Object** thisPoint = nullptr;
		std::unordered_map<int, std::unordered_set<int>> surfIdPatchIdx;
	};
	struct Edge {
		NetPoint start;
		NetPoint end;
		std::vector<NetPoint> intermediate;

		bool operator==(const Edge& other) const {
			return ((*start.thisPoint)->id == (*other.start.thisPoint)->id && (*end.thisPoint)->id == (*other.end.thisPoint)->id) ||
				((*start.thisPoint)->id == (*other.end.thisPoint)->id && (*end.thisPoint)->id == (*other.start.thisPoint)->id);
		}
	};
}