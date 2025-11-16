cbuffer cbModel : register(b0)
{
    matrix modelMatrix;
};

cbuffer cbView : register(b1)
{
    matrix viewMatrix;
};

cbuffer cbProj : register(b2)
{
    matrix projMatrix;
};

cbuffer cbViewInv : register(b3)
{
    matrix viewMatrixInv;
};

cbuffer cbMillInfo : register(b4)
{
    float4 size;
    float4 centre;
    uint4 resolutions;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : POSITION;
    float3 view : VIEW;
    float2 uv : TEXCOORD;
};

struct GSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

uint isMillingEdge(float3 v1, float3 v2)
{
    const float halfSizeX = size.x / 2;
    const float halfSizeY = size.y / 2;
    
    const float leftX = centre.y - halfSizeY; // 1
    const float rightX = centre.y + halfSizeY; // 3
    const float topZ = centre.x - halfSizeX; // 2
    const float bottomZ = centre.x + halfSizeX; // 4
    
    if (abs(v1.x - v2.x) < 1e-7f)
    {
        if (abs(v1.x - leftX) < 1e-7f)
        {
            return 1;
        }
        if (abs(v1.x - rightX) < 1e-7f)
        {
            return 3;
        }
    }
    
    if (abs(v1.z - v2.z) < 1e-7f)
    {
        if (abs(v1.z - topZ) < 1e-7f)
        {
            return 2;
        }
        if (abs(v1.z - bottomZ) < 1e-7f)
        {
            return 4;
        }          
    }
    
     return 0;
}

[maxvertexcount(15)]
void main(triangle GSInput input[3], inout TriangleStream<PSInput> outputStream)
{
    const float3 camPos = mul(viewMatrixInv, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    PSInput o;

    uint i;
    for (i = 0; i < 3; i++)
    {
        uint next_i = (i + 1) % 3;
        uint edge = isMillingEdge(input[i].position, input[next_i].position);
        if (edge == 0)
        {
            continue;
        }
        
        float3 v0, v1, v2, v3, n0, n1, n2, n3;
        float2 uv0, uv1, uv2, uv3;
        bool condX = input[i].position.x > input[next_i].position.x;
        bool condZ = input[i].position.z < input[next_i].position.z;
        if (edge == 1) // left
        {
            v0 = condZ ? input[i].position : input[next_i].position;
            v1 = condZ ? input[next_i].position : input[i].position;
            v2 = float3(v0.x, centre.z, v0.z);
            v3 = float3(v1.x, centre.z, v1.z);
            
            n0 = condZ ? input[i].normal : input[next_i].normal;
            n1 = condZ ? input[next_i].normal : input[i].normal;
            n2 = n3 = float3(-1, 0, 0);
            
            uv0 = condZ ? input[i].uv : input[next_i].uv;
            uv1 = condZ ? input[next_i].uv : input[i].uv;
        }
        else if (edge == 2) // top
        {
            v0 = condX ? input[i].position : input[next_i].position;
            v1 = condX ? input[next_i].position : input[i].position;
            v2 = float3(v0.x, centre.z, v0.z);
            v3 = float3(v1.x, centre.z, v1.z);
            
            n0 = condX ? input[i].normal : input[next_i].normal;
            n1 = condX ? input[next_i].normal : input[i].normal;
            n2 = n3 = float3(0, 0, -1);
            
            uv0 = condX ? input[i].uv : input[next_i].uv;
            uv1 = condX ? input[next_i].uv : input[i].uv;
        }
        else if (edge == 3) // right
        {
            v0 = !condZ ? input[i].position : input[next_i].position;
            v1 = !condZ ? input[next_i].position : input[i].position;
            v2 = float3(v0.x, centre.z, v0.z);
            v3 = float3(v1.x, centre.z, v1.z);
            
            n0 = !condZ ? input[i].normal : input[next_i].normal;
            n1 = !condZ ? input[next_i].normal : input[i].normal;
            n2 = n3 = float3(1, 0, 0);
            
            uv0 = !condZ ? input[i].uv : input[next_i].uv;
            uv1 = !condZ ? input[next_i].uv : input[i].uv;
        }
        else // bottom
        {
            v0 = !condX ? input[i].position : input[next_i].position;
            v1 = !condX ? input[next_i].position : input[i].position;
            v2 = float3(v0.x, centre.z, v0.z);
            v3 = float3(v1.x, centre.z, v1.z);
            
            n0 = !condX ? input[i].normal : input[next_i].normal;
            n1 = !condX ? input[next_i].normal : input[i].normal;
            n2 = n3 = float3(0, 0, 1);
            
            uv0 = !condX ? input[i].uv : input[next_i].uv;
            uv1 = !condX ? input[next_i].uv : input[i].uv;
        }
        uv2 = uv0;
        uv3 = uv1;
        
        uint j;
        GSInput tri1[3] =
        {
            { v0, n0, uv0 },
            { v2, n2, uv2 },
            { v3, n3, uv3 }
        };
        for (j = 0; j < 3; j++)
        {
            float4 worldPos = mul(modelMatrix, float4(tri1[j].position, 1.0f));
            o.view = normalize(camPos - worldPos.xyz);
            o.worldPos = worldPos.xyz;
            o.normal = tri1[j].normal;
            o.position = mul(projMatrix, mul(viewMatrix, worldPos));
            o.uv = tri1[j].uv;
            outputStream.Append(o);
        }
        outputStream.RestartStrip();
        
        GSInput tri2[3] =
        {
            { v0, n0, uv0 },
            { v3, n3, uv3 },
            { v1, n1, uv1 }
        };
        for (j = 0; j < 3; j++)
        {
            float4 worldPos = mul(modelMatrix, float4(tri2[j].position, 1.0f));
            o.view = normalize(camPos - worldPos.xyz);
            o.worldPos = worldPos.xyz;
            o.normal = tri2[j].normal;
            o.position = mul(projMatrix, mul(viewMatrix, worldPos));
            o.uv = tri2[j].uv;
            outputStream.Append(o);
        }
        outputStream.RestartStrip();
    }
    
    for (i = 0; i < 3; i++) {
        float4 worldPos = mul(modelMatrix, float4(input[i].position, 1.0f));
        o.view = normalize(camPos - worldPos.xyz);
        o.worldPos = worldPos.xyz;
        o.normal = input[i].normal;
        o.position = mul(projMatrix, mul(viewMatrix, worldPos));
        o.uv = input[i].uv;
        outputStream.Append(o);
    }
         
    outputStream.RestartStrip();
}