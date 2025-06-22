#pragma once

class Divisable {
public:
	unsigned int GetDivisions() const {
		return m_divisions;
	}
protected:
	unsigned int m_divisions;
};