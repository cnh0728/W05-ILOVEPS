// StaticMeshFogVertexShader.hlsl

cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 MVP;
    row_major float4x4 MInverseTranspose;
    row_major float4x4 ModelMatrix;
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
    output.position = mul(input.position, MVP);
    output.color = isSelected ? input.color * 0.5 : input.color;

    float normalLen = length(input.normal);
    output.normalFlag = (normalLen < 0.001) ? 0.0 : 1.0;
    output.normal = mul(input.normal, MInverseTranspose);

    output.texcoord = input.texcoord;
    float4 worldPos = mul(input.position, ModelMatrix);
    output.worldPos = mul(input.position, MInverseTranspose).xyz;
    output.worldPos = worldPos.xyz;
    return output;
}
