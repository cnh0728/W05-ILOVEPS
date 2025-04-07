// StaticMeshFogPixelShader.hlsl

Texture2D Textures : register(t0);
SamplerState Sampler : register(s0);

cbuffer MaterialConstants : register(b1)
{
    float3 DiffuseColor;
    float TransparencyScalar;
    float3 AmbientColor;
    float DensityScalar;
    float3 SpecularColor;
    float SpecularScalar;
    float3 EmissiveColor;
    float MaterialPad0;
};

cbuffer LightingConstants : register(b2)
{
    float3 LightDirection;
    float LightPad0;
    float3 LightColor;
    float LightPad1;
    float AmbientFactor;
    float3 LightPad2;
};

cbuffer FlagConstants : register(b3)
{
    bool IsLit;
    float3 flagPad0;
};

cbuffer TextureConstants : register(b5)
{
    float2 UVOffset;
    float2 TexturePad0;
};

cbuffer FogConstants : register(b6)
{
    float3 CameraWorldPos;
    float LinearStart;
    float LinearEnd;
    float HeightStart;
    float HeightEnd;
    float FogPad0;
    float4 FogColor;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD1;
    int materialIndex : MATERIAL_INDEX;
    float3 worldPos : TEXCOORD2;
    float normalFlag : TEXCOORD0;
};

float4 mainPS(PS_INPUT input) : SV_Target
{
    float3 texColor = Textures.Sample(Sampler, input.texcoord + UVOffset);
    float3 baseColor = (texColor.g == 0) ? DiffuseColor : texColor + DiffuseColor;
    
    if (IsLit && input.normalFlag > 0.5)
    {
        float3 N = normalize(input.normal);
        float3 L = normalize(LightDirection);
        float3 V = float3(0, 0, 1);
        float3 H = normalize(L + V);

        float diffuse = saturate(dot(N, L));
        float specular = pow(saturate(dot(N, H)), SpecularScalar * 32) * SpecularScalar;

        float3 ambient = AmbientColor * AmbientFactor;
        float3 lighting = ambient + (diffuse * LightColor * baseColor) + (specular * SpecularColor * LightColor);
        baseColor = lighting + EmissiveColor;
    }
    else
    {
        baseColor += EmissiveColor;
    }
    //baseColor =  float4(0,0,1,1);
    // ✅ Apply fog based on distance
    // 거리 기반
    float distance = length(input.worldPos - CameraWorldPos);
    float fogFactorDistance = saturate((distance - LinearStart) / (LinearEnd - LinearStart));

    float fogFactorHeight = saturate((HeightEnd - input.worldPos.z) / (HeightEnd - HeightStart));

    // 혼합 방식 (둘 다 적용)
    float fogFactor = max(fogFactorDistance, fogFactorHeight);
    //fogFactor=fogFactorDistance;
    // 최종 색상 보간
    //baseColor = lerp(baseColor, FogColor.rgb, fogFactor);
    
    return float4(baseColor, TransparencyScalar);
}
