Texture2D InputTexture : register(t0);
SamplerState InputSampler : register(s0);

struct VS_INPUT
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    output.Position = float4(input.Position, 1.0f);
    output.TexCoord = input.TexCoord;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    return InputTexture.Sample(InputSampler, input.TexCoord);
}
