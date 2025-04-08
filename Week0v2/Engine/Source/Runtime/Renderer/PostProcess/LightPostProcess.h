#pragma once
#include "Renderer/RenderResourceManager.h"

class FRenderer;
class FEditorViewportClient;
struct FLightParams
{
    FVector LightPosition;
    float LightRadius;
    FVector LightColor;
    float LightIntensity;
};
class FLightPostProcess
{
public:
    bool Initialize(FRenderer* inRenderer);
    void Render(ID3D11DeviceContext* context, std::shared_ptr<FEditorViewportClient> ActiveViewport);


    void Resize();
    void Release();

    ID3D11ShaderResourceView* GetOutputSRV() const { return LightSRV; }

private:
    void CreateOutputRenderTarget();
    void LoadShaders();

private:
    FRenderer* Renderer = nullptr;

    ID3D11VertexShader* VertexShader = nullptr;
    ID3D11PixelShader* PixelShader = nullptr;
    ID3D11InputLayout* InputLayout = nullptr;

    ID3D11Texture2D* LightTexture = nullptr;
    ID3D11RenderTargetView* LightRTV = nullptr;
    ID3D11ShaderResourceView* LightSRV = nullptr;

    ID3D11Buffer* LightConstantBuffer = nullptr;
    FLightParams LightParams;
};