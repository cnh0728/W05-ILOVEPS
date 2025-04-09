#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define _TCHAR_DEFINED
#include <d3d11.h>

#include "EngineBaseTypes.h"

#include "Core/HAL/PlatformType.h"
#include "Core/Math/Vector4.h"

class FGraphicsDevice {
public:
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* DeviceContext = nullptr;
    IDXGISwapChain* SwapChain = nullptr;
    ID3D11Texture2D* FrameBuffer = nullptr;
    ID3D11Texture2D* UUIDFrameBuffer = nullptr;
    ID3D11RenderTargetView* RTVs[2];
    ID3D11RenderTargetView* FrameBufferRTV = nullptr;
    ID3D11RenderTargetView* UUIDFrameBufferRTV = nullptr;
    ID3D11RasterizerState* RasterizerStateSOLID = nullptr;
    ID3D11RasterizerState* RasterizerStateWIREFRAME = nullptr;
    DXGI_SWAP_CHAIN_DESC SwapchainDesc;

#pragma region DeferredRender
    ID3D11Texture2D* PositionFrameBuffer = nullptr;
    ID3D11Texture2D* NormalFrameBuffer = nullptr;
    ID3D11Texture2D* AlbedoFrameBuffer = nullptr;
    ID3D11Texture2D* MaterialFrameBuffer = nullptr;
    ID3D11Texture2D* SpecularFrameBuffer = nullptr;
    ID3D11Texture2D* EmissiveFrameBuffer = nullptr;
    ID3D11Texture2D* AmbientFrameBuffer = nullptr;
    
    ID3D11RenderTargetView* PositionFrameBufferRTV = nullptr;
    ID3D11RenderTargetView* NormalFrameBufferRTV = nullptr;
    ID3D11RenderTargetView* AlbedoFrameBufferRTV = nullptr;
    ID3D11RenderTargetView* MaterialFrameBufferRTV = nullptr;
    ID3D11RenderTargetView* SpecularFrameBufferRTV = nullptr;
    ID3D11RenderTargetView* EmissiveFrameBufferRTV = nullptr;
    ID3D11RenderTargetView* AmbientFrameBufferRTV = nullptr;
    ID3D11RenderTargetView* DefferedRTVs[7];

    ID3D11ShaderResourceView* PositionResourceView = nullptr;
    ID3D11ShaderResourceView* NormalResourceView = nullptr;
    ID3D11ShaderResourceView* AlbedoResourceView = nullptr;
    ID3D11ShaderResourceView* MaterialResourceView = nullptr;
    ID3D11ShaderResourceView* SpecularResourceView = nullptr;
    ID3D11ShaderResourceView* EmissiveResourceView = nullptr;
    ID3D11ShaderResourceView* AmbientResourceView = nullptr;
    ID3D11ShaderResourceView* DeferredSRVs[7];

    //포스트프로세싱
    ID3D11Texture2D* FogFrameBuffer = nullptr;
    ID3D11RenderTargetView* FogFrameBufferRTV = nullptr;
    ID3D11ShaderResourceView* FogResourceView = nullptr;

    ID3D11Texture2D* LightFrameBuffer = nullptr;
    ID3D11RenderTargetView* LightFrameBufferRTV = nullptr;
    ID3D11ShaderResourceView* LightResourceView = nullptr;
    
//depth 렌더링 변수
    ID3D11Texture2D* DepthStencilResourceBuffer = nullptr;
    ID3D11ShaderResourceView* DepthStencilResourceView = nullptr;
    
    ID3D11ShaderResourceView* FullScreenResourceView[7];
    UINT RenderResourceTextureCount = 1;
    
    UINT screenWidth = 0;
    UINT screenHeight = 0;
#pragma endregion 

    // Depth-Stencil 관련 변수
    ID3D11Texture2D* DepthStencilBuffer = nullptr;  // 깊이/스텐실 텍스처    
    ID3D11DepthStencilView* DepthStencilView = nullptr;  // 깊이/스텐실 뷰
    ID3D11DepthStencilState* DepthStencilState = nullptr;
    FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f }; // 화면을 초기화(clear) 할 때 사용할 색상(RGBA)

    ID3D11DepthStencilState* DepthStateDisable = nullptr;

    void Initialize(HWND hWindow);
    void CreateDeviceAndSwapChain(HWND hWindow);
    void CreateDepthStencilBuffer(HWND hWindow);
    void CreateDepthStencilState();
    void CreateRasterizerState();
    void ReleaseDeviceAndSwapChain();
    void CreateFrameBuffer();
    void ReleaseFrameBuffer();
    void ReleaseRasterizerState();
    void ReleaseDepthStencilResources();
    void Release();
    void SwapBuffer();
    void Prepare();
    // void Prepare(D3D11_VIEWPORT* viewport);
    void OnResize(HWND hWindow);
    ID3D11RasterizerState* GetCurrentRasterizer() { return CurrentRasterizer; }
    void ChangeRasterizer(EViewModeIndex evi);
    void ChangeDepthStencilState(ID3D11DepthStencilState* newDetptStencil);

    uint32 GetPixelUUID(POINT pt);
    uint32 DecodeUUIDColor(FVector4 UUIDColor);
private:
    ID3D11RasterizerState* CurrentRasterizer = nullptr;
};

