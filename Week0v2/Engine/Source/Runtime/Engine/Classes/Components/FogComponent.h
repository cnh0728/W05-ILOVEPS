#pragma once
#include "SceneComponent.h"

class UFogComponent : public USceneComponent
{
    DECLARE_CLASS(UFogComponent, USceneComponent)
public:
    UFogComponent() = default;
    UFogComponent(const UFogComponent& Other);

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    
    void SetColor(const FVector& InColor) {Color = InColor;}
    void SetStartEnd(float InStart){Start = InStart;}
    void SetEnd(float InEnd){End = InEnd;}
    void SetDensity(float InDensity){Density = InDensity;}
    void SetHeightFallOff(float InHeightFallOf){HeightFallOff=InHeightFallOf;}
    void SetBaseHeight(float InBaseHeight) {BaseHeight = InBaseHeight;}
    void SetScatteringIntensity(float InScatteringIntensity) {  ScatteringIntensity = InScatteringIntensity; }
    void SetLightShaftDensity(float InLightShaftDensity) {  LightShaftDensity = InLightShaftDensity; }
    void SetIsHeightFog(bool InbIsHeight){ bIsHeight = InbIsHeight; }
    
    FVector GetColor(){return Color;}
    float GetStart(){return Start;}
    float GetEnd(){return End;}
    float GetDensity(){return Density;}
    float GetHeightFallOff(){return HeightFallOff;}
    float GetBaseHeight(){return BaseHeight;}
    float GetScatteringIntensity(){ return ScatteringIntensity;}
    float GetLightShaftDensity(){return LightShaftDensity;}
    bool GetIsHeightFog(){return bIsHeight;}

    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;
private:
    FVector Color = FVector(0.8f, 0.9f, 1.0f); //푸른빛
    float Start = 0.0f;
    float End = 100.f;
    float Density = 0.02f;
    float HeightFallOff = 0.2f;
    float BaseHeight = 5.f;
    float ScatteringIntensity = 10.f;  // 추가: 빛 산란 강도 [4]
    float LightShaftDensity = 0.02f;    // 추가: 광선 밀도 [4]

    bool bIsHeight = true;
};
