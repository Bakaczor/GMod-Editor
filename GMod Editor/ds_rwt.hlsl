cbuffer cbView : register(b1)
{
    matrix viewMatrix;
};

cbuffer cbProj : register(b2)
{
    matrix projMatrix;
};

struct PSInput
{
    float4 position : SV_POSITION;
};

struct DSInput
{
    float3 wPosition : WORLDPOS;
};

struct HSConstOutput
{
    float edges[4]  : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 4

[domain("isoline")]
PSInput main(HSConstOutput input, float2 uv : SV_DomainLocation, const OutputPatch<DSInput, NUM_CONTROL_POINTS> patch)
{
    PSInput output;
    float3 pos =
        pow(1 - uv.x, 3) * patch[0].wPosition +
        3 * pow(1 - uv.x, 2) * uv.x * patch[1].wPosition +
        3 * (1 - uv.x) * pow(uv.x, 2) * patch[2].wPosition +
        pow(uv.x, 3) * patch[3].wPosition;
    
    output.position = mul(projMatrix, mul(viewMatrix, float4(pos, 1.0f)));
    return output;
}
