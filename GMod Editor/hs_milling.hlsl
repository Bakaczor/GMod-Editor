#define PATCH_SIZE 4

cbuffer cbMillInfo : register(b3)
{
    float3 size;
    float3 centre;
    uint3 resolutions; 
    float3 padding; 
};

struct HSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct HSPatchOutput
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
    uint patchID : ID;
};

struct DSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};


HSPatchOutput HSPatch(InputPatch<HSInput, PATCH_SIZE> patch, uint patchID : SV_PrimitiveID)
{
    HSPatchOutput output;
    output.edges[0] = output.edges[2] = (float) resolutions.x;
    output.edges[1] = output.edges[3] = (float) resolutions.y;
    output.inside[0] = (float) resolutions.y;
    output.inside[1] = (float) resolutions.x;
    output.patchID = patchID;
    return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(PATCH_SIZE)]
[patchconstantfunc("HSPatch")]
[maxtessfactor(64.0f)]
DSInput main(InputPatch<HSInput, PATCH_SIZE> patch, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    DSInput output;
    output.position = patch[i].position;
    output.normal = patch[i].normal;
    return output;
}