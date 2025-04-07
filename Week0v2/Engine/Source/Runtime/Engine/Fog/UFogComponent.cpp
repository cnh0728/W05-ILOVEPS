#include "UFogComponent.h"

UFogComponent::UFogComponent()
{
}

void UFogComponent::SetFogParams(
    float linearStart, float linearEnd,
    float heightStart, float heightEnd,
    const FVector4& fogColor)
{
    FogParams.LinearStart = linearStart;
    FogParams.LinearEnd = linearEnd;
    FogParams.HeightStart = heightStart;
    FogParams.HeightEnd = heightEnd;
    FogParams.FogColor = fogColor;
}

void UFogComponent::SetCameraWorldPosition(const FVector& inCameraPos)
{
    FogParams.CameraWorldPos = inCameraPos;
}

const FFogParams& UFogComponent::GetFogParams() const
{
    return FogParams;
}
