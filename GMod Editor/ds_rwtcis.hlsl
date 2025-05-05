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
    float3 wPosition : WORLDPOS0;
    float3 wCoefficient : WORLDPOS1;
};

struct HSConstOutput
{
    float EdgeTess[2] : SV_TessFactor;
};

#define NUM_CONTROL_POINTS 2

[domain("isoline")]
PSInput main(HSConstOutput input, float2 uv : SV_DomainLocation, const OutputPatch<DSInput, NUM_CONTROL_POINTS> patch)
{
    float3 Pi = patch[0].wPosition;
    float3 Pip1 = patch[1].wPosition;
    float3 ci = patch[0].wCoefficient;
    float3 cip1 = patch[1].wCoefficient;
    
    float diffX = Pip1.x - Pi.x;
    float diffY = Pip1.y - Pi.y;
    float diffZ = Pip1.z - Pi.z;
    
    float h = sqrt(diffX * diffX + diffY * diffY + diffZ * diffZ);
    float t = uv.x * h;
    
    float3 ai = Pi;
    float3 bi = (Pip1 - Pi) / h - h * (2 * ci + cip1) / 3;
    float3 di = (cip1 + ci) / (3 * h);
    
    PSInput output;
    float3 pos = ai + t * (bi + t * (ci + t * di));
    output.position = mul(projMatrix, mul(viewMatrix, float4(pos, 1.0f)));
    return output;
}
