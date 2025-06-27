cbuffer cbView : register(b0)
{
    matrix viewMatrix;
};

cbuffer cbProj : register(b1)
{
    matrix projMatrix;
};

struct GSInput
{
    float4 position : SV_POSITION;
    float2 patchUV : UV;
    float2 uv : TEXCOORD;
};

struct DSInput
{
    float3 wPosition : WORLDPOS;
};

struct HSConstOutput
{
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
    uint patchID : ID;
    uint uPatches : USIZE;
    uint vPatches : VSIZE;
};

#define NUM_CONTROL_POINTS 16

float B(int i, float t)
{
    if (i == 0)
    {
        return (1 - t) * (1 - t) * (1 - t);

    }
    if (i == 1)
    {
        return 3 * t * (1 - t) * (1 - t);

    }
    if (i == 2)
    {
        return 3 * t * t * (1 - t);

    }
    if (i == 3)
    {
        return t * t * t;

    }
    return 0;
}

float3 P(OutputPatch<DSInput, NUM_CONTROL_POINTS> patch, float2 uv)
{
    float3 pos = 0;
    for (int i = 0; i < 4; i++)
    {
        float Bu = B(i, uv.x);
        for (int j = 0; j < 4; j++)
        {
            float Bv = B(j, uv.y);
            pos += patch[i * 4 + j].wPosition * Bu * Bv;
        }
    }
    return pos;
}

[domain("quad")]
GSInput main(HSConstOutput input, float2 uv : SV_DomainLocation, OutputPatch<DSInput, NUM_CONTROL_POINTS> patch)
{
    GSInput output;
    float3 pos = P(patch, uv);
    output.position = mul(projMatrix, mul(viewMatrix, float4(pos, 1.0f)));
    output.patchUV = uv;
    
    int uID = input.patchID % input.uPatches;
    int vID = input.patchID / input.uPatches;
    output.uv = float2(uv.y + uID, uv.x + vID);
    output.uv.x /= input.uPatches;
    output.uv.y /= input.vPatches;
    
    return output;
}
