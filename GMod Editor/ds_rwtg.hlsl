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
};

#define NUM_CONTROL_POINTS 20

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

struct FArray
{
    float3 F0;
    float3 F1;
    float3 F2;
    float3 F3;
};

FArray F(OutputPatch<DSInput, NUM_CONTROL_POINTS> patch, float2 uv)
{
    const float u = uv.x;
    const float v = uv.y;
    
    const float3 f0m = patch[12].wPosition;
    const float3 f0p = patch[13].wPosition;
    const float3 f1m = patch[14].wPosition;
    const float3 f1p = patch[15].wPosition;
    const float3 f2m = patch[16].wPosition;
    const float3 f2p = patch[17].wPosition;
    const float3 f3m = patch[18].wPosition;
    const float3 f3p = patch[19].wPosition;
    
    FArray res;
    res.F0 = (u * f0p + v * f0m) / (u + v + 0.000001f);
    res.F1 = ((1 - u) * f1m + v * f1p) / (1 - u + v + 0.000001f);
    res.F2 = ((1 - u) * f2p + (1 - v) * f2m) / (2 - u - v + 0.000001f);
    res.F3 = (u * f3m + (1 - v) * f3p) / (1 + u - v + 0.000001f);
    
    return res;
}

float3 P(OutputPatch<DSInput, NUM_CONTROL_POINTS> patch, float2 uv)
{
    FArray Fs = F(patch, uv);
    float3 G[16];
    G[0] = patch[0].wPosition;
    G[1] = patch[4].wPosition;
    G[2] = patch[11].wPosition;
    G[3] = patch[3].wPosition;
    G[4] = patch[5].wPosition;
    G[5] = Fs.F0;
    G[6] = Fs.F3;
    G[7] = patch[10].wPosition;
    G[8] = patch[6].wPosition;
    G[9] = Fs.F1;
    G[10] = Fs.F2;
    G[11] = patch[9].wPosition;
    G[12] = patch[1].wPosition;
    G[13] = patch[7].wPosition;
    G[14] = patch[8].wPosition;
    G[15] = patch[2].wPosition;
    
    float3 pos = 0;
    for (int i = 0; i < 4; i++)
    {
        float Bu = B(i, uv.x);
        for (int j = 0; j < 4; j++)
        {
            float Bv = B(j, uv.y);
            pos += G[i * 4 + j] * Bu * Bv;
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
    output.uv = uv;
    return output;
}
