Texture2D<float> heightMap : register(t0);
SamplerState samp : register(s0);

cbuffer cbMillInfo : register(b4)
{
    float4 size;
    float4 centre;
    uint4 resolutions;
};

struct VSInput
{
    float3 position : POSITION;
};

struct GSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

float2 calculate_uv(float3 pos)
{
    float2 uv;
    uv.x = (pos.z + size.x / 2 - centre.x) / size.x;
    uv.y = (pos.x + size.y / 2 - centre.y) / size.y;
    return uv;
}

float get_y(float2 uv)
{
    //float3 tex = heightMap.SampleLevel(samp, uv.yx, 0).rgb;
    //float maxH = (tex.g + tex.b / 100.0f) * 255.f;
    //return tex.r * maxH + centre.z;
    return heightMap.SampleLevel(samp, uv, 0);
}

float3 calculate_normal(float2 uv)
{
    const float uvStepX = 1.0f / (resolutions.x - 1);
    const float uvStepY = 1.0f / (resolutions.y - 1);
    
    const float stepX = size.x * uvStepX;
    const float stepY = size.y * uvStepY;
   
    float z_center = get_y(uv);
    float dz_dx, dz_dy;
    
    if (uv.x <= 0.0f)
    {
        // Top edge - forward difference
        float z_below = get_y(float2(uv.x + uvStepX, uv.y));
        dz_dx = (z_below - z_center) / stepX;
    }
    else if (uv.x >= 1.0f)
    {
        // Bottom edge - backward difference
        float z_above = get_y(float2(uv.x - uvStepX, uv.y));
        dz_dx = (z_center - z_above) / stepX;
    }
    else
    {
        // Interior - central difference
        float z_above = get_y(float2(uv.x - uvStepX, uv.y));
        float z_below = get_y(float2(uv.x + uvStepX, uv.y));
        dz_dx = (z_below - z_above) / (2.0f * stepX);
    }
    
    if (uv.y <= 0.0f)
    {
        // Left edge - forward difference
        float z_right = get_y(float2(uv.x, uv.y + uvStepY));
        dz_dy = (z_right - z_center) / stepY;
    }
    else if (uv.y >= 1.0f)
    {
        // Right edge - backward difference  
        float z_left = get_y(float2(uv.x, uv.y - uvStepY));
        dz_dy = (z_center - z_left) / stepY;
    }
    else
    {
        // Interior - central difference
        float z_left = get_y(float2(uv.x, uv.y - uvStepY));
        float z_right = get_y(float2(uv.x, uv.y + uvStepY));
        dz_dy = (z_right - z_left) / (2.0f * stepY);
    }
    
    float3 normal = float3(-dz_dy, 1.0f, -dz_dx);
    return normalize(normal);
}


GSInput main(VSInput input)
{
    GSInput output;
    float2 localUV = calculate_uv(input.position);
    output.uv = localUV.yx; // uv names are actually flipped
    output.position = float3(input.position.x, get_y(localUV), input.position.z);
    output.normal = calculate_normal(localUV);

    return output;
}