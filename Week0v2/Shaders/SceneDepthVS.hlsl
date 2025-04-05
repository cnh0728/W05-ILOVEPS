struct VS_INPUT {
    float4 Position : POSITION; // 정점 위치
    float2 TexCoord : TEXCOORD; // 텍스처 좌표
};

struct PS_INPUT {
    float4 Position : SV_POSITION;   // 화면 좌표
    float2 TexCoord : TEXCOORD; // 텍스처 좌표
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;
    output.Position = input.Position; // 화면 좌표 그대로 출력
    output.TexCoord = input.TexCoord; // 텍스처 좌표 전달
    return output;
}