cbuffer FullScreenConstant : register(b0)
{
    int bIsDepth;
};

cbuffer CameraConstant : register(b1){
    float3 CameraPos;
    float Padding;
    row_major float4x4 InverseVMatrix;
    row_major float4x4 InversePMatrix;
}

Texture2D PositionTexture : register(t0); // Can be Depth
Texture2D NormalTexture : register(t1); 
Texture2D AlbedoTexture : register(t2); 
Texture2D MaterialTexture : register(t3);
Texture2D SpecularTexture : register(t4);
Texture2D EmissiveTexture : register(t5);
Texture2D AmbientTexture : register(t6);
Texture2D FogTexture : register(t7);

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
    float3 albedo = AlbedoTexture.Sample(TextureSampler, input.Texcoord).rgb; //diffuse가 결합된 상태
    float3 material = MaterialTexture.Sample(TextureSampler, input.Texcoord).rgb;
    float4 specular = SpecularTexture.Sample(TextureSampler, input.Texcoord); //w에 scalar값이 있음
    float3 emissive = EmissiveTexture.Sample(TextureSampler, input.Texcoord).rgb;
    float3 ambient = AmbientTexture.Sample(TextureSampler, input.Texcoord).rgb;
    float3 fog = FogTexture.Sample(TextureSampler, input.Texcoord).rgb;
    
    float3 litColor = fog;  

    //여기서 포스트프로세스 진행

    return float4(litColor, 1.0);  
}