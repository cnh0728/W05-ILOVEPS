#include "Renderer.h"
#include <d3dcompiler.h>

#include "Engine/World.h"
#include "Actors/Player.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "Components/LightComponent.h"
#include "Components/QuadTexture.h"
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


void FRenderer::Initialize(FGraphicsDevice* graphics)
{
    Graphics = graphics;
    RenderResourceManager.Initialize(Graphics->Device);
    ShaderManager.Initialize(Graphics->Device, Graphics->DeviceContext);
    ConstantBufferUpdater.Initialize(Graphics->DeviceContext);

    CreateShader();
    CreateConstantBuffer();
    ConstantBufferUpdater.UpdateLitUnlitConstant(FlagBuffer, 1);

    CreateScreenBuffer();
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
        L"Shaders/DefferedVertexShader.hlsl", "mainVS",
        VertexShader, layout, ARRAYSIZE(layout), &InputLayout, &Stride, sizeof(FVertexSimple));

    ShaderManager.CreatePixelShader(
        L"Shaders/DefferedPixelShader.hlsl", "mainPS",
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
        L"Shaders/FullScreenVertexShader.hlsl", "mainVS",
        FullScreenVertexShader, textureLayout, ARRAYSIZE(textureLayout), &TextureInputLayout, &TextureStride, sizeof(FVertexTexture));

    ShaderManager.CreatePixelShader(
        L"Shaders/FullScreenPixelShader.hlsl", "mainPS",
        FullScreenPixelShader);

    ShaderManager.CreateVertexShader(
        L"Shaders/LightVertexShader.hlsl", "mainVS",
        LightVertexShader, textureLayout, ARRAYSIZE(textureLayout), &TextureInputLayout, &TextureStride, sizeof(FVertexTexture));

    ShaderManager.CreatePixelShader(
        L"Shaders/LightPixelShader.hlsl", "mainPS",
        LightPixelShader);

    ShaderManager.CreateVertexShader(
        L"Shaders/FogVertexShader.hlsl", "mainVS",
        FogVertexShader, textureLayout, ARRAYSIZE(textureLayout), &TextureInputLayout, &TextureStride, sizeof(FVertexTexture));

    ShaderManager.CreatePixelShader(
        L"Shaders/FogPixelShader.hlsl", "mainPS",
        FogPixelShader);
}

void FRenderer::ReleaseShader()
{
    ShaderManager.ReleaseShader(InputLayout, VertexShader, PixelShader);
    ShaderManager.ReleaseShader(TextureInputLayout, VertexTextureShader, PixelTextureShader);
    ShaderManager.ReleaseShader(nullptr, VertexLineShader, PixelLineShader);

    ShaderManager.ReleaseShader(nullptr, FullScreenVertexShader, FullScreenPixelShader);
    ShaderManager.ReleaseShader(nullptr, LightVertexShader, LightPixelShader);
    ShaderManager.ReleaseShader(nullptr, FogVertexShader, FogPixelShader);    
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
        Graphics->DeviceContext->PSSetConstantBuffers(2, 1, &TextureConstantBufer);
        Graphics->DeviceContext->PSSetConstantBuffers(3, 1, &FlagBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(4, 1, &SubMeshConstantBuffer);
    }
}

