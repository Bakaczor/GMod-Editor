cbuffer cbMillInfo : register(b3)
{
    float4 size;
    float4 centre;
    uint4 resolutions; 
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
};

struct DSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

#define PATCH_SIZE 4

HSPatchOutput HSPatch(InputPatch<HSInput, PATCH_SIZE> patch, uint patchID : SV_PrimitiveID)
{
    HSPatchOutput output;
    // adding "... * 1" fixes issues with compiler optimization in release
    // TODO : investigate the reason
    output.edges[0] = output.edges[2] = resolutions.x * 1;
    output.edges[1] = output.edges[3] = resolutions.y * 1;
    output.inside[0] = resolutions.y * 1;
    output.inside[1] = resolutions.x * 1;
    return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
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