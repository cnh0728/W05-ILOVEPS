Texture2D Input0 : register(t0); // SceneColor
Texture2D Input1 : register(t1); // Fog
Texture2D Input2 : register(t2); // Light (optional)
// 필요에 따라 추가 가능

SamplerState LinearSampler : register(s0);

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

// 기본 fullscreen vertex shader
PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    output.Position = float4(input.Position, 1.0f);
    output.TexCoord = input.TexCoord;
    return output;
}

// 후처리 합성 픽셀 셰이더 (최대 3개 텍스처 합성 예시)
float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float4 scene = Input0.Sample(LinearSampler, input.TexCoord);
    float4 fog = Input1.Sample(LinearSampler, input.TexCoord); // RGB: FogColor, A: fogFactor
    float4 light = Input2.Sample(LinearSampler, input.TexCoord);

    // Scene + Fog 합성
    float3 blended = lerp(scene.rgb, fog.rgb, fog.a);

    // Lighting 추가
    blended += light.rgb * 0.5f;

    return float4(blended, 1.0f);
}
