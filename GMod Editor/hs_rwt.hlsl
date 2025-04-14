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
#define OFFSET 64

float TessFactor(InputPatch<HSInput, NUM_CONTROL_POINTS> patch)
{
    float sum = 0.0f;
    for (int i = 0; i < NUM_CONTROL_POINTS - 1; i++)
    {
        float4 u = mul(projMatrix, mul(viewMatrix, float4(patch[i].wPosition, 1.0f)));
        u /= u.w;
        float4 v = mul(projMatrix, mul(viewMatrix, float4(patch[i + 1].wPosition, 1.0f)));
        v /= v.w;
        sum += sqrt(pow(u.x - v.x, 2) + pow(u.y - v.y, 2));
    }
    return clamp(sum * OFFSET / 3.0f, 2.0f, 64.0f);
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
	return output;
}
