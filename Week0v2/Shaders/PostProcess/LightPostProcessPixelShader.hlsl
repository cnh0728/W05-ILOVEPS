// LightPostProcessPixelShader.hlsl

#define MAX_LIGHT_COUNT 2000

cbuffer LightParams : register(b1)
{
    float4 LightPositionRadius[MAX_LIGHT_COUNT]; // xyz + radius
    float4 LightColorIntensity[MAX_LIGHT_COUNT]; // rgb + intensity
    int LightCount; // 실제 라이트 개수
    float3 Padding; // 16바이트 정렬을 위한 패딩
};

cbuffer FogConstants : register(b6)
{
    row_major float4x4 InvViewProj;
    float3 CameraWorldPos;
    float LinearStart;
    float LinearEnd;
    float HeightStart;
    float HeightEnd;
    float FogPad0;
    float4 FogColor;
};

Texture2D SceneDepth : register(t0);
SamplerState LinearSampler : register(s0);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

float4 mainPS(PS_INPUT input) : SV_Target
{
    float depth = SceneDepth.Sample(LinearSampler, input.TexCoord).r;

    float2 ndc = input.TexCoord * 2.0f - 1.0f;
    float4 clipPos = float4(ndc, depth, 1.0f);
    clipPos.y = -clipPos.y; // Y축 반전 (좌표계 보정)

    float4 worldPos = mul(clipPos, InvViewProj);
    worldPos /= worldPos.w;


    float3 totalLight = float3(0, 0, 0);
    float totalAttenuation = 0.0f;

    for (int i = 0; i < MAX_LIGHT_COUNT; ++i)
    {
        if (i >= LightCount)
            break;

        float3 lightPos = LightPositionRadius[i].xyz;
        float radius = LightPositionRadius[i].w;
        float3 lightColor = LightColorIntensity[i].rgb;
        float intensity = LightColorIntensity[i].a;

        float3 toLight = lightPos - worldPos.xyz;
        float dist = length(toLight);
        float attenuation = saturate(1.0 - dist / radius);

        totalLight += lightColor * intensity * attenuation;
        totalAttenuation += attenuation;
    }

    return float4(totalLight, saturate(totalAttenuation));
}