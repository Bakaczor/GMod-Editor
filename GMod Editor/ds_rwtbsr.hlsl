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

float4 N(float t)
{
    float N1[2] =
    {
        1 - t,
        t - 0
    };
    float N2[3] =
    {
        N1[0] * (1 - t) / 2,
        N1[0] * (t + 1) / 2 + N1[1] * (2 - t) / 2,
        N1[1] * (t - 0) / 2
    };
    float N3[4] =
    {
        N2[0] * (1 - t) / 3,
        N2[0] * (t + 2) / 3 + N2[1] * (2 - t) / 3,
        N2[1] * (t + 1) / 3 + N2[2] * (3 - t) / 3,
        N2[2] * (t - 0) / 3
    };
    
    return float4(N3[0], N3[1], N3[2], N3[3]);
}

float3 P(OutputPatch<DSInput, NUM_CONTROL_POINTS> patch, float2 uv)
{
    float4 Nu = N(uv.x);
    float4 Nv = N(uv.y);
    float tNu[4] = { Nu.x, Nu.y, Nu.z, Nu.w };
    float tNv[4] = { Nv.x, Nv.y, Nv.z, Nv.w };
    
    float3 pos = 0;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            pos += patch[i * 4 + j].wPosition * tNu[i] * tNv[j];
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
