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
    float3 size;
    float3 centre;
    uint3 resolutions;
    float3 padding;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : POSITION;
    float3 view : VIEW;
};

struct GSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

const float halfSizeX = size.x / 2;
const float halfSizeY = size.y / 2;
    
const float leftX = centre.y - halfSizeY; // 1
const float rightX = centre.y + halfSizeY; // 3
const float topZ = centre.x - halfSizeX; // 2
const float bottomZ = centre.x + halfSizeX; // 4

int isMillingEdge(float3 v1, float3 v2)
{
    if (abs(v1.x - v2.x) < 1e-6f)
    {
        if (abs(v1.x - leftX) < 1e-6f)
        {
            return 1;
        }
        if (abs(v1.x - rightX) < 1e-6f)
        {
            return 3;
        }
        return 0;
    }
    
    if (abs(v1.z - v2.z) < 1e-6f)
    {
        if (abs(v1.z - topZ) < 1e-6f)
        {
            return 2;
        }
        if (abs(v1.z - bottomZ) < 1e-6f)
        {
            return 4;
        }          
        return 0;
    }
    
     return 0;
}

[maxvertexcount(6)]
void main(triangle GSInput input[3], inout TriangleStream<PSInput> outputStream)
{
    PSInput o;
    const float3 camPos = mul(viewMatrixInv, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    
    float3 positions[3] = { input[0].position, input[1].position, input[2].position };
    
    // use heightmap to adjust y

    for (int i = 0; i < 3; i++)
    {
        int next_i = (i + 1) % 3;
        int edge = isMillingEdge(positions[i], positions[next_i]);
        if (edge < 0)
        {
            continue;
        }
        
        float3 v0, v1, v2, v3, n0, n1, n2, n3;
        bool condX = positions[i].x > positions[next_i].x;
        bool condZ = positions[i].z < positions[next_i].z;
        if (edge == 1) // left
        {
            v0 = condZ ? positions[i] : positions[next_i];
            v1 = condZ ? positions[next_i] : positions[i];
            v2 = float3(v0.x, centre.z, v0.z);
            v3 = float3(v1.x, centre.z, v1.z);
            
            n0 = condZ ? input[i].normal : input[next_i].normal;
            n1 = condZ ? input[next_i].normal : input[i].normal;
            n2 = n3 = float3(-1, 0, 0);
        }
        else if (edge == 2) // top
        {
            v0 = condX ? positions[i] : positions[next_i];
            v1 = condX ? positions[next_i] : positions[i];
            v2 = float3(v0.x, centre.z, v0.z);
            v3 = float3(v1.x, centre.z, v1.z);
            
            n0 = condX ? input[i].normal : input[next_i].normal;
            n1 = condX ? input[next_i].normal : input[i].normal;
            n2 = n3 = float3(0, 0, -1);
        }
        else if (edge == 3) // right
        {
            v0 = !condZ ? positions[i] : positions[next_i];
            v1 = !condZ ? positions[next_i] : positions[i];
            v2 = float3(v0.x, centre.z, v0.z);
            v3 = float3(v1.x, centre.z, v1.z);
            
            n0 = !condZ ? input[i].normal : input[next_i].normal;
            n1 = !condZ ? input[next_i].normal : input[i].normal;
            n2 = n3 = float3(1, 0, 0);
        }
        else // bottom
        {
            
            v0 = !condX ? positions[i] : positions[next_i];
            v1 = !condX ? positions[next_i] : positions[i];
            v2 = float3(v0.x, centre.z, v0.z);
            v3 = float3(v1.x, centre.z, v1.z);
            
            n0 = !condX ? input[i].normal : input[next_i].normal;
            n1 = !condX ? input[next_i].normal : input[i].normal;
            n2 = n3 = float3(0, 0, 1);
        }
        
        GSInput tri1[3] =
        {
            { v0, n0 },
            { v3, n3 },
            { v2, n2 }
        };
        
        for (int j = 0; j < 3; j++)
        {
            float4 worldPos = mul(modelMatrix, float4(tri1[j].position, 1.0f));
            o.view = normalize(camPos - worldPos.xyz);
            o.view = worldPos.xyz;
            o.normal = tri1[j].normal;
            o.position = mul(projMatrix, mul(viewMatrix, worldPos));
            outputStream.Append(o);
        }
        
        outputStream.RestartStrip();
        
        GSInput tri2[3] =
        {
            { v0, n0 },
            { v1, n1 },
            { v3, n3 }
        };
        
        for (int j = 0; j < 3; j++)
        {
            float4 worldPos = mul(modelMatrix, float4(tri2[j].position, 1.0f));
            o.view = normalize(camPos - worldPos.xyz);
            o.view = worldPos.xyz;
            o.normal = tri2[j].normal;
            o.position = mul(projMatrix, mul(viewMatrix, worldPos));
            outputStream.Append(o);
        }
        
        outputStream.RestartStrip();
        
    }
    
    for (int j = 0; j < 3; j++) {
        float4 worldPos = mul(modelMatrix, float4(positions[j], 1.0f));
        o.view = normalize(camPos - worldPos.xyz);
        o.view = worldPos.xyz;
        o.normal = input[j].normal;
        o.position = mul(projMatrix, mul(viewMatrix, worldPos));
        outputStream.Append(o);
    }
         
    outputStream.RestartStrip();
}