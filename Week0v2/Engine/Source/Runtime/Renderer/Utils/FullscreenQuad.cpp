#include "FullscreenQuad.h"

#include "EditorEngine.h"

void FullscreenQuad::Initialize()
{
    struct FVertex
    {
        float x, y, z;
        float u, v;
    };

    FVertex QuadVertices[6] =
    {
        { -1.f,  1.f, 0.f, 0.f, 0.f },
        {  1.f,  1.f, 0.f, 1.f, 0.f },
        { -1.f, -1.f, 0.f, 0.f, 1.f },
        { -1.f, -1.f, 0.f, 0.f, 1.f },
        {  1.f,  1.f, 0.f, 1.f, 0.f },
        {  1.f, -1.f, 0.f, 1.f, 1.f }
    };

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(QuadVertices);
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = QuadVertices;

    ID3D11Device* device = UEditorEngine::graphicDevice.Device;
    device->CreateBuffer(&desc, &initData, &VertexBuffer);
}

void FullscreenQuad::Draw(ID3D11DeviceContext* context)
{
    UINT stride = sizeof(float) * 5;
    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->Draw(6, 0);
}