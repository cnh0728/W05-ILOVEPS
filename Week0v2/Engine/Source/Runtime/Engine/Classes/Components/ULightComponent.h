#pragma once
#include "ActorComponent.h"
#include "Renderer/PostProcess/LightPostProcess.h"

class ULightComponent : public UActorComponent
{
    DECLARE_CLASS(ULightComponent, UActorComponent)
public:
    ULightComponent();
    ULightComponent(const ULightComponent& Other);
    ULightComponent(const FLightParams& InParams);
    ~ULightComponent() override;

    FLightParams* GetLightParamsPtr() { return &LightParams; } 
    FLightParams GetLightParams() const { return LightParams; }
    //void SetLightParams(const FLightParams& InParams) { LightParams = InParams; }
    virtual void TickComponent(float DeltaTime) override;

    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;
    float ComputeLightPriority(const FVector& CameraPosition) const;

    void SetLight(const float radius, const FVector& color, float intensity)
    {
        LightParams.LightRadius = radius;
        LightParams.LightColor = color;
        LightParams.LightIntensity = intensity;
        OriginalColor = color;
    }

    void SetColorCycling(bool bEnable) { bCycleColor = bEnable; }

    void SetColorPhaseOffset(float InOffset) { ColorPhaseOffset = InOffset; }
private:
    FLightParams LightParams;
    FVector OriginalColor;
    bool bCycleColor = false;
    float ColorPhaseOffset = 0.0f;
    float ColorCycleTime = 0.0f;
};