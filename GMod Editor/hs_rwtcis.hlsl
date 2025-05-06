cbuffer cbView : register(b0)
{
    matrix viewMatrix;
};

cbuffer cbProj : register(b1)
{
    matrix projMatrix;
};

struct HSInput
{
    float3 wPosition : WORLDPOS;
    float3 wCoefficient : WORLDCOEF;
};

struct DSInput
{
    float3 wPosition : WORLDPOS;
    float3 wCoefficient : WORLDCOEF;
};

struct HSConstOutput
{
    float EdgeTess[2] : SV_TessFactor;
};

#define NUM_CONTROL_POINTS 2
#define OFFSET 256

float TessFactor(InputPatch<HSInput, NUM_CONTROL_POINTS> patch)
{
    float4 u = mul(projMatrix, mul(viewMatrix, float4(patch[0].wPosition, 1.0f)));
    u /= u.w;
    float4 v = mul(projMatrix, mul(viewMatrix, float4(patch[1].wPosition, 1.0f)));
    v /= v.w;
    
    float diffX = u.x - v.x;
    float diffY = u.y - v.y;
    float dist = sqrt(diffX * diffX + diffY * diffY);
    
    return clamp(dist * (OFFSET / NUM_CONTROL_POINTS) / 3.0f, 2.0f, 64.0f);
}

HSConstOutput ConstantHS(InputPatch<HSInput, NUM_CONTROL_POINTS> patch, uint patchID : SV_PrimitiveID)
{
    HSConstOutput output;
    output.EdgeTess[0] = 1;
    output.EdgeTess[1] = TessFactor(patch);
    return output;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(NUM_CONTROL_POINTS)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
DSInput main(InputPatch<HSInput, NUM_CONTROL_POINTS> patch, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    DSInput output;
    output.wPosition = patch[i].wPosition;
    output.wCoefficient = patch[i].wCoefficient;
    return output;
}
