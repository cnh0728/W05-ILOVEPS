Texture2D<float> DepthTexture : register(t0); // SRV로부터 샘플링
SamplerState Sampler : register(s0);

struct PS_INPUT {
    float4 Position : SV_POSITION;   // 화면 좌표
    float2 TexCoord : TEXCOORD; // 텍스처 좌표
};

struct PS_OUTPUT
{
    float4 Color : SV_TARGET; // 픽셀 셰이더 출력
};

PS_OUTPUT main(PS_INPUT input)
{
    PS_OUTPUT output;

    float depthValue = DepthTexture.Sample(Sampler, input.TexCoord); // 깊이값 샘플링 (0~1 범위)
    if (depthValue == 1.0f)
    {
        output.Color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        float powValue = pow(depthValue, 1000.0f); // 깊이값을 제곱하여 비선형 변환

        //// 색상 변환 (파랑 -> 초록 -> 빨강)
        //float3 color;
        //if (powValue < 0.5) {
        //    color = lerp(float3(0, 0, 1), float3(0, 1, 0), powValue * 2); // 파랑 -> 초록
        //}
        //else {
        //    color = lerp(float3(0, 1, 0), float3(1, 0, 0), (powValue - 0.5) * 2); // 초록 -> 빨강
        //}

        //output.Color = float4(color.x, color.y, color.z, 1.0f); // Grayscale 출력
        output.Color = float4(powValue, powValue, powValue, 1.0f); // Grayscale 출력
    }

    return output;
}