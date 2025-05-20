#pragma once
#include "Spline.h"

namespace app {
	class BSpline : public Spline {
	public:
		bool showBernstein = false;
		bool localChanges = true;
		std::vector<std::unique_ptr<Object>> bernsteinPoints;

		BSpline(std::vector<Object*> objects);
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const override;
		virtual void UpdateMesh(const Device& device) override;
		virtual void RenderProperties() override;
		virtual std::optional <std::vector<std::unique_ptr<Object>>*> GetSubObjects() override;
	private:
		static unsigned short m_globalBSplineNum;
		static gmod::matrix4<double> Mbs;
		static gmod::matrix4<double> Mbs_inv;
		Mesh m_bernsteinMesh;

		void MoveDeBoore(Object* sender);
		void RecalculateDeBoore();
		void RecalculateBernstein(Object* sender, bool remake = true);
	};
}