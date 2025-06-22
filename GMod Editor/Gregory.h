#pragma once
#include "Divisable.h"
#include "Edge.h"
#include "Surface.h"
#include "Transformable.h"

namespace app {
	class Gregory : public Transformable, public Divisable {
	public:
		static const unsigned int minDivisions = 4;
		static const unsigned int maxDivisions = 64;

		Gregory(const std::array<Edge, 3>& edges, const std::vector<Surface*>& surfaces);
		virtual ~Gregory();
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const override;
		virtual void UpdateMesh(const Device& device) override;
		virtual void RenderProperties() override;
		virtual void Replace(int id, Object* obj) override;
	private:
		int m_selectedIdx = -1;
		static unsigned short m_globalGregoryNum;

		bool m_showVectors = false;
		Mesh m_vectorsMesh;
		Mesh m_gregoryMesh;

		static std::array<Object*, 4> RetrieveInwardEdge(const Edge& edge, const std::vector<Surface*>& surfaces);
		static std::optional<std::array<Object*, 4>> MatchInwardEdge(const std::array<Object*, Patch::patchSize>& patchPoints, const std::array<Object*, Patch::rowSize>& edgePoints);

		struct DeCasteljauRes {
			std::array<gmod::vector3<double>, 3> R;
			std::array<gmod::vector3<double>, 2> S;
			gmod::vector3<double> T;
		};
		static DeCasteljauRes DeCasteljau(const std::array<gmod::vector3<double>, 4>& controlPoints);
		
		struct EdgeData {
			DeCasteljauRes boundaryRes;
			std::array<gmod::vector3<double>, 4> bernsteinPoints;
			std::array<gmod::vector3<double>, 4> edgeGregPoints;
			gmod::vector3<double> P3;
			gmod::vector3<double> P2;
			gmod::vector3<double> P1;
			gmod::vector3<double> P0;
			gmod::vector3<double> Q;
		};
		struct PatchData {
			std::array<gmod::vector3<double>, 4> patchGregPoints;
		};
		static gmod::vector3<double> ExtrapolateToward(gmod::vector3<double>& A, gmod::vector3<double>& B);
		static gmod::vector3<double> Q(gmod::vector3<double>& P2, gmod::vector3<double>& P3);
		static gmod::vector3<double> P(const std::array<gmod::vector3<double>, 3>& Q);
		static gmod::vector3<double> P1(gmod::vector3<double>& Q, gmod::vector3<double>& P);

		struct TangentData {
			gmod::vector3<double> a;
			gmod::vector3<double> C;
			gmod::vector3<double> b;
		};
		static std::array<gmod::vector3<double>, 3> gVecs(const std::array<TangentData, 2>& tangentData);
		static std::array<gmod::vector3<double>, 3> cVecs(const std::array<gmod::vector3<double>, 4>& controlPoints);
		static gmod::vector3<double> B2(double v, const std::array<gmod::vector3<double>, 3>& gVecs);
		static std::array<double, 2> evaluateKH(const gmod::vector3<double>& b, const gmod::vector3<double>& g, const gmod::vector3<double>& c);
		static std::array<gmod::vector3<double>, 2> D(
			const std::array<gmod::vector3<double>, 2>& bVecs,
			const std::array<gmod::vector3<double>, 3>& gVecs,
			const std::array<gmod::vector3<double>, 4>& controlPoints);

		struct QuadGreg {
			std::array<gmod::vector3<double>, 4> cornerPoints;
			std::array<gmod::vector3<double>, 8> edgePoints;
			std::array<gmod::vector3<double>, 8> facePoints;
		};
		void CalculateGregoryPatchesAndTangentVectors(std::array<Gregory::QuadGreg, 3>& gregoryPatches, std::vector<gmod::vector3<double>>& tangentVectorsEnds) const;
	};
}