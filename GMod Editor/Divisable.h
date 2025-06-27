#pragma once
#include <utility>

class Divisable {
public:
	unsigned int GetDivisions() const {
		return m_divisions;
	}
	virtual std::pair<unsigned int, unsigned int> GetUVPatches() const {
		return std::make_pair(0, 0);
	}
protected:
	unsigned int m_divisions;
};