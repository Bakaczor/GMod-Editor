#include "CISpline.h"
#include <numeric>

using namespace app;

unsigned short CISpline::m_globalCISplineNum = 0;

CISpline::CISpline(std::vector<Object*> objects) {
	m_type = "CISpline";
	std::ostringstream os;
	os << "cispline_" << m_globalCISplineNum;
	name = os.str();
	m_globalCISplineNum += 1;

	this->objects = objects;
	for (auto& obj : this->objects) {
		obj->AddParent(this);
	}
	geometryChanged = true;
}

void CISpline::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	if (m_showPolyline) {
		map.at(ShaderType::Regular).Set(context);
		m_polylineMesh.Render(context);
	}
	if (objects.size() > 1) {
		map.at(ShaderType::RegularWithTesselationCISpline).Set(context);
		m_curveMesh.Render(context);
	}
}

void CISpline::UpdateMesh(const Device& device) {
	std::vector<Vertex_Po> polyVerts;
	polyVerts.reserve(objects.size());
	std::vector<USHORT> polyIdxs(objects.size());
	std::iota(polyIdxs.begin(), polyIdxs.end(), 0);

	for (const auto& obj : objects) {
		const auto& pos = obj->position();
		polyVerts.push_back({ DirectX::XMFLOAT3(pos.x(), pos.y(), pos.z()) });
	}

	if (objects.size() > 0) {
		m_polylineMesh.Update(device, polyVerts, polyIdxs, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	}

	std::vector<Vertex_PoCoef> curveVerts;
	curveVerts.reserve(objects.size());
	std::vector<USHORT> curveIdxs;
	curveIdxs.reserve(2 * (objects.size() - 1));

	auto coefficients = ComputeCoefficients();
	for (int i = 0; i < objects.size(); ++i) {
		const auto& pos = objects[i]->position();
		const auto& coef = coefficients[i];
		curveVerts.push_back(Vertex_PoCoef(
			{ DirectX::XMFLOAT3(pos.x(), pos.y(), pos.z()) },
			{ DirectX::XMFLOAT3(coef.x(), coef.y(), coef.z()) }
		));
	}

	for (int i = 0; i < objects.size() - 1; ++i) {
		curveIdxs.push_back(static_cast<USHORT>(i));
		curveIdxs.push_back(static_cast<USHORT>(i + 1));
	}

	if (objects.size() > 1) {
		m_curveMesh.Update(device, curveVerts, curveIdxs, D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST);
	}

	Object::UpdateMesh(device);
}

std::vector<gmod::vector3<double>> app::CISpline::ComputeCoefficients() const {
	const int n = objects.size() - 1;
	if (n == 1) {
		return std::vector<gmod::vector3<double>>(2);
	}

	// [h_0 ... h_n-1]
	std::vector<double> h(n);
	for (int i = 0; i < n; ++i) {
		const auto& pi = objects[i]->position();
		const auto& pip1 = objects[i + 1]->position();
		h[i] = gmod::distance(pip1, pi);
	}

	// [alpha_1 ... alpha_n-1]
	std::vector<double> alpha(n - 1);
	// [beta_1 ... beta_n-1]
	std::vector<double> beta(n - 1);
	// [gamma_1 ... gamma_n-1]
	std::vector<double> gamma(n - 1);
	// [R_1 ... R_n-1]
	std::vector<gmod::vector3<double>> R(n - 1);
	for (int i = 1, j = 0; i < n; ++i, ++j) {
		const auto& pim1 = objects[i - 1]->position();
		const auto& pi = objects[i]->position();
		const auto& pip1 = objects[i + 1]->position();

		const double& him1 = h[i - 1];
		const double& hi = h[i];

		alpha[j] = him1;
		beta[j] = 2 * (him1 + hi);
		gamma[j] = hi;
		R[j] = 3 * (((pip1 - pi) * (1.0 / hi)) - ((pi - pim1) * (1.0 / him1)));
	}

	// trójdiagonalna -> trójk¹tna górna
	for (int j = 1; j < n - 1; ++j) {
		const double mi = alpha[j] / beta[j - 1];
		alpha[j] = 0;
		beta[j] = beta[j] - mi * gamma[j - 1];
		R[j] = R[j] - mi * R[j - 1];
	}

	// [c_0 c_1 ... c_n-1 c_n]
	std::vector<gmod::vector3<double>> coefficients(objects.size());
	coefficients[0] = 0;
	coefficients[n] = 0;

	coefficients[n - 1] = R.back() * (1.0 / beta.back());
	for (int i = n - 2, j = n - 3; i >= 1; --i, --j) {
		coefficients[i] = (R[j] - gamma[j] * coefficients[j + 1]) * (1.0 / beta[j]);
	}

	return coefficients;
}
