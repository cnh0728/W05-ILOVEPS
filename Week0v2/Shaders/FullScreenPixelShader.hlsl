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
    float Padding1;
    float FogDensity;
    float FogStart;
    float FogEnd;
    int bIsFogOn;
};

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
    float3 normal = normalize(NormalTexture.Sample(TextureSampler, input.Texcoord).rgb);  
    float3 albedo = AlbedoTexture.Sample(TextureSampler, input.Texcoord).rgb;  
    float3 material = MaterialTexture.Sample(TextureSampler, input.Texcoord).rgb;  

    // 2. 라이팅 계산  
    float3 lightDir = normalize(float3(1, 1, -1));  
    float diff = max(dot(normal, lightDir), 0.0);  
    float3 litColor = albedo * diff * float3(1,1,1);  

    // 3. 포그 효과 (수정된 공식)
    if (bIsFogOn)  
    {
        if (all(worldPos == float3(0.025, 0.025, 0.025))) //배경(오브젝트가 아닐때)
        {
            litColor = FogColor;  
        }else //오브젝트일 때
        {
            float3 cameraToWorld = worldPos - CameraPos;  
            float dist = length(cameraToWorld);  
            // 카메라-월드 거리 (월드 좌표계 기준)

            // 지수 감쇠 + 선형 범위 제한  
            float fogFactor = 1.0 - exp(-dist * FogDensity);

            // 포그 색상 혼합  
            litColor = lerp(litColor, FogColor, fogFactor);  
        }
    }  

    return float4(litColor, 1.0);  
}