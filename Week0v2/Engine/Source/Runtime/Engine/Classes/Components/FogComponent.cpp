#include "FogComponent.h"

#include "Engine/World.h"
#include "UObject/ObjectFactory.h"

UFogComponent::UFogComponent(const UFogComponent& Other) : Color(Other.Color), Start(Other.Start), End(Other.End), Density(Other.Density), HeightFallOff(Other.HeightFallOff),
BaseHeight(Other.BaseHeight), ScatteringIntensity(Other.ScatteringIntensity), LightShaftDensity(Other.LightShaftDensity),bIsHeight(Other.bIsHeight)
{
    GetWorld()->SetFog(this);
}

void UFogComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UFogComponent::TickComponent(float DeltaTime)
{   
    Super::TickComponent(DeltaTime);
}

UObject* UFogComponent::Duplicate() const
{
    UFogComponent* NewComp = FObjectFactory::ConstructObjectFrom<UFogComponent>(this);
    NewComp->DuplicateSubObjects(this);
    NewComp->PostDuplicate();
    return NewComp;
}

void UFogComponent::DuplicateSubObjects(const UObject* Source)
{
    Super::DuplicateSubObjects(Source);
}

void UFogComponent::PostDuplicate() {
}