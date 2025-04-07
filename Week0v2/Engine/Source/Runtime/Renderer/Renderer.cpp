#include "Renderer.h"
#include <d3dcompiler.h>

#include "Engine/World.h"
#include "Actors/Player.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "Components/LightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/UBillboardComponent.h"
#include "Components/UParticleSubUVComp.h"
#include "Components/UText.h"
#include "Components/Material/Material.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Launch/EditorEngine.h"
#include "Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "UObject/Casts.h"
#include "UObject/Object.h"
#include "PropertyEditor/ShowFlags.h"
#include "UObject/UObjectIterator.h"
#include "Components/SkySphereComponent.h"
#include "Fog/UFogComponent.h"


void FRenderer::Initialize(FGraphicsDevice* graphics)
{
    Graphics = graphics;
    RenderResourceManager.Initialize(Graphics->Device);
    ShaderManager.Initialize(Graphics->Device, Graphics->DeviceContext);
    ConstantBufferUpdater.Initialize(Graphics->DeviceContext);

    CreateShader();
    CreateConstantBuffer();
    CreateFullscreenQuad();
    CreateSampler();
    ConstantBufferUpdater.UpdateLitUnlitConstant(FlagBuffer, 1);
}

void FRenderer::Release()
{
    ReleaseShader();
    ReleaseConstantBuffer();
}

#pragma region Shader
void FRenderer::CreateShader()
{
    // 기본 셰이더 설정
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    ShaderManager.CreateVertexShader(
        L"Shaders/StaticMeshVertexShader.hlsl", "mainVS",
        VertexShader, layout, ARRAYSIZE(layout), &InputLayout, &Stride, sizeof(FVertexSimple));

    ShaderManager.CreatePixelShader(
        L"Shaders/StaticMeshPixelShader.hlsl", "mainPS",
        PixelShader);

    // 텍스쳐 셰이더 설정
    D3D11_INPUT_ELEMENT_DESC textureLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    ShaderManager.CreateVertexShader(
        L"Shaders/VertexTextureShader.hlsl", "main",
        VertexTextureShader, textureLayout, ARRAYSIZE(textureLayout), &TextureInputLayout, &TextureStride, sizeof(FVertexTexture));

    ShaderManager.CreatePixelShader(
        L"Shaders/PixelTextureShader.hlsl", "main",
        PixelTextureShader);

    // 라인 셰이더 설정
    ShaderManager.CreateVertexShader(
        L"Shaders/ShaderLine.hlsl", "mainVS",
        VertexLineShader, nullptr, 0); // 라인 셰이더는 Layout 안 쓰면 nullptr 전달

    ShaderManager.CreatePixelShader(
        L"Shaders/ShaderLine.hlsl", "mainPS",
        PixelLineShader);

    ShaderManager.CreateVertexShader(
        L"Shaders/StaticMeshFogVertexShader.hlsl", "mainVS",
        StaticMeshFogVertexShader, layout, ARRAYSIZE(layout), &FogInputLayout, &FogStride, sizeof(FVertexSimple));

    ShaderManager.CreatePixelShader(
        L"Shaders/StaticMeshFogPixelShader.hlsl", "mainPS",
        StaticMeshFogPixelShader);
    
    D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, sizeof(FVector),            D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ShaderManager.CreateVertexShader(
        L"Shaders/DebugSceneDepthShader.hlsl", "mainVS",
        DebugDepthVertexShader, inputLayoutDesc, ARRAYSIZE(inputLayoutDesc), &DebugDepthInputLayout, &DebugDepthStride, sizeof(FFullscreenVertex));

    ShaderManager.CreatePixelShader(
        L"Shaders/DebugSceneDepthShader.hlsl", "mainPS",
        DebugDepthPixelShader);
}

void FRenderer::ReleaseShader()
{
    ShaderManager.ReleaseShader(InputLayout, VertexShader, PixelShader);
    ShaderManager.ReleaseShader(TextureInputLayout, VertexTextureShader, PixelTextureShader);
    ShaderManager.ReleaseShader(nullptr, VertexLineShader, PixelLineShader);
    ShaderManager.ReleaseShader(FogInputLayout, StaticMeshFogVertexShader, StaticMeshFogPixelShader);
}


