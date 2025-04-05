struct DepthVSINPUT
{
    float3 Dposition : POSITION; // 버텍스 위치
    float2 Dtexcoord : TEXCOORD;
};

struct DepthPSInput {
    float4 Dposition : SV_POSITION;
    float2 Dtexcoord : TEXCOORD;
};

DepthPSInput mainVS(DepthVSINPUT input) {
    DepthPSInput output;
    output.Dposition = float4(input.Dposition, 1.0); // NDC 좌표 그대로 출력
    output.Dtexcoord = input.Dtexcoord;             // 텍스처 좌표 전달
    return output;
}