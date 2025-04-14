struct VSInput
{
    float3 position : POSITION;
};

struct HSInput
{
    float3 wPosition : WORLDPOS;
};

HSInput main(VSInput input)
{
    HSInput output;
    output.wPosition = input.position;
    return output;
}