#pragma once

#include "FogTypes.h"
#include "Components/SceneComponent.h"
#include "Math/JungleMath.h"

class UFogComponent : public USceneComponent
{
    DECLARE_CLASS(UFogComponent, USceneComponent);

public:
    UFogComponent();

    void SetFogParams(
        float linearStart, float linearEnd,
        float heightStart, float heightEnd,
        const FVector4& fogColor);
    void SetFogParams(const FFogParams& inFogParams);

    void SetInvViewProj(const FMatrix& InvViewProj);
    void SetCameraWorldPosition(const FVector& inCameraPos);
    const FFogParams& GetFogParams() const;

private:
    FFogParams FogParams;
};
