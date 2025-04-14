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
    float edges[4]  : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 4

HSConstOutput HSDeg3Bezier(InputPatch<HSInput, NUM_CONTROL_POINTS> ip, uint PatchID : SV_PrimitiveID)
{
    HSConstOutput output;
    output.edges[0] = output.edges[1] =
    output.edges[2] = output.edges[3] = 16;
    output.inside[0] = output.inside[1] = 16;
    return output;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(4)]
[patchconstantfunc("HSDeg3Bezier")]
DSInput main(InputPatch<HSInput, NUM_CONTROL_POINTS> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID)
{
    DSInput output;
    output.wPosition = ip[i].wPosition;
	return output;
}
