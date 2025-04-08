Texture2D SceneColor : register(t0);
Texture2D SceneDepth : register(t1);
SamplerState BilinearSampler : register(s0);
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

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 WorldPosition : TEXCOORD1;
};
float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float4 sceneColor = SceneColor.Sample(BilinearSampler, input.TexCoord);
    float depth = SceneDepth.Sample(BilinearSampler, input.TexCoord).r;

    // NDC 좌표 (-1 ~ +1)
    float2 ndcXY = input.TexCoord * 2.0f - 1.0f;
    float4 clipPos = float4(ndcXY, depth, 1.0f);

    // World Position 복원
    float4 worldPos = mul(clipPos, InvViewProj);
    worldPos /= worldPos.w;

    float distance = length(worldPos.xyz - CameraWorldPos.xyz);

    float fogFactor = saturate((distance - LinearStart) / (LinearEnd - LinearStart));
    fogFactor = pow(fogFactor, 1.5);

    float3 fog = FogColor.rgb * fogFactor;
    return float4(fog, fogFactor); // alpha도 fogFactor로 넘겨주면 후처리에 도움
    //SceneColor 포함된 버전
    float3 finalColor = lerp(sceneColor.rgb, FogColor.rgb, fogFactor);
    return float4(finalColor, sceneColor.a);
}
/*float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float4 sceneColor = SceneColor.Sample(BilinearSampler, input.TexCoord);
    float distance = length(input.WorldPosition - CameraWorldPos);
    float fogFactor = saturate((distance - LinearStart) / (LinearEnd - LinearStart));
    fogFactor = pow(fogFactor, 1.5);
    float3 finalColor = lerp(sceneColor.rgb, FogColor.rgb, fogFactor);
    return float4(finalColor, sceneColor.a);
}*/

float LinearizeDepth(float depth)
{
    float zNear = 0.1f;
    float zFar = 1000.0f;
    return zNear * zFar / (zFar + depth * (zNear - zFar));
}
/*
float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float4 sceneColor = SceneColor.Sample(BilinearSampler, input.TexCoord);
    float depth = SceneDepth.Sample(BilinearSampler, input.TexCoord).r;
    float linearDepth = LinearizeDepth(depth);

    float fogFactor = saturate((linearDepth - LinearStart) / (LinearEnd - LinearStart));
    fogFactor = pow(fogFactor, 1.5); // 곡선으로 부드럽게

    float3 finalColor = lerp(sceneColor.rgb, FogColor.rgb, fogFactor);
    return float4(finalColor, sceneColor.a);
}
*/