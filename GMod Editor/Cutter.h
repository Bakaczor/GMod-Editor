#pragma once
#include "../gmod/Transform.h"
#include "../gmod/matrix4.h"
#include <unordered_map>
#include "Mesh.h"
#include "CutterType.h"
#include "Shaders.h"

namespace app {
	class Cutter {
	public:
		const std::array<float, 4> color = { 0.4f, 0.4f, 0.4f, 1.f };
		bool propertiesChanged = false;
		bool showCutter = true;

		Cutter();
		void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const;
		void UpdateMesh(const Device& device);
		void RenderProperties();
		void RenderCutterOrientation();

		float GetRadius() const;
		float GetCuttingHeight() const;
		float GetTotalLength() const;
		float GetMaxAngle() const;
		bool isBaseOriented() const;
		gmod::vector3<float> GetPosition() const;

		void SetCutterDiameter(int diameter);
		void SetCutterType(CutterType type);
		void MoveTo(gmod::vector3<float> pos);

		gmod::matrix4<float> modelMatrix();
	private:
		Mesh m_mesh;
		const int m_parts = 32;
		gmod::Transform<float> m_transform;

		int m_selectedCutterType = 0;
		const static std::vector<CutterType> m_cutterTypes;
		const static std::vector<const char*> m_cutterTypeNames;

		// all in millimetres
		float m_millingPartDiameter = 1.f;
		float m_millingPartHeight = 1.f;
		float m_totalCutterLength = 10.f;

		CutterType m_cutterType = CutterType::Spherical;
		float m_maxHorizontalDeviationAngle = 0.0f; // degrees

		bool m_useCutterBase = true;

		void GenerateSpherical(std::vector<Vertex_PoNo>& verts, std::vector<USHORT>& idxs);
		void GenerateCylindrical(std::vector<Vertex_PoNo>& verts, std::vector<USHORT>& idxs) const;
	};
}