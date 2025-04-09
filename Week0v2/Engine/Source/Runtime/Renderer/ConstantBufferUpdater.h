#pragma once

#define _TCHAR_DEFINED
#include <d3d11.h>
#include "Define.h"

class ULightComponent;
struct FLightParams;
struct FFogParams;

class FConstantBufferUpdater
{
public:
    void Initialize(ID3D11DeviceContext* InDeviceContext);

    void UpdateConstant(ID3D11Buffer* ConstantBuffer,
        const FMatrix& Model,const FMatrix& View,const FMatrix& Proj,const FMatrix& NormalMatrix,
        const FVector4& UUIDColor,bool IsSelected) const;
    void UpdateFogConstant(ID3D11Buffer* FogConstantBuffer, const FFogParams& FogParams) const;
    void UpdateLightConstant(
        ID3D11Buffer* LightConstantBuffer,
        const TArray<ULightComponent*>& LightComponents,
        const FVector& CameraPosition);
    void UpdateMaterialConstant(ID3D11Buffer* MaterialConstantBuffer, const FObjMaterialInfo& MaterialInfo) const;
    void UpdateLightConstant(ID3D11Buffer* LightingBuffer) const;
    void UpdateLitUnlitConstant(ID3D11Buffer* FlagBuffer, int isLit) const;
    void UpdateSubMeshConstant(ID3D11Buffer* SubMeshConstantBuffer, bool isSelected) const;
    void UpdateTextureConstant(ID3D11Buffer* TextureConstantBufer, float UOffset, float VOffset);
    void UpdateSubUVConstant(ID3D11Buffer* SubUVConstantBuffer, float _indexU, float _indexV) const;

private:
    ID3D11DeviceContext* DeviceContext = nullptr;
};
