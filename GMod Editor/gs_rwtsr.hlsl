struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct GSInput
{
    float4 position : SV_POSITION;
    float2 patchUV : UV;
    float2 uv : TEXCOORD;
};

bool isGridEdge(float2 uvA, float2 uvB)
{
    return (abs(uvA.x - uvB.x) < 1e-6f || abs(uvA.y - uvB.y) < 1e-6f);
}

[maxvertexcount(6)] 
void main(triangle GSInput input[3], inout LineStream<PSInput> outputStream)
{
    float2 uv0 = input[0].patchUV;
    float2 uv1 = input[1].patchUV;
    float2 uv2 = input[2].patchUV;

    PSInput o;
    if (isGridEdge(uv0, uv1))
    {
        o.position = input[0].position;
        o.uv = input[0].uv;
        outputStream.Append(o);
        
        o.position = input[1].position;
        o.uv = input[1].uv;
        outputStream.Append(o);
        
        outputStream.RestartStrip();
    }
    if (isGridEdge(uv1, uv2))
    {
        o.position = input[1].position;
        o.uv = input[1].uv;
        outputStream.Append(o);
        
        o.position = input[2].position;
        o.uv = input[2].uv;
        outputStream.Append(o);
        
        outputStream.RestartStrip();
    }
    if (isGridEdge(uv2, uv0))
    {
        o.position = input[2].position;
        o.uv = input[2].uv;
        outputStream.Append(o);
        
        o.position = input[0].position;
        o.uv = input[0].uv;
        outputStream.Append(o);
        
        outputStream.RestartStrip();
    }
}