// Prepare
void FRenderer::PrepareShader() const
{
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);

    if (ConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(0, 1, &ConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &MaterialConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(2, 1, &LightingBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(3, 1, &FlagBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(4, 1, &SubMeshConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(5, 1, &TextureConstantBufer);
    }
}

void FRenderer::PrepareFogShader() const
{
    Graphics->DeviceContext->VSSetShader(StaticMeshFogVertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(StaticMeshFogPixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(FogInputLayout);

    if (ConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(0, 1, &ConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &MaterialConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(2, 1, &LightingBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(3, 1, &FlagBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(4, 1, &SubMeshConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(5, 1, &TextureConstantBufer);
        Graphics->DeviceContext->PSSetConstantBuffers(6, 1, &FogConstantBuffer); // ✅ Fog 추가
    }
}

void FRenderer::PrepareTextureShader() const
{
    Graphics->DeviceContext->VSSetShader(VertexTextureShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelTextureShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(TextureInputLayout);

    if (ConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
    }
}

void FRenderer::PrepareSubUVConstant() const
{
    if (SubUVConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &SubUVConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &SubUVConstantBuffer);
    }
}

void FRenderer::PrepareLineShader() const
{
    Graphics->DeviceContext->VSSetShader(VertexLineShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelLineShader, nullptr, 0);

    if (ConstantBuffer && GridConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer); // MatrixBuffer (b0)
        Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &GridConstantBuffer); // GridParameters (b1)
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &GridConstantBuffer);
        Graphics->DeviceContext->VSSetConstantBuffers(3, 1, &LinePrimitiveBuffer);
        Graphics->DeviceContext->VSSetShaderResources(2, 1, &pBBSRV);
        Graphics->DeviceContext->VSSetShaderResources(3, 1, &pConeSRV);
        Graphics->DeviceContext->VSSetShaderResources(4, 1, &pOBBSRV);
    }
}
#pragma endregion Shader


#pragma region ConstantBuffer
// ConstantBuffer
void FRenderer::CreateConstantBuffer()
{
    ConstantBuffer = RenderResourceManager.CreateConstantBuffer(sizeof(FConstants));
    SubUVConstantBuffer = RenderResourceManager.CreateConstantBuffer(sizeof(FSubUVConstant));
    GridConstantBuffer = RenderResourceManager.CreateConstantBuffer(sizeof(FGridParameters));
    LinePrimitiveBuffer = RenderResourceManager.CreateConstantBuffer(sizeof(FPrimitiveCounts));
    MaterialConstantBuffer = RenderResourceManager.CreateConstantBuffer(sizeof(FMaterialConstants));
    SubMeshConstantBuffer = RenderResourceManager.CreateConstantBuffer(sizeof(FSubMeshConstants));
    TextureConstantBufer = RenderResourceManager.CreateConstantBuffer(sizeof(FTextureConstants));
    LightingBuffer = RenderResourceManager.CreateConstantBuffer(sizeof(FLighting));
    FlagBuffer = RenderResourceManager.CreateConstantBuffer(sizeof(FLitUnlitConstants));
    FogConstantBuffer = RenderResourceManager.CreateConstantBuffer(sizeof(FFogParams));
}

void FRenderer::ReleaseConstantBuffer()
{
    RenderResourceManager.ReleaseBuffer(ConstantBuffer);
    RenderResourceManager.ReleaseBuffer(SubUVConstantBuffer);
    RenderResourceManager.ReleaseBuffer(GridConstantBuffer);
    RenderResourceManager.ReleaseBuffer(LinePrimitiveBuffer);
    RenderResourceManager.ReleaseBuffer(MaterialConstantBuffer);
    RenderResourceManager.ReleaseBuffer(SubMeshConstantBuffer);
    RenderResourceManager.ReleaseBuffer(TextureConstantBufer);
    RenderResourceManager.ReleaseBuffer(LightingBuffer);
    RenderResourceManager.ReleaseBuffer(FlagBuffer);
}
#pragma endregion ConstantBuffer

#pragma region

void FRenderer::ClearRenderArr()
{
    StaticMeshObjs.Empty();
    GizmoObjs.Empty();
    BillboardObjs.Empty();
    LightObjs.Empty();
}

void FRenderer::PrepareRender()
{
    if (GEngine->GetWorld()->WorldType == EWorldType::Editor)
    {
        for (const auto iter : TObjectRange<USceneComponent>())
        {
            UE_LOG(LogLevel::Display, "%d", GUObjectArray.GetObjectItemArrayUnsafe().Num());
            if (UStaticMeshComponent* pStaticMeshComp = Cast<UStaticMeshComponent>(iter))
            {
                if (!Cast<UGizmoBaseComponent>(iter))
                    StaticMeshObjs.Add(pStaticMeshComp);
            }
            if (UGizmoBaseComponent* pGizmoComp = Cast<UGizmoBaseComponent>(iter))
            {
                GizmoObjs.Add(pGizmoComp);
            }
            if (UBillboardComponent* pBillboardComp = Cast<UBillboardComponent>(iter))
            {
                BillboardObjs.Add(pBillboardComp);
            }
            if (ULightComponentBase* pLightComp = Cast<ULightComponentBase>(iter))
            {
                LightObjs.Add(pLightComp);
            }
        }
    }
    else if (GEngine->GetWorld()->WorldType == EWorldType::PIE)
    {
        // UE_LOG(LogLevel::Display, "%d", GEngine->GetWorld()->GetActors().Num() );
        for (const auto iter : GEngine->GetWorld()->GetActors())
        {
            for (const auto iter2 : iter->GetComponents())
            {
                if (UStaticMeshComponent* pStaticMeshComp = Cast<UStaticMeshComponent>(iter2))
                {
                    if (!Cast<UGizmoBaseComponent>(iter2))
                        StaticMeshObjs.Add(pStaticMeshComp);
                }
                if (UBillboardComponent* pBillboardComp = Cast<UBillboardComponent>(iter2))
                {
                    BillboardObjs.Add(pBillboardComp);
                }
                if (ULightComponentBase* pLightComp = Cast<ULightComponentBase>(iter2))
                {
                    LightObjs.Add(pLightComp);
                }
            }
        }
    }
}

void FRenderer::Render(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    Graphics->DeviceContext->RSSetViewports(1, &ActiveViewport->GetD3DViewport());
    Graphics->ChangeRasterizer(ActiveViewport->GetViewMode());
    ChangeViewMode(ActiveViewport->GetViewMode());
    ConstantBufferUpdater.UpdateLightConstant(LightingBuffer);
    UPrimitiveBatch::GetInstance().RenderBatch(ConstantBuffer, ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix());

    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
        RenderStaticMeshes(World, ActiveViewport);
    RenderGizmos(World, ActiveViewport);
    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))
        RenderBillboards(World, ActiveViewport);
    RenderLight(World, ActiveViewport);
    ClearRenderArr();
}

void FRenderer::RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &offset);
    Graphics->DeviceContext->Draw(numVertices, 0);
}

void FRenderer::RenderPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

void FRenderer::RenderPrimitive(OBJ::FStaticMeshRenderData* renderData, TArray<FStaticMaterial*> materials, TArray<UMaterial*> overrideMaterial,
                                int selectedSubMeshIndex = -1) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &renderData->VertexBuffer, &Stride, &offset);

    if (renderData->IndexBuffer)
        Graphics->DeviceContext->IASetIndexBuffer(renderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    if (renderData->MaterialSubsets.Num() == 0)
    {
        // no submesh
        Graphics->DeviceContext->DrawIndexed(renderData->Indices.Num(), 0, 0);
    }

    for (int subMeshIndex = 0; subMeshIndex < renderData->MaterialSubsets.Num(); subMeshIndex++)
    {
        int materialIndex = renderData->MaterialSubsets[subMeshIndex].MaterialIndex;

        subMeshIndex == selectedSubMeshIndex
            ? ConstantBufferUpdater.UpdateSubMeshConstant(SubMeshConstantBuffer, true)
            : ConstantBufferUpdater.UpdateSubMeshConstant(SubMeshConstantBuffer, false);

        overrideMaterial[materialIndex] != nullptr
            ? UpdateMaterial(overrideMaterial[materialIndex]->GetMaterialInfo())
            : UpdateMaterial(materials[materialIndex]->Material->GetMaterialInfo());

        if (renderData->IndexBuffer)
        {
            // index draw
            uint64 startIndex = renderData->MaterialSubsets[subMeshIndex].IndexStart;
            uint64 indexCount = renderData->MaterialSubsets[subMeshIndex].IndexCount;
            Graphics->DeviceContext->DrawIndexed(indexCount, startIndex, 0);
        }
    }
}

void FRenderer::RenderTexturePrimitive(
    ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* _TextureSRV,
    ID3D11SamplerState* _SamplerState
) const
{
    if (!_TextureSRV || !_SamplerState)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    }
    if (numIndices <= 0)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "numIndices Error");
    }
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &TextureStride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);
    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

