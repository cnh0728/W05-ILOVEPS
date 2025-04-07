#pragma once
#include "D3D11RHI/GraphicDevice.h"
#include "Renderer/ConstantBufferUpdater.h"
#include "Renderer/Renderer.h"

class FullscreenQuad
{
public:
    static FullscreenQuad* Get()
    {
        static FullscreenQuad Instance;
        return &Instance;
    }

    void Initialize();
    void Draw(ID3D11DeviceContext* context);
    inline void DrawFullscreenTexture(
        FRenderer* Renderer,
        ID3D11ShaderResourceView* inputSRV,
        ID3D11RenderTargetView* outputRTV)
    {
        ID3D11DeviceContext* context = Renderer->Graphics->DeviceContext;

        context->OMSetRenderTargets(1, &outputRTV, nullptr);

        context->IASetInputLayout(Renderer->FullscreenInputLayout);
        context->VSSetShader(Renderer->FullscreenVertexShader, nullptr, 0);
        context->PSSetShader(Renderer->FullscreenPixelShader, nullptr, 0);

        context->PSSetShaderResources(0, 1, &inputSRV);
        context->PSSetSamplers(0, 1, &Renderer->LinearSampler);

        FullscreenQuad::Get()->Draw(context);
    }
private:
    ID3D11Buffer* VertexBuffer = nullptr;
};
