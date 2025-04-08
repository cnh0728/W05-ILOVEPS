cbuffer FullScreenConstant : register(b0)
{
    int bIsDepth;
};

cbuffer CameraConstant : register(b1){
    float3 CameraPos;
    float Padding;
}

cbuffer FogConstant : register(b2){
    float3 FogColor;
    float FogDensity;
    float FogStart;
    float FogEnd;
    int bIsFogOn;
    float FogZPosition;
    float FogBaseHeight;
    float HeightFallOff;
    int bIsHeightFog;
    float Padding1;
}

cbuffer PointLightsConstant : register(b3){
    float4 PointLightPositions[50];  // xyz: 위치, w: Radius
    float4 PointLightColors[50];     // rgb: 색상, a: Intensity
    float4 PointLightFallOff[50];    // x: RadiusFallOff 나머지 Pad
}

Texture2D PositionTexture : register(t0); // Can be Depth
Texture2D NormalTexture : register(t1); 
Texture2D AlbedoTexture : register(t2); 
Texture2D MaterialTexture : register(t3); 

SamplerState TextureSampler : register(s0);

struct PSInput {
    float4 Position : SV_POSITION;
    float2 Texcoord : TEXCOORD;
};

float4 mainPS(PSInput input) : SV_Target {
    if (bIsDepth)
    {
        float depth = 1 - PositionTexture.Sample(TextureSampler, input.Texcoord).r; //depth 모드면 depthstencil resourceview가 들어온다.
        depth = pow(depth, 0.8);
        return float4(depth, depth, depth, 1.0); // 흑백으로 출력
    }
    
    // 1. G-Buffer 샘플링  
    float3 worldPos = PositionTexture.Sample(TextureSampler, input.Texcoord).rgb;  
    float3 normal = normalize((NormalTexture.Sample(TextureSampler, input.Texcoord).rgb - 0.5) * 2);
    float3 albedo = AlbedoTexture.Sample(TextureSampler, input.Texcoord).rgb;  
    float3 material = MaterialTexture.Sample(TextureSampler, input.Texcoord).rgb;  

    // 2. 라이팅 계산  
    float3 lightDir = normalize(float3(1, 1, -1));  
    float diff = max(dot(normal, lightDir), 0.0);  
    float3 litColor = albedo * diff * float3(1,1,1); 

    // mainPS 함수 내 라이팅 계산 부분에 추가  
    float3 pointLightAccumulation = float3(0, 0, 0);  

    for (int i = 0; i < 50; ++i)  
    {  
        // 1. 유효 라이트 필터링  
        if (PointLightFallOff[i].y < 0.5f) break;  // 유효성 플래그  

        // 2. 거리/방향 계산  
        float3 lightDir = PointLightPositions[i].xyz - worldPos;  
        float dist = length(lightDir);  
        lightDir = normalize(lightDir);  

        // 3. 감쇠 계산 (물리 기반)  
        float radius = PointLightPositions[i].w;  
        float fallOff = PointLightFallOff[i].x;  
        float attenuation = saturate(1.0 - dist / radius);  
        attenuation *= exp(-dist * fallOff);  

        // 4. 확산광 계산  
        float diff = max(dot(normal, lightDir), 0.0);  
        float3 lightColor = PointLightColors[i].rgb * PointLightColors[i].a;  
        pointLightAccumulation += lightColor * diff * attenuation;  
    }  

    // 5. 최종 라이트 합성  
    litColor += albedo * pointLightAccumulation;  
    
    // 3. 포그 효과 (수정된 공식)
    if (bIsFogOn)  
    {
        // 4. 포그 계산  
        // 거리 기반 포그  
        float dist = distance(worldPos, CameraPos);  
        float distanceFog = 1.0 - exp(FogDensity * -dist);  

        bool bIsInFog = true;
        
        // 높이 기반 포그  
        float heightFog = 0.0;
        float fogFactor = distanceFog;
        if (bIsHeightFog)  
        {  
            // 기준 높이 아래로 갈수록 포그 증가  
            float heightDelta = (FogZPosition + FogBaseHeight) - worldPos.z;  
            heightFog = exp(-HeightFallOff * max(heightDelta, 0.0));  
            heightFog = 1.0 - heightFog;

            //HeightFog안이라면 
            if (heightDelta > CameraPos.z)
            {
                fogFactor = max(distanceFog, heightFog);
            }else
            {
                //밖이면 배경처리 안해주고 싶은데
                //fog 깔린곳만 색칠해주고 싶음
                bIsInFog = false;
            }
        }

        // 포그 혼합 (거리 + 높이)  

        if (bIsInFog)
        {
            if (all(worldPos == float3(0.025, 0.025, 0.025))) //배경(오브젝트가 아닐때)
            {
                litColor = FogColor;  
            }else
            {
                litColor = lerp(litColor, FogColor, fogFactor);  
            }
        }
    }  

    return float4(litColor, 1.0);  
}