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

struct VSInput {
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct PSInput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

PSInput mainVS(VSInput input) {
    PSInput output;
    float4 worldPos = mul(float4(input.position, 1.0f), Model);
    float4 viewPos = mul(worldPos, View);
    output.position = mul(viewPos, Projection);
    output.texCoord = input.texCoord;
    return output;
}