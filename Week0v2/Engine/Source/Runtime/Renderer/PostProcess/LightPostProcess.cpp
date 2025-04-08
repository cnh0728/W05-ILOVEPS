#include "LightPostProcess.h"

#include "FogPostProcess.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Renderer/Renderer.h"
#include "Renderer/Utils/FullscreenQuad.h"
bool FLightPostProcess::Initialize(FRenderer* inRenderer)
{
    Renderer = inRenderer;
    LoadShaders();
    CreateOutputRenderTarget();
    LightConstantBuffer =  Renderer->GetResourceManager().CreateConstantBuffer(sizeof(FLightParams));
    LightParams =
    {
    FVector(0, 0, 0),  // 테스트용 광원 위치
    5.0f,              // Radius
    FVector(1.0f, 0.0f, 0.0f), // Red
    1.5f                // Intensity
    };
    return true;
}

void FLightPostProcess::LoadShaders()
{
    D3D11_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    Renderer->GetShaderManager().CreateVertexShader(
        L"Shaders/PostProcess/LightPostProcessVertexShader.hlsl",
        "mainVS",
        VertexShader,
        inputLayout,
        ARRAYSIZE(inputLayout),
        &InputLayout
    );

    Renderer->GetShaderManager().CreatePixelShader(
        L"Shaders/PostProcess/LightPostProcessPixelShader.hlsl",
        "mainPS",
        PixelShader
    );
}

void FLightPostProcess::Render(ID3D11DeviceContext* context, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    Renderer->GetConstantBufferUpdater().UpdateLightConstant(LightConstantBuffer, LightParams);
    context->OMSetRenderTargets(1, &LightRTV, nullptr);
    context->IASetInputLayout(InputLayout);
    context->VSSetShader(VertexShader, nullptr, 0);
    context->PSSetShader(PixelShader, nullptr, 0);

    ID3D11ShaderResourceView* depthSRV = Renderer->Graphics->SceneDepthSRV;
    context->PSSetShaderResources(0, 1, &depthSRV);

    context->PSSetConstantBuffers(1, 1, &LightConstantBuffer);
    context->PSSetConstantBuffers(6, 1, &Renderer->FogConstantBuffer);

    ID3D11SamplerState* sampler = Renderer->LinearSampler;
    context->PSSetSamplers(0, 1, &sampler);

    FullscreenQuad::Get()->Draw(context);
}
void FLightPostProcess::CreateOutputRenderTarget()
{
    Renderer->GetResourceManager().CreateRenderTarget(
        LightTexture,
        LightRTV,
        LightSRV,
        Renderer->Graphics->screenWidth,
        Renderer->Graphics->screenHeight,
        DXGI_FORMAT_R8G8B8A8_UNORM
    );
}

void FLightPostProcess::Resize()
{
    SAFE_RELEASE(LightTexture);
    SAFE_RELEASE(LightRTV);
    SAFE_RELEASE(LightSRV);

    CreateOutputRenderTarget();
}

void FLightPostProcess::Release()
{
    SAFE_RELEASE(VertexShader);
    SAFE_RELEASE(PixelShader);
    SAFE_RELEASE(InputLayout);

    SAFE_RELEASE(LightTexture);
    SAFE_RELEASE(LightRTV);
    SAFE_RELEASE(LightSRV);

    SAFE_RELEASE(LightConstantBuffer);
}