void FRenderer::RenderTextPrimitive(
    ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11ShaderResourceView* _TextureSRV, ID3D11SamplerState* _SamplerState
) const
{
    if (!_TextureSRV || !_SamplerState)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    }
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &TextureStride, &offset);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);
    Graphics->DeviceContext->Draw(numVertices, 0);
}

void FRenderer::RenderTexturedModelPrimitive(
    ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* InTextureSRV,
    ID3D11SamplerState* InSamplerState
) const
{
    if (!InTextureSRV || !InSamplerState)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    }
    if (numIndices <= 0)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "numIndices Error");
    }
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    //Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &InTextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &InSamplerState);

    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

void FRenderer::RenderStaticMeshes(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareFogShader(); // ✅ Fog 셰이더 준비

    UFogComponent* FogComponent = World->GetFogComponent();
    FogComponent->SetCameraWorldPosition(ActiveViewport->ViewTransformPerspective.GetLocation());
    const FFogParams& FogParams = FogComponent->GetFogParams();

    ID3D11DeviceContext* Context = Graphics->DeviceContext;

    for (UStaticMeshComponent* StaticMeshComp : StaticMeshObjs)
    {
        FMatrix Model = JungleMath::CreateModelMatrix(
            StaticMeshComp->GetWorldLocation(),
            StaticMeshComp->GetWorldRotation(),
            StaticMeshComp->GetWorldScale()
        );

        FMatrix View = ActiveViewport->GetViewMatrix();
        FMatrix Proj = ActiveViewport->GetProjectionMatrix();

        // 최종 MVP 행렬
        //FMatrix MVP = Model * View * Proj;
        // 노말 회전시 필요 행렬
        FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
        FVector4 UUIDColor = StaticMeshComp->EncodeUUID() / 255.0f;

        bool bSelected = (World->GetSelectedActor() == StaticMeshComp->GetOwner());
        ConstantBufferUpdater.UpdateConstant(ConstantBuffer, Model, View, Proj, NormalMatrix, UUIDColor, bSelected);

        if (USkySphereComponent* skysphere = Cast<USkySphereComponent>(StaticMeshComp))
        {
            ConstantBufferUpdater.UpdateTextureConstant(TextureConstantBufer, skysphere->UOffset, skysphere->VOffset);
        }
        else
        {
            ConstantBufferUpdater.UpdateTextureConstant(TextureConstantBufer, 0, 0);
        }

        if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
        {
            UPrimitiveBatch::GetInstance().RenderAABB(
                StaticMeshComp->GetBoundingBox(),
                StaticMeshComp->GetWorldLocation(),
                Model
            );
        }

        if (!StaticMeshComp->GetStaticMesh()) continue;

        OBJ::FStaticMeshRenderData* renderData = StaticMeshComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        // ⬇ Fog 파라미터 넘기기 (PS: b6 슬롯)
        ConstantBufferUpdater.UpdateFogConstant(FogConstantBuffer, FogParams);
        //Context->UpdateSubresource(FogConstantBuffer, 0, nullptr, &FogParams, 0, 0);
        Context->PSSetConstantBuffers(6, 1, &FogConstantBuffer);

        // 렌더링 (Fog 셰이더 포함)
        RenderPrimitive(renderData, StaticMeshComp->GetStaticMesh()->GetMaterials(), StaticMeshComp->GetOverrideMaterials(),
                        StaticMeshComp->GetselectedSubMeshIndex());
    }
}


