Texture2D tex1 : register(t0); 
Texture2D tex2 : register(t1);
SamplerState samp : register(s0); 

cbuffer cbColor : register(b0)
{
    float4 color;
}

cbuffer cbTrimInfo : register(b1)
{
    uint apply;
    uint textureID; 
    uint trimmingMode;
    float padding;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    if (apply == 1)
    {
        float side = 0.5f;
        if (textureID == 1)
        {
            side = tex1.Sample(samp, input.uv).r;
        }
        else if (textureID == 2)
        {
            side = tex2.Sample(samp, input.uv).r;
        }
        
        if (trimmingMode == 1 && side < 0.5f)
        {
            discard;
        }
        else if (trimmingMode == 2 && side > 0.5f)
        {
            discard;
        }
    }
    return color;
}