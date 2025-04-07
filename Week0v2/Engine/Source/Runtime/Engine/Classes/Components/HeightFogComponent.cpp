#include "HeightFogComponent.h"

UHeightFogComponent::UHeightFogComponent()
    : UPrimitiveComponent()
    , FogDensity(0.02f)
    , FogHeightFalloff(0.2f)
    , StartDistance(0.0f)
    , FogCutoffDistance(0.0f)
    , FogMaxOpacity(1.0f)
    , FogInscatteringColor(FLinearColor::White)
{
    SetType(StaticClass()->GetName());
}
UHeightFogComponent::~UHeightFogComponent()
{
}

void UHeightFogComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UHeightFogComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}