void FRenderer::RenderGizmos(const UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    if (!World->GetSelectedActor())
    {
        return;
    }
    FMatrix View = ActiveViewport->GetViewMatrix();
    FMatrix Proj = ActiveViewport->GetProjectionMatrix();

#pragma region GizmoDepth
    ID3D11DepthStencilState* DepthStateDisable = Graphics->DepthStateDisable;
    Graphics->DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
#pragma endregion GizmoDepth

    //  fill solid,  Wirframe 에서도 제대로 렌더링되기 위함
    Graphics->DeviceContext->RSSetState(UEditorEngine::graphicDevice.RasterizerStateSOLID);

    for (auto GizmoComp : GizmoObjs)
    {
        if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowX ||
                GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowY ||
                GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowZ)
            && World->GetEditorPlayer()->GetControlMode() != CM_TRANSLATION)
            continue;
        else if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleX ||
                GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleY ||
                GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleZ)
            && World->GetEditorPlayer()->GetControlMode() != CM_SCALE)
            continue;
        else if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleX ||
                GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleY ||
                GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleZ)
            && World->GetEditorPlayer()->GetControlMode() != CM_ROTATION)
            continue;
        FMatrix Model = JungleMath::CreateModelMatrix(GizmoComp->GetWorldLocation(),
                                                      GizmoComp->GetWorldRotation(),
                                                      GizmoComp->GetWorldScale()
        );
        FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
        FVector4 UUIDColor = GizmoComp->EncodeUUID() / 255.0f;

        //FMatrix MVP = Model * ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();

        if (GizmoComp == World->GetPickingGizmo())
            ConstantBufferUpdater.UpdateConstant(ConstantBuffer, Model, View, Proj, NormalMatrix, UUIDColor, true);
        else
            ConstantBufferUpdater.UpdateConstant(ConstantBuffer, Model, View, Proj, NormalMatrix, UUIDColor, false);

        if (!GizmoComp->GetStaticMesh()) continue;

        OBJ::FStaticMeshRenderData* renderData = GizmoComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        RenderPrimitive(renderData, GizmoComp->GetStaticMesh()->GetMaterials(), GizmoComp->GetOverrideMaterials());
    }

    Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer());

