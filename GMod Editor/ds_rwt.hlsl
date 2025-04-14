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
    float B[4] = { 
        (1 - t) * (1 - t) * (1 - t),
        3 * t * (1 - t) * (1 - t),
        3 * t * t * (1 - t),
        t * t * t
    };
    PSInput output;
    float3 pos = float3(0, 0, 0);
    for (int i = 0; i < NUM_CONTROL_POINTS; i++)
    {
        pos += B[i] * patch[i].wPosition;
    }
    output.position = mul(projMatrix, mul(viewMatrix, float4(pos, 1.0f)));
    return output;
}
