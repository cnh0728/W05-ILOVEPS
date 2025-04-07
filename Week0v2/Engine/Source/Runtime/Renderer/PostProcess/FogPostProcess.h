#pragma once
#include "FPostProcessPass.h"
#include "Fog/FogTypes.h"

class FEditorViewportClient;

class FFogPostProcess : public FPostProcessPass
{
public:
    bool Initialize(FRenderer* inRenderer) override;
    void Render(ID3D11DeviceContext* context, std::shared_ptr<FEditorViewportClient> ActiveViewport) override;
    void Release() override;

    void SetFogParams(const FFogParams& params);

    ID3D11RenderTargetView* GetOutputRTV() const override { return FoggedSceneRTV; }
    ID3D11ShaderResourceView* GetOutputSRV() const override { return FoggedSceneSRV; }

private:
    FRenderer* Renderer = nullptr; // ShaderManager, ResourceManager 접근용

    ID3D11VertexShader* VertexShader = nullptr;
    ID3D11PixelShader* PixelShader = nullptr;
    ID3D11InputLayout* InputLayout = nullptr;
    /*ID3D11Buffer* FogConstantBuffer = nullptr;
    ID3D11Buffer* MatrixConstantBuffer = nullptr;*/

    ID3D11Texture2D* FoggedSceneTexture = nullptr;
    ID3D11RenderTargetView* FoggedSceneRTV = nullptr;
    ID3D11ShaderResourceView* FoggedSceneSRV = nullptr;

    FFogParams FogParams;

    void CreateOutputRenderTarget(); // ▶ Renderer를 통해 RTV 생성
    void UpdateConstants(ID3D11DeviceContext* context, std::shared_ptr<FEditorViewportClient> ActiveViewport);

    //void CreateConstantBuffers();
};
