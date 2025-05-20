#pragma once
#include "../imgui/imgui.h"
#include "framework.h"
#include "Surface.h"

namespace app {
	class SurfaceBuilder {
	public:
		bool shouldBuild;

		SurfaceBuilder();
		bool RenderProperties();
		Surface* Build() const;
		void Reset();
	private:
		static const unsigned int m_minDivisions = 4;
		SurfaceType m_type;
		bool m_isC2;
		float m_a;
		float m_b;
		unsigned int m_aPatch;
		unsigned int m_bPatch;
		unsigned int m_divisions;
	};
}