#pragma region GizmoDepth
    ID3D11DepthStencilState* originalDepthState = Graphics->DepthStencilState;
    Graphics->DeviceContext->OMSetDepthStencilState(originalDepthState, 0);
#pragma endregion GizmoDepth
}

void FRenderer::RenderBillboards(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareTextureShader();
    PrepareSubUVConstant();
    for (auto BillboardComp : BillboardObjs)
    {
        ConstantBufferUpdater.UpdateSubUVConstant(SubUVConstantBuffer, BillboardComp->finalIndexU, BillboardComp->finalIndexV);

        FMatrix Model = BillboardComp->CreateBillboardMatrix();
        FMatrix View = ActiveViewport->GetViewMatrix();
        FMatrix Proj = ActiveViewport->GetProjectionMatrix();

        // 최종 MVP 행렬
        //FMatrix MVP = Model * ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
        FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
        FVector4 UUIDColor = BillboardComp->EncodeUUID() / 255.0f;
        if (BillboardComp == World->GetPickingGizmo())
            ConstantBufferUpdater.UpdateConstant(ConstantBuffer, Model, View, Proj, NormalMatrix, UUIDColor, true);
        else
            ConstantBufferUpdater.UpdateConstant(ConstantBuffer, Model, View, Proj, NormalMatrix, UUIDColor, false);

        if (UParticleSubUVComp* SubUVParticle = Cast<UParticleSubUVComp>(BillboardComp))
        {
            RenderTexturePrimitive(
                SubUVParticle->vertexSubUVBuffer, SubUVParticle->numTextVertices,
                SubUVParticle->indexTextureBuffer, SubUVParticle->numIndices, SubUVParticle->Texture->TextureSRV, SubUVParticle->Texture->SamplerState
            );
        }
        else if (UText* Text = Cast<UText>(BillboardComp))
        {
            UEditorEngine::renderer.RenderTextPrimitive(
                Text->vertexTextBuffer, Text->numTextVertices,
                Text->Texture->TextureSRV, Text->Texture->SamplerState
            );
        }
        else
        {
            RenderTexturePrimitive(
                BillboardComp->vertexTextureBuffer, BillboardComp->numVertices,
                BillboardComp->indexTextureBuffer, BillboardComp->numIndices, BillboardComp->Texture->TextureSRV, BillboardComp->Texture->SamplerState
            );
        }
    }
    PrepareShader();
}


