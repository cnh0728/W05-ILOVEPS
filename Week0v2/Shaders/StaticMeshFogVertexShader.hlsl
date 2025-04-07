// StaticMeshFogVertexShader.hlsl

cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 Model;
    row_major float4x4 View;
    row_major float4x4 Projection;
    row_major float4x4 ModelMatrixInverseTranspose;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
};


struct VS_INPUT
{
    float4 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
    int materialIndex : MATERIAL_INDEX;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD1;
    int materialIndex : MATERIAL_INDEX;
    float3 worldPos : TEXCOORD2; // For fog
    float normalFlag : TEXCOORD0;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    output.materialIndex = input.materialIndex;

    // 월드 공간 위치
    float4 worldPos = mul(input.position, Model);
    output.worldPos = worldPos.xyz;

    // 뷰-프로젝션 변환
    float4 viewPos = mul(worldPos, View);
    output.position = mul(viewPos, Projection);

    // UUID 색상 처리
    output.color = isSelected ? input.color * 0.5 : input.color;

    // 노멀 변환 및 유효성 검사
    float normalLen = length(input.normal);
    output.normalFlag = (normalLen < 0.001) ? 0.0 : 1.0;
    output.normal = mul(input.normal, ModelMatrixInverseTranspose);

    output.texcoord = input.texcoord;
    return output;
}