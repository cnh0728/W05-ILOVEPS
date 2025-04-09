#include "ConstantBufferUpdater.h"
#include <Engine/Texture.h>

#include "Renderer.h"
#include "Components/ULightComponent.h"
#include "Fog/FogTypes.h"
#include "GameFramework/Actor.h"
#include "PostProcess/LightPostProcess.h"

void FConstantBufferUpdater::Initialize(ID3D11DeviceContext* InDeviceContext)
{
    DeviceContext = InDeviceContext;
}

void FConstantBufferUpdater::UpdateConstant(
    ID3D11Buffer* ConstantBuffer,
    const FMatrix& Model,
    const FMatrix& View,
    const FMatrix& Proj,
    const FMatrix& NormalMatrix,
    const FVector4& UUIDColor,
    bool IsSelected) const
{
    if (ConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR;
        DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
        {
            FConstants* constants = static_cast<FConstants*>(ConstantBufferMSR.pData);
            constants->Model = Model;
            constants->View = View;
            constants->Projection = Proj;
            constants->ModelMatrixInverseTranspose = NormalMatrix;
            constants->UUIDColor = UUIDColor;
            constants->IsSelected = IsSelected;
        }
        DeviceContext->Unmap(ConstantBuffer, 0);
    }
}

void FConstantBufferUpdater::UpdateFogConstant(ID3D11Buffer* FogConstantBuffer, const FFogParams& FogParams) const
{
    if (FogConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        DeviceContext->Map(FogConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, &FogParams, sizeof(FFogParams));
        DeviceContext->Unmap(FogConstantBuffer, 0);
    }
}
void FConstantBufferUpdater::UpdateLightConstant(
    ID3D11Buffer* LightConstantBuffer,
    const TArray<ULightComponent*>& LightComponents,
    const FVector& CameraPosition)
{
    if (!LightConstantBuffer || LightComponents.Num() == 0)
        return;

    struct FLightBuffer
    {
        FVector4 LightPositionRadius[MAX_LIGHT_COUNT];
        FVector4 LightColorIntensity[MAX_LIGHT_COUNT];
        int LightCount;
        float Padding[3];
    };

    FLightBuffer BufferData = {};

    // 복사 및 우선순위 기반 정렬용 구조체
    struct FSortableLight
    {
        ULightComponent* Comp;
        float Priority;
    };

    TArray<FSortableLight> Sorted;
    for (ULightComponent* Comp : LightComponents)
    {
        if (!Comp) continue;
        float Score = Comp->ComputeLightPriority(CameraPosition);
        Sorted.Add({ Comp, Score });
    }

    Sorted.Sort([](const FSortableLight& A, const FSortableLight& B)
        {
            return A.Priority > B.Priority;
        });

    int Count = 0;
    for (const FSortableLight& LightEntry : Sorted)
    {
        if (Count >= MAX_LIGHT_COUNT)
            break;

        const FLightParams* Light = LightEntry.Comp->GetLightParamsPtr();
        if (!Light) continue;

        const FVector WorldPosition = Light->LightPosition + LightEntry.Comp->GetOwner()->GetActorLocation();
        BufferData.LightPositionRadius[Count] = FVector4(WorldPosition, Light->LightRadius);
        BufferData.LightColorIntensity[Count] = FVector4(Light->LightColor, Light->LightIntensity);
        ++Count;
    }

    BufferData.LightCount = Count;

    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(DeviceContext->Map(LightConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        memcpy(mapped.pData, &BufferData, sizeof(FLightBuffer));
        DeviceContext->Unmap(LightConstantBuffer, 0);
    }
}



void FConstantBufferUpdater::UpdateMaterialConstant(ID3D11Buffer* MaterialConstantBuffer, const FObjMaterialInfo& MaterialInfo) const
{
    if (MaterialConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR;

        DeviceContext->Map(MaterialConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        {
            FMaterialConstants* constants = static_cast<FMaterialConstants*>(ConstantBufferMSR.pData);
            constants->DiffuseColor = MaterialInfo.Diffuse;
            constants->TransparencyScalar = MaterialInfo.TransparencyScalar;
            constants->AmbientColor = MaterialInfo.Ambient;
            constants->DensityScalar = MaterialInfo.DensityScalar;
            constants->SpecularColor = MaterialInfo.Specular;
            constants->SpecularScalar = MaterialInfo.SpecularScalar;
            constants->EmmisiveColor = MaterialInfo.Emissive;
        }
        DeviceContext->Unmap(MaterialConstantBuffer, 0);
    }
}

void FConstantBufferUpdater::UpdateLightConstant(ID3D11Buffer* LightingBuffer) const
{
    if (!LightingBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    DeviceContext->Map(LightingBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    {
        FLighting* constants = static_cast<FLighting*>(mappedResource.pData);
        constants->lightDirX = 1.0f;
        constants->lightDirY = 1.0f;
        constants->lightDirZ = 1.0f;
        constants->lightColorX = 1.0f;
        constants->lightColorY = 1.0f;
        constants->lightColorZ = 1.0f;
        constants->AmbientFactor = 0.06f;
    }
    DeviceContext->Unmap(LightingBuffer, 0);
}


void FConstantBufferUpdater::UpdateLitUnlitConstant(ID3D11Buffer* FlagBuffer, int isLit) const
{
    if (FlagBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR;
        DeviceContext->Map(FlagBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        auto constants = static_cast<FLitUnlitConstants*>(constantbufferMSR.pData);
        {
            constants->isLit = isLit;
        }
        DeviceContext->Unmap(FlagBuffer, 0);
    }
}

void FConstantBufferUpdater::UpdateSubMeshConstant(ID3D11Buffer* SubMeshConstantBuffer, bool isSelected) const
{
    if (SubMeshConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR;
        DeviceContext->Map(SubMeshConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        FSubMeshConstants* constants = (FSubMeshConstants*)constantbufferMSR.pData;
        {
            constants->isSelectedSubMesh = isSelected;
        }
        DeviceContext->Unmap(SubMeshConstantBuffer, 0);
    }
}

void FConstantBufferUpdater::UpdateTextureConstant(ID3D11Buffer* TextureConstantBufer, float UOffset, float VOffset)
{
    if (TextureConstantBufer)
    {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR;
        DeviceContext->Map(TextureConstantBufer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        FTextureConstants* constants = (FTextureConstants*)constantbufferMSR.pData;
        {
            constants->UOffset = UOffset;
            constants->VOffset = VOffset;
        }
        DeviceContext->Unmap(TextureConstantBufer, 0);
    }
}

void FConstantBufferUpdater::UpdateSubUVConstant(ID3D11Buffer* SubUVConstantBuffer, float _indexU, float _indexV) const
{
    if (SubUVConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

        DeviceContext->Map(SubUVConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR); // update constant buffer every frame
        auto constants = static_cast<FSubUVConstant*>(constantbufferMSR.pData);
        {
            constants->indexU = _indexU;
            constants->indexV = _indexV;
        }
        DeviceContext->Unmap(SubUVConstantBuffer, 0);
    }
}
