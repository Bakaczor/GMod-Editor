Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(float4 position : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET
{
    return tex.Sample(samp, uv);
}