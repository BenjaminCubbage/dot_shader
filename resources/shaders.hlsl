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
    float2 vec1 = { 2.0f, 1.0f };
    float2 vec2 = { input.pos.x * 0.0002f, input.pos.y * 0.0002f };

    float dotp = dot(vec1, vec2);
    float4 output_clr = { dotp, 0, 0, 0 };

    int ix = input.pos.x;
    int iy = input.pos.y;

    output_clr.b += ((ix & 2) ^ (iy & 2)) * 0.03f;
    return output_clr;
}