struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct HSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

HSInput main(VSInput input)
{
    HSInput output;
    output.position = input.position;
    output.normal = input.normal;
    return output;
}