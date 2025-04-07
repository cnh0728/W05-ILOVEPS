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
        float linearDepth = (0.1f * 1000.0f) / (1000.0f - depthValue * (1000.0f - 0.1f)); // 깊이값을 선형 깊이로 변환

        float normalizedDepth = (linearDepth - 0.1f) / (1000.0f - 0.1f); // 깊이값을 정규화 (0~1 범위)

        output.Color = float4(normalizedDepth, normalizedDepth, normalizedDepth, 1.0f);
    }

    return output;
}