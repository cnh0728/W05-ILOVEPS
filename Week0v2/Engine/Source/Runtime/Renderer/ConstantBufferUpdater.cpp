#include "ConstantBufferUpdater.h"
#include <Engine/Texture.h>

#include "Components/FireballComponent.h"
#include "Components/SceneComponent.h"
#include "Components/FogComponent.h"
#include "UnrealEd/EditorViewportClient.h"

void FConstantBufferUpdater::Initialize(ID3D11DeviceContext* InDeviceContext)
{
    DeviceContext = InDeviceContext;
}

void FConstantBufferUpdater::UpdateConstant(ID3D11Buffer* ConstantBuffer, const FMatrix& M, const FMatrix& VP, const FMatrix& NormalMatrix, FVector4 UUIDColor, bool IsSelected, bool bIsGizmo) const
{
    if (ConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR;

        DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        {
            FConstants* constants = static_cast<FConstants*>(ConstantBufferMSR.pData);
            constants->M = M;
            constants->VP = VP;
            constants->ModelMatrixInverse = NormalMatrix;
            constants->UUIDColor = UUIDColor;
            constants->IsSelected = IsSelected;
            constants->IsGizmo = bIsGizmo;
        }
        DeviceContext->Unmap(ConstantBuffer, 0);
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

void FConstantBufferUpdater::UpdateDirectionalLightConstant(ID3D11Buffer* DirectionalLightBuffer) const
{
    if (!DirectionalLightBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    DeviceContext->Map(DirectionalLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    {
        FDirectionalLight* constants = static_cast<FDirectionalLight*>(mappedResource.pData);
        constants->lightDirX = 1.0f;
        constants->lightDirY = 1.0f;
        constants->lightDirZ = 1.0f;
        constants->lightColorX = 1.0f;
        constants->lightColorY = 1.0f;
        constants->lightColorZ = 1.0f;
        constants->AmbientFactor = 0.06f;
    }
    DeviceContext->Unmap(DirectionalLightBuffer, 0);
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

void FConstantBufferUpdater::UpdateFullScreenConstant(ID3D11Buffer* FullScreenConstantBuffer, bool bIsDepth) const
{
    if (FullScreenConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR;
        DeviceContext->Map(FullScreenConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        FFullScreenConstants* constants = static_cast<FFullScreenConstants*>(constantbufferMSR.pData);
        {
            constants->bIsDepth = static_cast<int>(bIsDepth);
        }
        DeviceContext->Unmap(FullScreenConstantBuffer, 0);
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

void FConstantBufferUpdater::UpdatePointLightConstant(ID3D11Buffer* PointLightConstantBuffer, TArray<UFireBallComponent*> PointLights)
{
    if (PointLightConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

        DeviceContext->Map(PointLightConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        FPointLightConstants* constants = (FPointLightConstants*)constantbufferMSR.pData;
        {
            int Index = 0;

            for (UFireBallComponent* Comp : PointLights)
            {
                FVector4 Pos = {Comp->GetWorldLocation(), Comp->GetRadius()};
                constants->PointLightPosition[Index] = Pos;
                FVector4 TempColor = {Comp->GetColor(), Comp->GetIntensity()};
                constants->Color[Index] = TempColor;
                FVector4 TempFalloff = {Comp->GetRadiusFallOff(), 1.0}; //2번째꺼는 유효한건지 판단 기본이 0으로하고 0이면 유효데이터가 아님
                constants->FallOff[Index] = TempFalloff;
                Index++;
            }
        }
        DeviceContext->Unmap(PointLightConstantBuffer, 0);
    }
}

void FConstantBufferUpdater::UpdateCameraPosConstant(ID3D11Buffer* CameraPosConstantBuffer, std::shared_ptr<FEditorViewportClient> ActiveViewPort)
{
    if (CameraPosConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

        DeviceContext->Map(CameraPosConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        FCameraPosConstants* constants = (FCameraPosConstants*)constantbufferMSR.pData;
        {
            constants->CameraPos = ActiveViewPort->ViewTransformPerspective.GetLocation();
            constants->InverseVMatrix = FMatrix::Inverse(ActiveViewPort->View);
            constants->InversePMatrix = FMatrix::Inverse(ActiveViewPort->Projection);
        }
        DeviceContext->Unmap(CameraPosConstantBuffer, 0);
    }
    
}

void FConstantBufferUpdater::UpdateFogConstant(ID3D11Buffer* FogConstantBuffer, UFogComponent* FogComp)
{
    if (FogConstantBuffer)
    {
        bool bIsFogOn = false;
        if (FogComp)
        {
            bIsFogOn = true;
        }
        
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

        DeviceContext->Map(FogConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        FFogConstants* constants = (FFogConstants*)constantbufferMSR.pData;
        {
            constants->bIsFogOn = static_cast<int>(bIsFogOn);
            if (bIsFogOn)
            {
                constants->Color = FogComp->GetColor();
                constants->Density = FogComp->GetDensity();
                constants->FogStart = FogComp->GetStart();
                constants->FogEnd = FogComp->GetEnd();

                constants->FogZPosition = FogComp->GetWorldLocation().z;
                constants->HeightFallOff = FogComp->GetHeightFallOff();
                constants->FogBaseHeight = FogComp->GetBaseHeight();
                constants->bIsHeightFog = FogComp->GetIsHeightFog();

                constants->ScatteringIntensity = FogComp->GetScatteringIntensity();
                constants->LightShaftDensity = FogComp->GetLightShaftDensity();
            }
        }
        DeviceContext->Unmap(FogConstantBuffer, 0);
    }
}