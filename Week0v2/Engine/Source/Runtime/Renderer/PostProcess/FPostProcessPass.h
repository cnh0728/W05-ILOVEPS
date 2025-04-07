#pragma once
#include "Renderer/RenderResourceManager.h"

class FEditorViewportClient;
class FRenderer;

class FPostProcessPass
{
public:
    virtual ~FPostProcessPass() {}
    virtual bool Initialize(FRenderer* inRenderer) = 0;
    virtual void Render(ID3D11DeviceContext* context, std::shared_ptr<FEditorViewportClient> ActiveViewport) = 0;
    virtual void Release() = 0;

    virtual ID3D11RenderTargetView* GetOutputRTV() const = 0;
    virtual ID3D11ShaderResourceView* GetOutputSRV() const = 0;
};
