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
    void SetLightParams(const FLightParams& InParams) { LightParams = InParams; }
    virtual void TickComponent(float DeltaTime) override;

    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;
    float ComputeLightPriority(const FVector& CameraPosition) const;

private:
    FLightParams LightParams;
};