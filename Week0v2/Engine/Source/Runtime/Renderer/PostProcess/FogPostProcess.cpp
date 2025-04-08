#include "FogPostProcess.h"

#include "EditorEngine.h"
#include "LaunchEngineLoop.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Renderer/Renderer.h"
#include "Renderer/Utils/FullscreenQuad.h"
#include "UnrealEd/EditorViewportClient.h"

bool FFogPostProcess::Initialize(FRenderer* inRenderer)
{
    Renderer = inRenderer;

    // 셰이더 생성
    D3D11_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(FVector), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    Renderer->GetShaderManager().CreateVertexShader(
        L"Shaders/PostProcess/FogPostProcessVertexShader.hlsl",
        "mainVS",
        VertexShader,
        inputLayout,
        2,
        &InputLayout
    );

    Renderer->GetShaderManager().CreatePixelShader(
        L"Shaders/PostProcess/FogPostProcessPixelShader.hlsl",
        "mainPS",
        PixelShader
    );

    CreateOutputRenderTarget();

    /*D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(FFogParams);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    Renderer->Graphics->Device->CreateBuffer(&cbDesc, nullptr, &FogConstantBuffer);*/
    //CreateConstantBuffers();
    return true;
}

void FFogPostProcess::SetFogParams(const FFogParams& params)
{
    FogParams = params;
}

void FFogPostProcess::Render(ID3D11DeviceContext* context, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    UpdateConstants(context, ActiveViewport);

    context->IASetInputLayout(InputLayout);
    context->VSSetShader(VertexShader, nullptr, 0);
    context->PSSetShader(PixelShader, nullptr, 0);
    //context->ClearRenderTargetView(FoggedSceneRTV, Renderer->Graphics->ClearColor);
    context->OMSetRenderTargets(1, &FoggedSceneRTV, nullptr);

    ID3D11ShaderResourceView* colorSRV = Renderer->Graphics->SceneColorSRV;
    ID3D11ShaderResourceView* depthSRV = Renderer->Graphics->SceneDepthSRV;

    context->PSSetShaderResources(0, 1, &colorSRV);
    context->PSSetShaderResources(1, 1, &depthSRV);
    context->VSSetConstantBuffers(0, 1, &Renderer->ConstantBuffer);
    //context->PSSetConstantBuffers(0, 1, &Renderer->ConstantBuffer);
    context->PSSetConstantBuffers(6, 1, &Renderer->FogConstantBuffer);

    ID3D11SamplerState* sampler = Renderer->LinearSampler;
    context->PSSetSamplers(0, 1, &sampler);

    FullscreenQuad::Get()->Draw(context);
}

void FFogPostProcess::UpdateConstants(ID3D11DeviceContext* context, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    // Matrix 파라미터 업데이트
    const FMatrix& View = ActiveViewport->GetViewMatrix();
    const FMatrix& Proj = ActiveViewport->GetProjectionMatrix();
    const FMatrix Identity = FMatrix::Identity;
    const FVector4 DummyUUID(0, 0, 0, 0);
    const bool bSelected = false;
    FogParams.InvViewProj = FMatrix::Inverse(View * Proj);
    // Fog 파라미터 업데이트
    Renderer->GetConstantBufferUpdater().UpdateFogConstant(Renderer->FogConstantBuffer, FogParams);



    Renderer->GetConstantBufferUpdater().UpdateConstant(
        Renderer->ConstantBuffer,
        Identity, View, Proj, Identity,
        DummyUUID, bSelected);
}

void FFogPostProcess::CreateOutputRenderTarget()
{
    Renderer->GetResourceManager().CreateRenderTarget(
        FoggedSceneTexture,
        FoggedSceneRTV,
        FoggedSceneSRV,
        Renderer->Graphics->screenWidth,
        Renderer->Graphics->screenHeight,
        DXGI_FORMAT_R8G8B8A8_UNORM
    );
}
void FFogPostProcess::Resize()
{
    SAFE_RELEASE(FoggedSceneRTV);
    SAFE_RELEASE(FoggedSceneTexture);
    SAFE_RELEASE(FoggedSceneSRV);
    Renderer->GetResourceManager().CreateRenderTarget(
        FoggedSceneTexture, FoggedSceneRTV, FoggedSceneSRV,
        Renderer->Graphics->screenWidth,
        Renderer->Graphics->screenHeight,
        DXGI_FORMAT_R8G8B8A8_UNORM
    );
}
void FFogPostProcess::Release()
{
    SAFE_RELEASE(VertexShader);
    SAFE_RELEASE(PixelShader);
    SAFE_RELEASE(InputLayout);
    SAFE_RELEASE(FoggedSceneTexture);
    SAFE_RELEASE(FoggedSceneRTV);
    SAFE_RELEASE(FoggedSceneSRV);
}