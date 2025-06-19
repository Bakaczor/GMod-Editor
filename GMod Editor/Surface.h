#pragma once
#include "Object.h"
#include "Patch.h"
#include <unordered_set>

namespace app {
	enum class SurfaceType {
		Flat, Cylindric
	};

	class Surface : public Object {
	public:
		static const unsigned int minDivisions = 4;
		static const unsigned int maxDivisions = 64;

		struct Edge {
			Object** start;
			Object** end;
			std::vector<Object**> intermediate;

			bool operator==(const Edge& other) const {
				return (start == other.start && end == other.end) || (start == other.end && end == other.start);
			}
		};
		using Cycle3 = std::array<Edge, 3>;
		static std::vector<Cycle3> FindCycles3(const std::vector<Surface*>& surfaces);

		Surface(bool increment = false);
		Surface(SurfaceType type, unsigned int aPoints, unsigned int bPoints, unsigned int divisions, std::vector<Object*> controlPoints);
		virtual ~Surface();
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const override;
		virtual void UpdateMesh(const Device& device) override;
		virtual void RenderProperties() override;
		virtual void Replace(int id, Object* obj) override;
		SurfaceType GetSurfaceType() const;
		unsigned int GetDivisions() const;
		unsigned int GetAPoints() const;
		unsigned int GetBPoints() const;

		const std::vector<Object*>& GetControlPoints() const;
		void ClearControlPoints();

#pragma region TRANSFORM
		virtual gmod::vector3<double> position() const override;
		virtual gmod::matrix4<double> modelMatrix() const override;
		virtual void SetTranslation(double tx, double ty, double tz) override;
		virtual void SetRotation(double rx, double ry, double rz) override;
		virtual void SetRotationAroundPoint(double rx, double ry, double rz, const gmod::vector3<double>& p) override;
		virtual void SetScaling(double sx, double sy, double sz) override;
		virtual void SetScalingAroundPoint(double sx, double sy, double sz, const gmod::vector3<double>& p) override;
		virtual void UpdateTranslation(double dtx, double dty, double dtz) override;
		virtual void UpdateRotation_Quaternion(double drx, double dry, double drz) override;
		virtual void UpdateRotationAroundPoint_Quaternion(double drx, double dry, double drz, const gmod::vector3<double>& p) override;
		virtual void UpdateScaling(double dsx, double dsy, double dsz) override;
		virtual void UpdateScalingAroundPoint(double dsx, double dsy, double dsz, const gmod::vector3<double>& p) override;
#pragma endregion
	protected:
		unsigned int m_divisions;
		bool m_showNet = false;
		//bool m_hidePoints = false;
		Mesh m_netMesh;

		std::vector<Object*> m_controlPoints;
		std::vector<Patch> m_patches;
		Mesh m_surfaceMesh;

		unsigned int m_aPoints;
		unsigned int m_bPoints;
		SurfaceType m_surfaceType;
		gmod::vector3<double> UpdateMidpoint();
	private:
		int m_selectedIdx = -1;
		static unsigned short m_globalSurfaceNum;
		gmod::vector3<double> m_midpoint;

		struct BoundaryPoint {
			Object** thisPoint = nullptr;
			bool isBoundary = false;
			bool isCorner = false;
			std::unordered_set<Object**> neighbours;
		};
		std::vector<BoundaryPoint> m_boundaryPoints;
		void UpdateBoundaryPoints();

		bool isCorner(USHORT idx) const;
		static const std::vector<std::pair<USHORT, USHORT>> m_borderEdges;

		static std::vector<Cycle3> FindUniqueTrianglesInGraph(const std::unordered_map<int, std::vector<Edge>>& criticalGraph);
	};
}
