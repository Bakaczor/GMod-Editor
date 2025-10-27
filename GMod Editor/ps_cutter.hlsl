cbuffer cbColor : register(b0)
{
    float4 surfaceColor;
}

cbuffer cbDirLight : register(b2)
{
    float3 direction;
    float3 color;
    float3 weights;
    float3 padding3;
};

cbuffer cbMaterial : register(b3)
{
    float3 ambient;
    float3 diffuse;
    float3 specular;
    float shininess;
    float2 padding2;
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
    float3 final = weights.x * color * ambient * surfaceColor.rgb;
    float3 N = normalize(input.normal);
    float3 V = normalize(input.view);
    float3 L = -normalize(direction);
    float3 R = 2 * dot(N, L) * N - L;
    final += weights.y * color * diffuse * saturate(dot(N, L)) * surfaceColor.rgb;
    final += weights.z * color * specular * pow(saturate(dot(V, R)), shininess) * surfaceColor.rgb;
    return float4(saturate(final), 1.0f);
}