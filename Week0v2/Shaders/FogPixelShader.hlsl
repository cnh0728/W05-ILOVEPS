cbuffer CameraConstant : register(b0){
    float3 CameraPos;
    float Padding;
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
    
    // 3. 포그 효과 (수정된 공식)
    if (bIsFogOn)  
    {
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
                // litColor = lerp(litColor, fogLightInfluence, fogFactor * 0.5); // 50% 블렌딩
            }
        }
    }  

    return float4(litColor, 1.0);  
}