cbuffer CameraConstant : register(b0){
    float3 CameraPos;
    float Padding;
}

cbuffer PointLightsConstant : register(b1){
    float4 PointLightPositions[100];  // xyz: 위치, w: Radius
    float4 PointLightColors[100];     // rgb: 색상, a: Intensity
    float4 PointLightFallOff[100];    // x: RadiusFallOff 나머지 Pad
}


cbuffer LightingConstants : register(b2)
{
    float3 LightDirection; // 조명 방향 (단위 벡터; 빛이 들어오는 방향의 반대 사용)
    float LightPad0; // 16바이트 정렬용 패딩
    float3 LightColor; // 조명 색상 (예: (1, 1, 1))
    float LightPad1; // 16바이트 정렬용 패딩
    float AmbientFactor; // ambient 계수 (예: 0.1)
    float3 LightPad2; // 16바이트 정렬 맞춤 추가 패딩
};

Texture2D PositionTexture : register(t0); // Can be Depth
Texture2D NormalTexture : register(t1); 
Texture2D AlbedoTexture : register(t2); 
Texture2D MaterialTexture : register(t3);
Texture2D SpecularTexture : register(t4);
Texture2D EmissiveTexture : register(t5);
Texture2D AmbientTexture : register(t6);

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

    // 2. 라이팅 계산  
    float3 lightDir = normalize(LightDirection);  
    float diff = max(dot(normal, lightDir), 0.0);  
    float3 litColor = albedo * diff; //litcolor에 최종 컬러를 전부 담기

    if (all(normal != float3(0,0,0))) //배경이 아닐때
    {
        float3 N = normalize(normal);
        float3 L = normalize(LightDirection);
        float3 V = normalize(float3(0, 0, 1) - worldPos);
        
        // 기본 디퓨즈 계산
        float diffuse = saturate(dot(N, L));
        
        // 스페큘러 계산 (간단한 Blinn-Phong)
        float3 H = normalize(L + V);
        float specularScalar = pow(saturate(dot(N, H)), specular.w * 32) * specular.w;
        
        // 라이느 누적
        ambient = ambient * AmbientFactor;
        float3 diffuseLight = diffuse * LightColor;
        float3 specularLight = specularScalar * specular * LightColor;

        //Albedo에 라이팅 적용
        litColor = ambient + (diffuseLight * albedo) + specularLight + emissive;
    }
    
    // mainPS 함수 내 라이팅 계산 부분에 추가  
    float3 pointLightAccumulation = float3(0, 0, 0);  

    if (all(worldPos != float3(0.025, 0.025, 0.025))) //배경(오브젝트가 아닐때)
    {
        for (int i = 0; i < 100; ++i)  
        {  
            // 1. 유효 라이트 필터링  
            if (PointLightFallOff[i].y < 0.5f) break;  // 유효성 플래그  

            // 2. 거리/방향 계산  
            lightDir = PointLightPositions[i].xyz - worldPos;  
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
    }

    // 5. 최종 라이트 합성  
    litColor += albedo * pointLightAccumulation;  

    return float4(litColor, 1.0);  
}