void FRenderer::RenderLight(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    for (auto Light : LightObjs)
    {
        FMatrix Model = JungleMath::CreateModelMatrix(Light->GetWorldLocation(), Light->GetWorldRotation(), {1, 1, 1});
        UPrimitiveBatch::GetInstance().AddCone(Light->GetWorldLocation(), Light->GetRadius(), 15, 140, Light->GetColor(), Model);
        UPrimitiveBatch::GetInstance().RenderOBB(Light->GetBoundingBox(), Light->GetWorldLocation(), Model);
    }
}

void FRenderer::RenderBatch(
    const FGridParameters& gridParam, ID3D11Buffer* pVertexBuffer, int boundingBoxCount, int coneCount, int coneSegmentCount, int obbCount
) const
{
    UINT stride = sizeof(FSimpleVertex);
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    UINT vertexCountPerInstance = 2;
    UINT instanceCount = gridParam.numGridLines + 3 + (boundingBoxCount * 12) + (coneCount * (2 * coneSegmentCount)) + (12 * obbCount);
    Graphics->DeviceContext->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
#pragma endregion Render

void FRenderer::ChangeViewMode(EViewModeIndex evi) const
{
    switch (evi)
    {
    case EViewModeIndex::VMI_Lit:
        ConstantBufferUpdater.UpdateLitUnlitConstant(FlagBuffer, 1);
        break;
    case EViewModeIndex::VMI_Wireframe:
    case EViewModeIndex::VMI_Unlit:
        ConstantBufferUpdater.UpdateLitUnlitConstant(FlagBuffer, 0);
        break;
    }
}

void FRenderer::UpdateMaterial(const FObjMaterialInfo& MaterialInfo) const
{
    ConstantBufferUpdater.UpdateMaterialConstant(MaterialConstantBuffer, MaterialInfo);

    if (MaterialInfo.bHasTexture == true)
    {
        std::shared_ptr<FTexture> texture = UEditorEngine::resourceMgr.GetTexture(MaterialInfo.DiffuseTexturePath);
        Graphics->DeviceContext->PSSetShaderResources(0, 1, &texture->TextureSRV);
        Graphics->DeviceContext->PSSetSamplers(0, 1, &texture->SamplerState);
    }
    else
    {
        ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
        ID3D11SamplerState* nullSampler[1] = {nullptr};

        Graphics->DeviceContext->PSSetShaderResources(0, 1, nullSRV);
        Graphics->DeviceContext->PSSetSamplers(0, 1, nullSampler);
    }
}


ID3D11ShaderResourceView* FRenderer::CreateBoundingBoxSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numBoundingBoxes;


    Graphics->Device->CreateShaderResourceView(pBoundingBoxBuffer, &srvDesc, &pBBSRV);
    return pBBSRV;
}

ID3D11ShaderResourceView* FRenderer::CreateOBBSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numBoundingBoxes;
    Graphics->Device->CreateShaderResourceView(pBoundingBoxBuffer, &srvDesc, &pOBBSRV);
    return pOBBSRV;
}

ID3D11ShaderResourceView* FRenderer::CreateConeSRV(ID3D11Buffer* pConeBuffer, UINT numCones)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numCones;


    Graphics->Device->CreateShaderResourceView(pConeBuffer, &srvDesc, &pConeSRV);
    return pConeSRV;
}

