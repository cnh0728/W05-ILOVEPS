#pragma once

#define _TCHAR_DEFINED
#include <d3d11.h>
#include "Define.h"
#include "Components/FogComponent.h"

class FEditorViewportClient;
class UFireBallComponent;

class FConstantBufferUpdater
{
public:
    void Initialize(ID3D11DeviceContext* InDeviceContext);

    void UpdateConstant(ID3D11Buffer* ConstantBuffer, const FMatrix& M, const FMatrix& VP, const FMatrix& NormalMatrix, FVector4 UUIDColor, bool
                        IsSelected, bool bIsGizmo) const;
    void UpdateMaterialConstant(ID3D11Buffer* MaterialConstantBuffer, const FObjMaterialInfo& MaterialInfo) const;
    void UpdateLightConstant(ID3D11Buffer* LightingBuffer) const;
    void UpdateLitUnlitConstant(ID3D11Buffer* FlagBuffer, int isLit) const;
    void UpdateFullScreenConstant(ID3D11Buffer* FullScreenConstantBuffer, bool bIsDepth) const;
    void UpdateSubMeshConstant(ID3D11Buffer* SubMeshConstantBuffer, bool isSelected) const;
    void UpdateTextureConstant(ID3D11Buffer* TextureConstantBufer, float UOffset, float VOffset);
    void UpdateSubUVConstant(ID3D11Buffer* SubUVConstantBuffer, float _indexU, float _indexV) const;
    void UpdateCameraPosConstant(ID3D11Buffer* CameraPosConstantBuffer, std::shared_ptr<FEditorViewportClient> ActiveViewPort);
    void UpdateFogConstant(ID3D11Buffer* FogConstantBuffer, UFogComponent* FogComp);
    void UpdatePointLightConstant(ID3D11Buffer* PointLightConstantBuffer, TArray<UFireBallComponent*> PointLights);

private:
    ID3D11DeviceContext* DeviceContext = nullptr;
};

