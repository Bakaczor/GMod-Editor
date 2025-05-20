cbuffer cbColor : register(b0)
{
    float4 color;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    float2 minCoord = min(input.uv, 1.0 - input.uv);
    float radius = min(minCoord.x, minCoord.y);
    const float lineWidth = 0.005f;
    float isEdge = radius < lineWidth ? 1.0 : 0.0;
    
    return float4(color.rgb, isEdge * 0.95 + 0.05);
}