void FRenderer::PrepareLightProcessShader() const
{
    Graphics->DeviceContext->VSSetShader(LightVertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(LightPixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(TextureInputLayout);
    
    //밑에는 렌더구간
    Graphics->DeviceContext->RSSetState(UEditorEngine::graphicDevice.RasterizerStateSOLID);
    Graphics->DeviceContext->OMSetDepthStencilState(nullptr, 0);
    Graphics->DeviceContext->OMSetRenderTargets(1, &Graphics->LightFrameBufferRTV, nullptr); // 렌더 타겟 설정(백버퍼를 가르킴)
    
    if (PointLightConstantBuffer)
    {
        Graphics->DeviceContext->PSSetConstantBuffers(0, 1, &CameraPosConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &PointLightConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(2, 1, &DirectionalLightBuffer);
    }
}

void FRenderer::PrepareFogProcessShader() const
{
    Graphics->DeviceContext->VSSetShader(FogVertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(FogPixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(TextureInputLayout);
    
    //밑에는 렌더구간
    Graphics->DeviceContext->RSSetState(UEditorEngine::graphicDevice.RasterizerStateSOLID);
    Graphics->DeviceContext->OMSetDepthStencilState(nullptr, 0);
    Graphics->DeviceContext->OMSetRenderTargets(1, &Graphics->FogFrameBufferRTV, nullptr); // 마지막이라서 바로 최종에 그림
    
    if (FullScreenConstantBuffer)
    {
        Graphics->DeviceContext->PSSetConstantBuffers(0, 1, &CameraPosConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &FogConstantBuffer);
    }
}

void FRenderer::PrepareFullScreenShader() const
{
    Graphics->DeviceContext->VSSetShader(FullScreenVertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(FullScreenPixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(TextureInputLayout);
    
    //밑에는 렌더구간
    Graphics->DeviceContext->RSSetState(UEditorEngine::graphicDevice.RasterizerStateSOLID);
    Graphics->DeviceContext->OMSetDepthStencilState(nullptr, 0);
    Graphics->DeviceContext->OMSetRenderTargets(1, &Graphics->FrameBufferRTV, nullptr); // 렌더 타겟 설정(백버퍼를 가르킴)
    
    if (FullScreenConstantBuffer)
    {
        Graphics->DeviceContext->PSSetConstantBuffers(0, 1, &FullScreenConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &CameraPosConstantBuffer);
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
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);     // MatrixBuffer (b0)
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
    DirectionalLightBuffer = RenderResourceManager.CreateConstantBuffer(sizeof(FDirectionalLight));
    FlagBuffer = RenderResourceManager.CreateConstantBuffer(sizeof(FLitUnlitConstants));
    FogConstantBuffer = RenderResourceManager.CreateConstantBuffer(sizeof(FFogConstants));
    FullScreenConstantBuffer = RenderResourceManager.CreateConstantBuffer(sizeof(FFullScreenConstants));
    CameraPosConstantBuffer = RenderResourceManager.CreateConstantBuffer(sizeof(FCameraPosConstants));
    PointLightConstantBuffer = RenderResourceManager.CreateConstantBuffer(sizeof(FPointLightConstants));
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
    RenderResourceManager.ReleaseBuffer(DirectionalLightBuffer);
    RenderResourceManager.ReleaseBuffer(FlagBuffer);
    RenderResourceManager.ReleaseBuffer(FogConstantBuffer);
    RenderResourceManager.ReleaseBuffer(FullScreenConstantBuffer);
    RenderResourceManager.ReleaseBuffer(CameraPosConstantBuffer);
    RenderResourceManager.ReleaseBuffer(PointLightConstantBuffer);
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
    ConstantBufferUpdater.UpdateDirectionalLightConstant(DirectionalLightBuffer);
    ConstantBufferUpdater.UpdateFogConstant(FogConstantBuffer, World->GetFog()); //TODO: Fog Update시에만 업데이트
    ConstantBufferUpdater.UpdatePointLightConstant(PointLightConstantBuffer, World->GetPointLights());
    ConstantBufferUpdater.UpdateCameraPosConstant(CameraPosConstantBuffer, ActiveViewport); //TODO: Camera Update시에만 업데이트
    ChangeViewMode(ActiveViewport->GetViewMode());

    UPrimitiveBatch::GetInstance().RenderBatch(ConstantBuffer, ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix());

    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
        RenderStaticMeshes(World, ActiveViewport);
    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))
        RenderBillboards(World, ActiveViewport);
    RenderLight(World, ActiveViewport);

#pragma region postProcessing

    ProcessLightScreen();
    ProcessFogScreen();
    RenderPostProcess(ActiveViewport);
    
#pragma endregion
    

    RenderGizmos(World, ActiveViewport);
    
    ClearRenderArr();
}

void FRenderer::ProcessLightScreen()
{
    PrepareLightProcessShader();
    
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &FullScreenVertexBuffer, &TextureStride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(FullScreenIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, Graphics->RenderResourceTextureCount, Graphics->FullScreenResourceView); //최초의 Process라서 GBuffer만
    Graphics->DeviceContext->PSSetSamplers(0, 1, &FullScreenSamplerState);
    Graphics->DeviceContext->DrawIndexed(FullScreenIndexCount, 0, 0);
}

void FRenderer::ProcessFogScreen()
{
    PrepareFogProcessShader();

    ID3D11ShaderResourceView* FogSRV[8];
    for (int i=0;i<ARRAYSIZE(Graphics->FullScreenResourceView); i++)
    {
        FogSRV[i] = Graphics->FullScreenResourceView[i];
    }
    FogSRV[7] = Graphics->LightResourceView; //이거 전이 Light라서
    
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &FullScreenVertexBuffer, &TextureStride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(FullScreenIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, Graphics->RenderResourceTextureCount+1, FogSRV); //이거 전이 Light라서
    Graphics->DeviceContext->PSSetSamplers(0, 1, &FullScreenSamplerState);
    Graphics->DeviceContext->DrawIndexed(FullScreenIndexCount, 0, 0);
}

void FRenderer::RenderPostProcess(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareFullScreenShader();

    ID3D11ShaderResourceView* PostProcessSRV[8];
    for (int i=0;i<ARRAYSIZE(Graphics->FullScreenResourceView); i++)
    {
        PostProcessSRV[i] = Graphics->FullScreenResourceView[i];
    }

    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_FOG))
    {
        PostProcessSRV[7] = Graphics->FogResourceView;
    }else
    {
        PostProcessSRV[7] = Graphics->LightResourceView;
    }
    
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &FullScreenVertexBuffer, &TextureStride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(FullScreenIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    Graphics->DeviceContext->PSSetShaderResources(0, Graphics->RenderResourceTextureCount+1, PostProcessSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &FullScreenSamplerState);
    Graphics->DeviceContext->DrawIndexed(FullScreenIndexCount, 0, 0);
    
    OrganizeFullScreen();
}

void FRenderer::CreateScreenBuffer()
{
    FullScreenVertexBuffer = UEditorEngine::renderer.GetResourceManager().CreateVertexBuffer(quadTextureVertices, ARRAYSIZE(quadTextureVertices));
    FullScreenVertexCount = ARRAYSIZE(quadTextureVertices);
    
    FullScreenIndexBuffer = UEditorEngine::renderer.GetResourceManager().CreateIndexBuffer(quadTextureInices, ARRAYSIZE(quadTextureInices));
    FullScreenIndexCount = ARRAYSIZE(quadTextureInices);

    D3D11_SAMPLER_DESC SamplerDesc = {};
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 선형 필터링
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;    // U축 래핑 모드
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;    // V축 래핑 모드
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

    Graphics->Device->CreateSamplerState(&SamplerDesc, &FullScreenSamplerState);
}


void FRenderer::OrganizeFullScreen()
{
    Graphics->DeviceContext->OMSetRenderTargets(1, &Graphics->FrameBufferRTV, Graphics->DepthStencilView); // 렌더 타겟 설정(백버퍼를 가르킴)
    Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilState, 0);
    Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer());

    Graphics->DeviceContext->VSSetShader(UEditorEngine::renderer.VertexShader, nullptr, 0); //TODO: FullscreenShader로 바꿔주기
    Graphics->DeviceContext->PSSetShader(UEditorEngine::renderer.PixelShader, nullptr, 0); //TODO: FullscreenShader로 바꿔주기
    Graphics->DeviceContext->IASetInputLayout(UEditorEngine::renderer.InputLayout);
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

void FRenderer::RenderPrimitive(OBJ::FStaticMeshRenderData* renderData, TArray<FStaticMaterial*> materials, TArray<UMaterial*> overrideMaterial, int selectedSubMeshIndex = -1) const
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

        subMeshIndex == selectedSubMeshIndex ? ConstantBufferUpdater.UpdateSubMeshConstant(SubMeshConstantBuffer, true) : ConstantBufferUpdater.UpdateSubMeshConstant(SubMeshConstantBuffer, false);

        overrideMaterial[materialIndex] != nullptr ?
            UpdateMaterial(overrideMaterial[materialIndex]->GetMaterialInfo()) : UpdateMaterial(materials[materialIndex]->Material->GetMaterialInfo());

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
    PrepareShader();
    for (UStaticMeshComponent* StaticMeshComp : StaticMeshObjs)
    {
        FMatrix Model = JungleMath::CreateModelMatrix(
            StaticMeshComp->GetWorldLocation(),
            StaticMeshComp->GetWorldRotation(),
            StaticMeshComp->GetWorldScale()
        );
        // 최종 MVP 행렬
        FMatrix VP = ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
        // 노말 회전시 필요 행렬
        FMatrix NormalMatrix = FMatrix::Inverse(Model);
        FVector4 UUIDColor = StaticMeshComp->EncodeUUID() / 255.0f;

        ConstantBufferUpdater.UpdateConstant(ConstantBuffer, Model, VP, NormalMatrix, UUIDColor, true, false);

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

        RenderPrimitive(renderData, StaticMeshComp->GetStaticMesh()->GetMaterials(), StaticMeshComp->GetOverrideMaterials(), StaticMeshComp->GetselectedSubMeshIndex());
    }
}

void FRenderer::RenderGizmos(const UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    if (!World->GetSelectedActor())
    {
        return;
    }

#pragma region GizmoDepth
    Graphics->DeviceContext->OMSetRenderTargets(1, &Graphics->FrameBufferRTV, nullptr);
    Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStateDisable, 0);
#pragma endregion GizmoDepth

    //  fill solid,  Wirframe 에서도 제대로 렌더링되기 위함
    Graphics->DeviceContext->RSSetState(UEditorEngine::graphicDevice.RasterizerStateSOLID);

    if (ConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(0, 1, &ConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &MaterialConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(2, 1, &DirectionalLightBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(3, 1, &FlagBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(4, 1, &SubMeshConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(5, 1, &TextureConstantBufer);
    }
    
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
        FMatrix NormalMatrix = FMatrix::Inverse(Model);
        FVector4 UUIDColor = GizmoComp->EncodeUUID() / 255.0f;

        FMatrix VP = ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();

        if (GizmoComp == World->GetPickingGizmo())
            ConstantBufferUpdater.UpdateConstant(ConstantBuffer, Model, VP, NormalMatrix, UUIDColor, true, true);
        else
            ConstantBufferUpdater.UpdateConstant(ConstantBuffer, Model, VP, NormalMatrix, UUIDColor, false, true);

        if (!GizmoComp->GetStaticMesh()) continue;

        OBJ::FStaticMeshRenderData* renderData = GizmoComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        RenderPrimitive(renderData, GizmoComp->GetStaticMesh()->GetMaterials(), GizmoComp->GetOverrideMaterials());
    }

    Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer());

#pragma region GizmoDepth
    Graphics->DeviceContext->OMSetRenderTargets(1, &Graphics->FrameBufferRTV, Graphics->DepthStencilView); // 렌더 타겟 설정(백버퍼를 가르킴)
    Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilState, 0);
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

        // 최종 MVP 행렬
        FMatrix VP = ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
        FMatrix NormalMatrix = FMatrix::Inverse(Model);
        FVector4 UUIDColor = BillboardComp->EncodeUUID() / 255.0f;
        if (BillboardComp == World->GetPickingGizmo())
            ConstantBufferUpdater.UpdateConstant(ConstantBuffer, Model, VP, NormalMatrix, UUIDColor, true, false);
        else
            ConstantBufferUpdater.UpdateConstant(ConstantBuffer, Model, VP, NormalMatrix, UUIDColor, false, false);

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
        FMatrix Model = JungleMath::CreateModelMatrix(Light->GetWorldLocation(), Light->GetWorldRotation(), { 1, 1, 1 });
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
    case VMI_Lit:
        ConstantBufferUpdater.UpdateLitUnlitConstant(FlagBuffer, 1);
        break;
    default:
        ConstantBufferUpdater.UpdateLitUnlitConstant(FlagBuffer, 0);
        break;
    }

    if (evi == VMI_DepthView)
    {
        ConstantBufferUpdater.UpdateFullScreenConstant(FullScreenConstantBuffer, true);
        Graphics->RenderResourceTextureCount = 1;
        Graphics->FullScreenResourceView[0] = Graphics->DepthStencilResourceView;
    }else
    {
        ConstantBufferUpdater.UpdateFullScreenConstant(FullScreenConstantBuffer, false);
        Graphics->RenderResourceTextureCount = ARRAYSIZE(Graphics->DeferredSRVs);
        for (int i=0;i<Graphics->RenderResourceTextureCount;i++)
        {
            Graphics->FullScreenResourceView[i] = Graphics->DeferredSRVs[i];
        }
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