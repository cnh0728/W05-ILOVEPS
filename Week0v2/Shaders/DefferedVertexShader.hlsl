// MatrixBuffer: 변환 행렬 관리
cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 M;
    row_major float4x4 VP;
    row_major float4x4 MInverse;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
};

struct VS_INPUT
{
    float4 position : POSITION; // 버텍스 위치
    float4 color : COLOR; // 버텍스 색상
    float3 normal : NORMAL; // 버텍스 노멀
    float2 texcoord : TEXCOORD;
    int materialIndex : MATERIAL_INDEX;
};

struct PS_INPUT
{
    float4 position : SV_POSITION; // 변환된 화면 좌표
    float3 worldPos : TEXCOORD0;
    float4 color : COLOR; // 전달할 색상
    float3 normal : NORMAL; // 정규화된 노멀 벡터
    bool normalFlag : TEXCOORD1; // 노멀 유효성 플래그 (1.0: 유효, 0.0: 무효)
    float2 texcoord : TEXCOORD2;
    int materialIndex : MATERIAL_INDEX;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;

    float4 worldPos = mul(input.position, M);
    output.worldPos = worldPos.xyz;
    output.position = mul(worldPos, VP);
    // 위치 변환
    output.color = input.color;
    
    if (isSelected)
        output.color *= 0.5;

    // 입력 normal 값의 길이 확인
    if (length(input.normal) > 0.001)
    {
        output.normal = normalize(mul(input.normal, MInverse));
        output.normalFlag = true; 
    }
    else
    {
        output.normal = float3(0, 0, 0);
        output.normalFlag = false;
    }
    
    output.texcoord = input.texcoord;
    output.materialIndex = input.materialIndex;
    
    return output;
}