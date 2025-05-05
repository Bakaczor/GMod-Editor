struct VSInput
{
    float3 position : POSITION0;
    float3 coefficient : POSITION1;
};

struct HSInput
{
    float3 wPosition : WORLDPOS0;
    float3 wCoefficient : WORLDPOS1;
};

HSInput main(VSInput input)
{
    HSInput output;
    output.wPosition = input.position;
    output.wCoefficient = input.coefficient;
    return output;
}