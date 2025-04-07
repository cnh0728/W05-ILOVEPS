// DebugSceneDepthPixelShader.hlsl

Texture2D SceneDepth : register(t0);
SamplerState LinearSampler : register(s0);
struct VSInput {
    float2 pos : POSITION;      // [-1, 1] 범위의 정점
    float2 uv  : TEXCOORD;
};

struct PSInput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

PSInput mainVS(VSInput input) {
    PSInput output;
    output.position = float4(input.pos, 0.0f, 1.0f);
    output.texcoord = input.uv;
    return output;
}

float4 mainPS(PSInput input) : SV_Target {
    //return SceneDepth.Sample(LinearSampler, input.texcoord);
    float depth = SceneDepth.Sample(LinearSampler, input.texcoord).r;
    float depthMin = 0.980;
    float depthMax = 0.995;
    float remapped = saturate((depth - depthMin) / (depthMax - depthMin));
    float visualized = pow(remapped, 2);
    return float4(visualized.xxx, 1.0f);
    float visualizedDepth = pow(saturate(depth), 0.3);
    return float4(visualizedDepth.xxx, 1.0f);
}
