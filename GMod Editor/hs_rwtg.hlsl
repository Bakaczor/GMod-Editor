cbuffer cbView : register(b0)
{
    matrix viewMatrix;
};

cbuffer cbProj : register(b1)
{
    matrix projMatrix;
};

cbuffer cbTessConst : register(b2)
{
    uint divisions;
    float3 padding;
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
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 20

HSConstOutput ConstantHS(InputPatch<HSInput, NUM_CONTROL_POINTS> patch, uint patchID : SV_PrimitiveID)
{
    HSConstOutput output;
    output.EdgeTess[0] = output.EdgeTess[1] = output.EdgeTess[2] = output.EdgeTess[3] = divisions - 1;
    output.InsideTess[0] = output.InsideTess[1] = divisions - 1;
    return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(NUM_CONTROL_POINTS)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
DSInput main(InputPatch<HSInput, NUM_CONTROL_POINTS> patch, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    DSInput output;
    output.wPosition = patch[i].wPosition;
    return output;
}
