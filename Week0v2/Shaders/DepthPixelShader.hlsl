Texture2D<float> DTextures : register(t0);
SamplerState DSampler : register(s0);

struct DepthPSInput {
    float4 DepthPosition : SV_POSITION;
    float2 DepthTexcoord : TEXCOORD;
};

float LogarithmicCurve(float x, float k) {  
    return log(1.0 + k * x) / log(1.0 + k);  
}  

float4 mainPS(DepthPSInput input) : SV_Target {
    float depth = 1 - DTextures.Sample(DSampler, input.DepthTexcoord);
    depth = LogarithmicCurve(depth, 2);
    // depth = pow(depth, 0.8);
    return float4(depth, depth, depth, 1.0); // 흑백으로 출력
}