cbuffer cbModel : register(b0)
{
    matrix modelMatrix;
};

cbuffer cbView : register(b1)
{
    matrix viewMatrix;
};

cbuffer cbProj : register(b2)
{
    matrix projMatrix;
};

cbuffer cbViewInv : register(b3)
{
    matrix viewMatrixInv;
};

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : POSITION;
    float3 view : VIEW;
};

PSInput main(VSInput input)
{
    PSInput output;
    float4 worldPos = mul(modelMatrix, float4(input.position, 1.0f));
    
    output.position = mul(projMatrix, mul(viewMatrix, worldPos));
    output.normal = normalize(input.normal);
    output.worldPos = worldPos.xyz;
    
    float3 camPos = mul(viewMatrixInv, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    output.view = normalize(camPos - output.worldPos);
    
    return output;
}