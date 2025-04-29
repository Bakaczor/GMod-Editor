cbuffer cbView : register(b0)
{
    matrix viewMatrix;
};

cbuffer cbProj : register(b1)
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
    float EdgeTess[2] : SV_TessFactor;
};

#define NUM_CONTROL_POINTS 4

[domain("isoline")]
PSInput main(HSConstOutput input, float2 uv : SV_DomainLocation, const OutputPatch<DSInput, NUM_CONTROL_POINTS> patch)
{
    float t = uv.x;
    float3 d1[3] =
    {
        (1 - (t + 2) / 3) * patch[0].wPosition + (t + 2) / 3 * patch[1].wPosition,
        (1 - (t + 1) / 3) * patch[1].wPosition + (t + 1) / 3 * patch[2].wPosition,
        (1 - (t + 0) / 3) * patch[2].wPosition + (t + 0) / 3 * patch[3].wPosition
    };
    float3 d2[2] =
    {
        (1 - (t + 1) / 2) * d1[0] + (t + 1) / 2 * d1[1],
        (1 - (t + 0) / 2) * d1[1] + (t + 0) / 2 * d1[2]
    };
    
    PSInput output;
    float3 pos = (1 - t) * d2[0] + t * d2[1];
    output.position = mul(projMatrix, mul(viewMatrix, float4(pos, 1.0f)));
    return output;
}
