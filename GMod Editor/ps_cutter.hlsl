cbuffer cbColor : register(b0)
{
    float4 surfaceColor;
}

cbuffer cbDirLight : register(b2)
{
    float4 direction;
    float4 color;
    float4 weights;
};

cbuffer cbMaterial : register(b3)
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float shininess;
    float3 padding3;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : POSITION;
    float3 view : VIEW;
};

float4 main(PSInput input) : SV_TARGET
{
    float3 final = weights.x * color.rgb * ambient.rgb * surfaceColor.rgb;
    float3 N = normalize(input.normal);
    float3 V = normalize(input.view);
    float3 L = -normalize(direction.xyz);
    float3 R = 2 * dot(N, L) * N - L;
    final += weights.y * color.rgb * diffuse.rgb * saturate(dot(N, L)) * surfaceColor.rgb;
    final += weights.z * color.rgb * specular.rgb * pow(saturate(dot(V, R)), shininess) * surfaceColor.rgb;
    return float4(saturate(final), 1.0f);
}