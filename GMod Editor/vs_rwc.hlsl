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

struct VSInput
{
    float3 position : POSITION;
    float3 color : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

PSInput main(VSInput input)
{
    PSInput output;
    float4 pos = float4(input.position, 1.0f);
    output.position = mul(projMatrix, mul(viewMatrix, mul(modelMatrix, pos)));
    output.color = input.color;
    return output;
}