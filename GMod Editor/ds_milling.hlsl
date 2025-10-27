#define PATCH_SIZE 4

struct GSInput
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

[domain("quad")]
GSInput main(HSPatchOutput factors, float2 uv : SV_DomainLocation, const OutputPatch<DSInput, PATCH_SIZE> patch)
{
    GSInput output;
    
   // bilinear interpolation
    float3 v1_pos = lerp(patch[0].position, patch[1].position, uv.x);
    float3 v2_pos = lerp(patch[2].position, patch[3].position, uv.x);
    output.position = lerp(v1_pos, v2_pos, uv.y);
    
    // bilinear interpolation
    float3 v1_norm = lerp(patch[0].normal, patch[1].normal, uv.x);
    float3 v2_norm = lerp(patch[2].normal, patch[3].normal, uv.x);
    output.normal = normalize(lerp(v1_norm, v2_norm, uv.y));

    return output;
}