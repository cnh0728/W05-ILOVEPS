#include "CompositePostProcess.h"

#include "Renderer/Utils/FullscreenQuad.h"

bool FCompositePostProcess::Initialize(FRenderer * inRenderer)
{
    Renderer = inRenderer;
    LoadShaders();
    return true;
}

void FCompositePostProcess::LoadShaders()
{
    D3D11_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(FVector), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    Renderer->GetShaderManager().CreateVertexShader(
        L"Shaders/PostProcess/CompositePostProcess.hlsl",
        "mainVS",
        VertexShader,
        inputLayout,
        ARRAYSIZE(inputLayout),
        &InputLayout
    );

    Renderer->GetShaderManager().CreatePixelShader(
        L"Shaders/PostProcess/CompositePostProcess.hlsl",
        "mainPS",
        PixelShader
    );
}

void FCompositePostProcess::SetOutput(ID3D11RenderTargetView* InOutputRTV)
{
    OutputRTV = InOutputRTV;
}

void FCompositePostProcess::SetInputs(const std::vector<FCompositeInput>& InInputs)
{
    Inputs = InInputs;
}

void FCompositePostProcess::Render(ID3D11DeviceContext* context)
{
    if (!OutputRTV || Inputs.empty()) return;

    context->OMSetRenderTargets(1, &OutputRTV, nullptr);
    context->IASetInputLayout(InputLayout);
    context->VSSetShader(VertexShader, nullptr, 0);
    context->PSSetShader(PixelShader, nullptr, 0);

    for (size_t i = 0; i < Inputs.size(); ++i)
    {
        context->PSSetShaderResources((UINT)i, 1, &Inputs[i].SRV);
    }

    ID3D11SamplerState* sampler = Renderer->LinearSampler;
    context->PSSetSamplers(0, 1, &sampler);

    FullscreenQuad::Get()->Draw(context);
}
void FCompositePostProcess::Release()
{
    SAFE_RELEASE(VertexShader);
    SAFE_RELEASE(PixelShader);
    SAFE_RELEASE(InputLayout);

    Inputs.clear();
    OutputRTV = nullptr;
}