#include "RenderResourceManager.h"

void FRenderResourceManager::Initialize(ID3D11Device* InDevice) {
    Device = InDevice;
}

ID3D11Buffer* FRenderResourceManager::CreateConstantBuffer(UINT size)
{
    D3D11_BUFFER_DESC constantbufferdesc = {};
    constantbufferdesc.ByteWidth = size + 0xf & 0xfffffff0;
    constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
    constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    ID3D11Buffer* buffer = nullptr;
    Device->CreateBuffer(&constantbufferdesc, nullptr, &buffer);
    return buffer;
}

void FRenderResourceManager::ReleaseBuffer(ID3D11Buffer*& buffer) {
    if (buffer)
    {
        buffer->Release();
        buffer = nullptr;
    }
}
bool FRenderResourceManager::CreateRenderTarget(
    ID3D11Texture2D*& outTexture,
    ID3D11RenderTargetView*& outRTV,
    ID3D11ShaderResourceView*& outSRV,
    UINT width,
    UINT height,
    DXGI_FORMAT format)
{
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = format;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = Device->CreateTexture2D(&texDesc, nullptr, &outTexture);
    if (FAILED(hr)) return false;

    hr = Device->CreateRenderTargetView(outTexture, nullptr, &outRTV);
    if (FAILED(hr)) return false;

    hr = Device->CreateShaderResourceView(outTexture, nullptr, &outSRV);
    return SUCCEEDED(hr);
}
