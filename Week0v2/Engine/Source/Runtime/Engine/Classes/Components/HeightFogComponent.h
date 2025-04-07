#pragma once
#include "PrimitiveComponent.h"
#include "Math/Color.h"

class UHeightFogComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UHeightFogComponent, UPrimitiveComponent)

public:
    UHeightFogComponent();
    UHeightFogComponent(const UHeightFogComponent& Other);
    virtual ~UHeightFogComponent() override;

public:
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

public:
    void ClampToValidRange()
    {
        FogDensity = FMath::Clamp(FogDensity, 0.0f, 10.0f);
        FogHeightFalloff = FMath::Clamp(FogHeightFalloff, 0.0f, 2.0f);
    }

    float GetFogDensity() const { return FogDensity; }
    float GetFogHeightFalloff() const { return FogHeightFalloff; }
    float GetStartDistance() const { return StartDistance; }
    float GetFogCutoffDistance() const { return FogCutoffDistance; }
    float GetFogMaxOpacity() const { return FogMaxOpacity; }
    FLinearColor GetFogInscatteringColor() const { return FogInscatteringColor; }

    void SetFogDensity(float Value)
    {
        if (FogDensity != Value)
        {
            FogDensity = Value;
        }
    }
    void SetFogHeightFalloff(float Value)
    {
        if (FogHeightFalloff != Value)
        {
            FogHeightFalloff = Value;
        }
    }
    void SetStartDistance(float Value)
    {
        if (StartDistance != Value)
        {
            StartDistance = Value;
        }
    }
    void SetFogCutoffDistance(float Value)
    {
        if (FogCutoffDistance != Value)
        {
            FogCutoffDistance = Value;
        }
    }
    void SetFogMaxOpacity(float Value)
    {
        if (FogMaxOpacity != Value)
        {
            FogMaxOpacity = Value;
        }
    }
    void SetFogInscatteringColor(const FLinearColor& Value)
    {
        if (FogInscatteringColor != Value)
        {
            FogInscatteringColor = Value;
        }
    }

private:
    /** Global density factor. */
    float FogDensity = 0.02f;

    /**
     * Height density factor, controls how the density increases as height decreases.
     * Smaller values make the visible transition larger.
     */
    float FogHeightFalloff = 0.2f;

    /** Distance from the camera that the fog will start, in world units. */
    float StartDistance;

    /** Scene elements past this distance will not have fog applied.  This is useful for excluding skyboxes which already have fog baked in. */
    float FogCutoffDistance;

    /**
     * Maximum opacity of the fog.
     * A value of 1 means the fog can become fully opaque at a distance and replace scene color completely,
     * A value of 0 means the fog color will not be factored in at all.
     */
    float FogMaxOpacity;

    FLinearColor FogInscatteringColor;
};
