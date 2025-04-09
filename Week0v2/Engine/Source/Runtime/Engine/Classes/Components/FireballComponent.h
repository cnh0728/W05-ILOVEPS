#pragma once
#include "StaticMeshComponent.h"

class UFireBallComponent : public UStaticMeshComponent
{
    DECLARE_CLASS(UFireBallComponent, UStaticMeshComponent)

public:
    UFireBallComponent();

    UFireBallComponent(const UFireBallComponent& Other);
    ~UFireBallComponent() override;
    
    void InitializeComponent() override;
    void TickComponent(float DeltaTime) override;
    
    [[nodiscard]] float GetIntensity() const { return Intensity; }
    [[nodiscard]] float GetRadius() const { return Radius; }
    [[nodiscard]] float GetRadiusFallOff() const { return RadiusFallOff; }
    [[nodiscard]] FVector GetColor() const { return Color; }

    void SetIntensity(float intensity) { Intensity = intensity; }
    void SetRadius(float radius) { Radius = radius; }
    void SetRadiusFallOff(float radius_fall_off) { RadiusFallOff = radius_fall_off; }
    void SetColor(const FVector& color) { Color = color; }

    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

private:
    float Intensity = 100.f;
    float Radius = 10.f;
    float RadiusFallOff = 0.03f;
    FVector Color = FVector(1.f, 0.f, 0.f);
};
