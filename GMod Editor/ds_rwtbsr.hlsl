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
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
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
PSInput main(HSConstOutput input, float2 uv : SV_DomainLocation, OutputPatch<DSInput, NUM_CONTROL_POINTS> patch)
{
    PSInput output;
    float3 pos = P(patch, uv);
    output.position = mul(projMatrix, mul(viewMatrix, float4(pos, 1.0f)));
    return output;
}
