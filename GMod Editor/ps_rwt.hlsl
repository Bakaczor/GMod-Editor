cbuffer cbColor : register(b0)
{
    float4 color;
}

struct PSInput
{
    float4 position : SV_POSITION;
};

float4 main(PSInput input) : SV_TARGET
{
    return color;
}