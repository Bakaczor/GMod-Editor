struct VSInput
{
    float3 position : POSITION;
    float3 coefficient : COEFFICIENT;
};

struct HSInput
{
    float3 wPosition : WORLDPOS;
    float3 wCoefficient : WORLDCOEF;
};

HSInput main(VSInput input)
{
    HSInput output;
    output.wPosition = input.position;
    output.wCoefficient = input.coefficient;
    return output;
}