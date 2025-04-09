cbuffer CameraConstant : register(b0){
    float3 CameraPos;
    float Padding;
    row_major float4x4 InverseVMatrix;
    row_major float4x4 InversePMatrix;
}

cbuffer FogConstant : register(b1){
    float3 FogColor;
    float FogDensity;
    float FogStart;
    float FogEnd;
    int bIsFogOn;
    float FogZPosition;
    float FogBaseHeight;
    float HeightFallOff;
    int bIsHeightFog;
    float ScatteringIntensity;  // 추가: 빛 산란 강도 [4]
    float LightShaftDensity;    // 추가: 광선 밀도 [4]
    float3 Padding1;
}

Texture2D PositionTexture : register(t0); // Can be Depth
Texture2D NormalTexture : register(t1); 
Texture2D AlbedoTexture : register(t2); 
Texture2D MaterialTexture : register(t3);
Texture2D SpecularTexture : register(t4);
Texture2D EmissiveTexture : register(t5);
Texture2D AmbientTexture : register(t6);
Texture2D LightTexture: register(t7);

SamplerState TextureSampler : register(s0);

struct PSInput {
    float4 Position : SV_POSITION;
    float2 Texcoord : TEXCOORD;
};

// 깊이 버퍼에서 월드 좌표 계산 함수
float3 ReconstructWorldPos(float2 uv)
{
    // 1. 클립 공간 좌표 계산
    float4 clipPos = float4(uv * 2.0 - 1.0, 1.0, 1.0);

    clipPos.y *= -1.0;
    
    // 2. 뷰/프로젝션 행렬 역변환 (엔진별 행렬 전달 필요)
    float4 worldPos = mul(clipPos, InversePMatrix);
    worldPos /= worldPos.w;
    worldPos = mul(worldPos, InverseVMatrix);
    
    return worldPos.xyz;
}

float4 mainPS(PSInput input) : SV_Target {

    // 1. G-Buffer 샘플링  
    float3 worldPos = PositionTexture.Sample(TextureSampler, input.Texcoord).rgb;  
    float3 normal = normalize((NormalTexture.Sample(TextureSampler, input.Texcoord).rgb - 0.5) * 2);
    float3 albedo = AlbedoTexture.Sample(TextureSampler, input.Texcoord).rgb; //diffuse가 결합된 상태
    float3 material = MaterialTexture.Sample(TextureSampler, input.Texcoord).rgb;
    float4 specular = SpecularTexture.Sample(TextureSampler, input.Texcoord); //w에 scalar값이 있음
    float3 emissive = EmissiveTexture.Sample(TextureSampler, input.Texcoord).rgb;
    float3 ambient = AmbientTexture.Sample(TextureSampler, input.Texcoord).rgb;
    float3 light = LightTexture.Sample(TextureSampler, input.Texcoord).rgb;

    float3 litColor = light;
    
    if (bIsFogOn)  
    {        
        // 포그 혼합 (거리 + 높이)          
        if (all(worldPos == float3(0.025, 0.025, 0.025))) //배경(오브젝트가 아닐때)
        {
            worldPos = ReconstructWorldPos(input.Texcoord);
        }

        float dist = distance(CameraPos, worldPos);
        // 거리 기반
        float fogRange = FogEnd - FogStart;
        float disFactor = saturate((dist - FogStart) / fogRange); // 0~1  50일떄 0

        float fogFactor = disFactor;
        
        if (bIsHeightFog)  
        {  
            float FogHeight = FogZPosition + FogBaseHeight;
        
            // 높이 기반 (지수 감쇠)
            float heightDiff = worldPos.z - FogHeight;
            float heightFactor = saturate(exp(-heightDiff * HeightFallOff)); // 0~1
            fogFactor = fogFactor * heightFactor * FogDensity; //factor가 클수록 fogcolor에 가까워짐
        }

        litColor = lerp(litColor, FogColor, fogFactor); 
    }  

    return float4(litColor, 1.0);  
}