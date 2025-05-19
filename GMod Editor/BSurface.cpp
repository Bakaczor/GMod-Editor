#include "BSurface.h"

using namespace app;

BSurface::BSurface(SurfaceType type, float a, float b, unsigned int aPatch, unsigned int bPatch, unsigned int divisions) : Surface(type, a, b, aPatch, bPatch, divisions) {}

void BSurface::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {

}

void BSurface::UpdateMesh(const Device& device) {

}
