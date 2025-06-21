#pragma once
#include "Transformable.h"

//namespace app {
//	class Gregory : public Transformable {
//	public:
//		Gregory(std::vector<Object*> controlPoints);
//		virtual ~Gregory();
//		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const override;
//		virtual void UpdateMesh(const Device& device) override;
//		virtual void RenderProperties() override;
//		virtual void Replace(int id, Object* obj) override;
//		unsigned int GetDivisions() const;
//	private:
//		int m_selectedIdx = -1;
//		static unsigned short m_globalGregoryNum;
//
//		bool m_showVectors = false;
//		Mesh m_vectorsMesh;
//
//		unsigned int m_divisions;
//		Mesh m_gregoryMesh;
//	};
//}