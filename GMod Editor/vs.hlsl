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
    float4 color : COLOR;
};

PSInput main(VSInput input)
{
    float4 pos = float4(input.position, 1.0f);
    pos = mul(modelMatrix, pos);
    pos = mul(viewMatrix, pos);
    pos = mul(projMatrix, pos);
    
    PSInput output;
    output.position = pos;
    output.color = float4(input.color, 1.0f);
    return output;
}