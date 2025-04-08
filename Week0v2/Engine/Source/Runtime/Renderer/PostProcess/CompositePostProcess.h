#pragma once
#include "Renderer/RenderResourceManager.h"

class FRenderer;

struct FCompositeInput
{
    ID3D11ShaderResourceView* SRV = nullptr;
    float Weight = 1.0f; // 각 입력에 가중치를 줄 수도 있음
};
enum class EPostProcessViewMode
{
    Composite,
    FogOnly,
    DepthOnly,
    LightOnly,
};
class FCompositePostProcess
{
public:
    bool Initialize(FRenderer* inRenderer);
    void Render(ID3D11DeviceContext* context);
    void Release();

    void SetOutput(ID3D11RenderTargetView* OutputRTV);
    void SetInputs(const std::vector<FCompositeInput>& InInputs);
    void SetViewMode(EPostProcessViewMode InMode) { ViewMode = InMode; }

private:
    FRenderer* Renderer = nullptr;

    ID3D11VertexShader* VertexShader = nullptr;
    ID3D11PixelShader* PixelShader = nullptr;
    ID3D11InputLayout* InputLayout = nullptr;

    ID3D11RenderTargetView* OutputRTV = nullptr;
    std::vector<FCompositeInput> Inputs;

    EPostProcessViewMode ViewMode = EPostProcessViewMode::Composite;
    void LoadShaders();
};