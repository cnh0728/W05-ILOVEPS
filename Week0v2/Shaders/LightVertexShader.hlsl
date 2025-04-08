struct VSINPUT
{
    float3 Position : POSITION; // 버텍스 위치
    float2 Texcoord : TEXCOORD;
};

struct PSInput {
    float4 Position : SV_POSITION;
    float2 Texcoord : TEXCOORD;
};

PSInput mainVS(VSINPUT input) {
    PSInput output;
    output.Position = float4(input.Position, 1.0); // NDC 좌표 그대로 출력
    output.Texcoord = input.Texcoord;             // 텍스처 좌표 전달
    return output;
}