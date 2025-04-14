#include "Shaders.h"

using namespace app;

void Shaders::Set(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	context->VSSetShader(vertexShader.get(), nullptr, 0);
	context->PSSetShader(pixelShader.get(), nullptr, 0);
	context->IASetInputLayout(layout.get());
}

void Shaders::Unset(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	context->VSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(nullptr, nullptr, 0);
	context->IASetInputLayout(nullptr);
}
