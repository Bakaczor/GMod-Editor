#pragma once
#include "../imgui/imgui.h"
#include "framework.h"
#include "Surface.h"

namespace app {
	class SurfaceBuilder {
	public:
		bool shouldBuild;

		SurfaceBuilder(std::vector<std::unique_ptr<Object>>& sceneObjects);
		bool RenderProperties();
		Surface* Build() const;
		void Reset();
		void SetC2(bool isC2);
	private:
		std::vector<std::unique_ptr<Object>>& m_sceneObjects;
		SurfaceType m_type;
		bool m_isC2;
		float m_a;
		float m_b;
		unsigned int m_aPatch;
		unsigned int m_bPatch;
		unsigned int m_divisions;

		Surface* BuildSurface() const;
		Surface* BuildBSurface() const;
	};
}
