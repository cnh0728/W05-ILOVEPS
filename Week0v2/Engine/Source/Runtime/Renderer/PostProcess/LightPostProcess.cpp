#include "LightPostProcess.h"

#include "EditorEngine.h"
#include "FogPostProcess.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Engine/World.h"
#include "Renderer/Renderer.h"
#include "Renderer/Utils/FullscreenQuad.h"
#include "UnrealEd/EditorViewportClient.h"

bool FLightPostProcess::Initialize(FRenderer* inRenderer)
{
    Renderer = inRenderer;
    LoadShaders();
    CreateOutputRenderTarget();
    LightConstantBuffer = Renderer->GetResourceManager().CreateConstantBuffer(
        sizeof(FVector4) * MAX_LIGHT_COUNT * 2 + sizeof(int32) + sizeof(float) * 3);

    //LightConstantBuffer =  Renderer->GetResourceManager().CreateConstantBuffer(sizeof(FLightParams));
    /*
    TArray<FLightParams> TestLights;
    float rad = 7.0f;
    TestLights.Add({ FVector(-5, 0, 0), rad, FVector(1.0f, 0.0f, 0.0f), 2.0f }); // Red
    TestLights.Add({ FVector(5, 0, 0), rad, FVector(0.0f, 1.0f, 0.0f), 2.0f });  // Green
    TestLights.Add({ FVector(0, 5, 0), rad, FVector(0.0f, 0.0f, 1.0f), 2.0f });  // Blue
    TestLights.Add({ FVector(0, -5, 0), rad, FVector(1.0f, 1.0f, 0.0f), 2.0f }); // Yellow
    TestLights.Add({ FVector(0, 0, 0), rad, FVector(1.0f, 0.0f, 1.0f), 2.0f });   // Magenta

    SetLightParams(TestLights);*/

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
void FLightPostProcess::UpdateConstants(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    if (LightConstantBuffer)
    {

        Renderer->GetConstantBufferUpdater().UpdateLightConstant(LightConstantBuffer,GEngine->GetWorld()->WorldLightComponents,ActiveViewport->ViewTransformPerspective.GetLocation());
        //Renderer->GetConstantBufferUpdater().UpdateLightConstant(LightConstantBuffer, LightParamsArray.GetData(), LightParamsArray.Num());
    }
}
void FLightPostProcess::Render(ID3D11DeviceContext* context, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    UpdateConstants(ActiveViewport);
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
