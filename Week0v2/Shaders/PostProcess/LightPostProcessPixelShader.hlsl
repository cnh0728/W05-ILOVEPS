cbuffer LightParams : register(b1)
{
    float3 LightPosition;
    float LightRadius;
    float3 LightColor;
    float LightIntensity;
};

cbuffer FogConstants : register(b6) // 기존 버퍼 위치 통일
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

    // NDC → Clip
    float2 ndc = input.TexCoord * 2.0f - 1.0f;
    float4 clipPos = float4(ndc, depth, 1.0f);
    clipPos.y = -clipPos.y; // Y축 반전
    // 복원된 World Position
    float4 worldPos = mul(clipPos, InvViewProj);
    worldPos /= worldPos.w;

    float3 toLight = LightPosition - worldPos.xyz;
    float dist = length(toLight);
    float attenuation = saturate(1.0 - dist / LightRadius);
    float3 light = LightColor * LightIntensity * attenuation;
    //float3 light = LightColor * 100.0f;
    //return float4(1, 0, 0, 1);
    return float4(light, attenuation); // 알파로 감쇠율 전달
}
