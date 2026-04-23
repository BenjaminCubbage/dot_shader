struct VSInput
{
    float3 pos : POSITION;
    float4 clr : COLOR;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float4 clr : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.pos = float4(input.pos, 1.0f);
    output.clr = input.clr;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float2 vec1 = {
        2.0f,
        1.0f
    };

    float2 vec2 = float2(
        input.pos.x * 0.0002f,
        input.pos.y * 0.0002f);

    float dotp = (vec1.x * vec2.x) + (vec1.y * vec2.y);

    float4 output_clr = {
        dotp,
        0,
        0,
        0
    };

    return output_clr;
}