void FRenderer::UpdateBoundingBoxBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FBoundingBox>& BoundingBoxes, int numBoundingBoxes) const
{
    if (!pBoundingBoxBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(pBoundingBoxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = reinterpret_cast<FBoundingBox*>(mappedResource.pData);
    for (int i = 0; i < BoundingBoxes.Num(); ++i)
    {
        pData[i] = BoundingBoxes[i];
    }
    Graphics->DeviceContext->Unmap(pBoundingBoxBuffer, 0);
}

void FRenderer::UpdateOBBBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FOBB>& BoundingBoxes, int numBoundingBoxes) const
{
    if (!pBoundingBoxBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(pBoundingBoxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = reinterpret_cast<FOBB*>(mappedResource.pData);
    for (int i = 0; i < BoundingBoxes.Num(); ++i)
    {
        pData[i] = BoundingBoxes[i];
    }
    Graphics->DeviceContext->Unmap(pBoundingBoxBuffer, 0);
}

void FRenderer::UpdateConesBuffer(ID3D11Buffer* pConeBuffer, const TArray<FCone>& Cones, int numCones) const
{
    if (!pConeBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(pConeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = reinterpret_cast<FCone*>(mappedResource.pData);
    for (int i = 0; i < Cones.Num(); ++i)
    {
        pData[i] = Cones[i];
    }
    Graphics->DeviceContext->Unmap(pConeBuffer, 0);
}

void FRenderer::UpdateGridConstantBuffer(const FGridParameters& gridParams) const
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = Graphics->DeviceContext->Map(GridConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr))
    {
        memcpy(mappedResource.pData, &gridParams, sizeof(FGridParameters));
        Graphics->DeviceContext->Unmap(GridConstantBuffer, 0);
    }
    else
    {
        UE_LOG(LogLevel::Warning, "gridParams");
    }
}

void FRenderer::UpdateLinePrimitveCountBuffer(int numBoundingBoxes, int numCones) const
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = Graphics->DeviceContext->Map(LinePrimitiveBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = static_cast<FPrimitiveCounts*>(mappedResource.pData);
    pData->BoundingBoxCount = numBoundingBoxes;
    pData->ConeCount = numCones;
    Graphics->DeviceContext->Unmap(LinePrimitiveBuffer, 0);
}

void FRenderer::RenderDebugDepth()
{
    ID3D11DeviceContext* Context = Graphics->DeviceContext;

    Context->VSSetShader(DebugDepthVertexShader, nullptr, 0);
    Context->PSSetShader(DebugDepthPixelShader, nullptr, 0);

    Context->IASetInputLayout(DebugDepthInputLayout);
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(FFullscreenVertex);
    UINT offset = 0;
    Context->IASetVertexBuffers(0, 1, &FullscreenVertexBuffer, &stride, &offset);

    Context->PSSetShaderResources(0, 1, &Graphics->SceneDepthSRV);
    Context->PSSetSamplers(0, 1, &LinearSampler); // bilinear 샘플러

    Context->Draw(6, 0); // 풀스크린 쿼드
}
void FRenderer::CreateSampler()
{
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    /*samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;*/
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    Graphics->Device->CreateSamplerState(&samplerDesc, &LinearSampler);
}
// FRenderer.cpp

void FRenderer::CreateFullscreenQuad()
{
    struct FVertexUV
    {
        float x, y, z;
        float u, v;
    };

    FVertexUV QuadVertices[6] =
    {
        // Triangle 1
        { -1.f,  1.f, 0.f, 0.f, 0.f }, // Top-left
        {  1.f,  1.f, 0.f, 1.f, 0.f }, // Top-right
        { -1.f, -1.f, 0.f, 0.f, 1.f }, // Bottom-left

        // Triangle 2
        { -1.f, -1.f, 0.f, 0.f, 1.f }, // Bottom-left
        {  1.f,  1.f, 0.f, 1.f, 0.f }, // Top-right
        {  1.f, -1.f, 0.f, 1.f, 1.f }, // Bottom-right
    };

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(QuadVertices);
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = QuadVertices;

    HRESULT hr = Graphics->Device->CreateBuffer(&desc, &initData, &FullscreenVertexBuffer);
    //check(SUCCEEDED(hr));
}
