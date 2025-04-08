#pragma once
#include "SceneComponent.h"

class UFogComponent : public USceneComponent
{
    DECLARE_CLASS(UFogComponent, USceneComponent)
public:
    UFogComponent() = default;
    
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    
    void SetColor(const FVector& InColor) {Color = InColor;}
    void SetStartEnd(float InStart){Start = InStart;}
    void SetEnd(float InEnd){End = InEnd;}
    void SetDensity(float InDensity){Density = InDensity;}
    void SetHeightFallOf(float InHeightFallOf){HeightFallOff=InHeightFallOf;}
    void SetBaseHeight(float InBaseHeight) {BaseHeight = InBaseHeight;}
    void SetIsHeightFog(bool InbIsHeight){ bIsHeight = InbIsHeight; }
    
    FVector GetColor(){return Color;}
    float GetStart(){return Start;}
    float GetEnd(){return End;}
    float GetDensity(){return Density;}
    float GetHeightFallOff(){return HeightFallOff;}
    float GetBaseHeight(){return BaseHeight;}
    bool GetIsHeightFog(){return bIsHeight;}
    
private:
    FVector Color = FVector(0.8f, 0.9f, 1.0f); //푸른빛
    float Start = 0.0f;
    float End = 10.f;
    float Density = 0.02f;
    float HeightFallOff = 0.2f;
    float BaseHeight = 10.f;
    bool bIsHeight = true;
};
