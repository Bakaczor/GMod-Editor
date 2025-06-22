#pragma once
#include "Divisable.h"
#include "Edge.h"
#include "Patch.h"
#include "Transformable.h"
#include <unordered_set>

namespace app {
	enum class SurfaceType {
		Flat, Cylindric
	};

	class Surface : public Transformable, public Divisable {
	public:
		static const unsigned int minDivisions = 4;
		static const unsigned int maxDivisions = 64;

		using Cycle3 = std::array<Edge, 3>;
		static std::vector<Cycle3> FindCycles3(const std::vector<Surface*>& surfaces, bool includePatchBoundaries);

		Surface(bool increment = false);
		Surface(SurfaceType type, unsigned int aPoints, unsigned int bPoints, unsigned int divisions, std::vector<Object*> controlPoints, int id = -1);
		virtual ~Surface();
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const override;
		virtual void UpdateMesh(const Device& device) override;
		virtual void RenderProperties() override;
		virtual void Replace(int id, Object* obj) override;
		SurfaceType GetSurfaceType() const;
		unsigned int GetAPoints() const;
		unsigned int GetBPoints() const;

		const std::vector<Object*>& GetControlPoints() const;
		const Patch& GetPatch(int idx) const;
		void ClearControlPoints();
	protected:
		bool m_showNet = false;
		Mesh m_netMesh;

		std::vector<Patch> m_patches;
		Mesh m_surfaceMesh;

		unsigned int m_aPoints;
		unsigned int m_bPoints;
		SurfaceType m_surfaceType;
	private:
		int m_selectedIdx = -1;
		static unsigned short m_globalSurfaceNum;

		static const std::vector<std::pair<USHORT, USHORT>> m_borderEdges;

		struct BpData {
			int idx;
			std::unordered_map<int, std::unordered_set<int>> surfIdPatchIdx;
		};
		struct BoundaryPoint {
			Object** thisPoint = nullptr;
			bool isPatchBoundary = false;
			bool isSurfaceBoundary = false;
			bool isPatchCorner = false;
			bool isSurfaceCorner = false;
			std::unordered_map<Object**, BpData> neighbours;

			BpData data;
		};
		std::vector<BoundaryPoint> m_boundaryPoints;
		bool isPatchCorner(USHORT idx) const;

		void InitializeBoundaryPoints();
		void DetectSurfaceBoundaryPoints();

		static std::unordered_map<int, BoundaryPoint> CombineBoundaryPoints(const std::vector<Surface*>& surfaces, bool includePatchBoundaries);
		static std::vector<Cycle3> FindUniqueTrianglesInGraph(const std::unordered_map<int, std::vector<Edge>>& criticalGraph);
	};
}
