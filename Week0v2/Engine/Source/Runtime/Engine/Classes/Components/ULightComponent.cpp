#include "ULightComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectFactory.h"

ULightComponent::ULightComponent() : UActorComponent()
{
    LightParams.LightPosition = FVector(0, 0, 0);
    LightParams.LightRadius = 200.0f;
    LightParams.LightColor = FVector(1, 1, 1);
    LightParams.LightIntensity = 1.0f;
    Activate();
    //SetComponentTickEnabled(true);
    if (UWorld* World = GetWorld())
    {
        World->RegisterLightComponent(this);
    }
}
ULightComponent::ULightComponent(const FLightParams& InParams)
    : LightParams(InParams)
{
    bAutoActive = true;
    OriginalColor = LightParams.LightColor;
    if (UWorld* World = GetWorld())
    {
        World->RegisterLightComponent(this);
    }
}
ULightComponent::ULightComponent(const ULightComponent& Other)
    : UActorComponent(Other), LightParams(Other.LightParams),OriginalColor(Other.OriginalColor),ColorPhaseOffset(Other.ColorPhaseOffset)
{
    if (UWorld* World = GetWorld())
    {
        World->RegisterLightComponent(this);
    }
}
ULightComponent::~ULightComponent()
{
    if (UWorld* World = GetWorld())
    {
        World->UnregisterLightComponent(this);
    }
}
void ULightComponent::TickComponent(float DeltaTime)
{
    if (true || bCycleColor)
    {
        ColorCycleTime += DeltaTime / 1000.0f; // Convert ms to sec
        float cycleSpeed = 2.0f * 3.14159265f / 1.0f; // 1초 주기
        float t = ColorCycleTime + ColorPhaseOffset;
        // 사인 곡선 기반 순환 값 (0~1)
        float rWave = (std::sin(t * cycleSpeed + 0.0f) + 1.0f) * 0.5f;
        float gWave = (std::sin(t * cycleSpeed + 2.0f) + 1.0f) * 0.5f;
        float bWave = (std::sin(t * cycleSpeed + 4.0f) + 1.0f) * 0.5f;

        // 원래 색상 기준으로 부드럽게 보간
        float alpha = 0.5f; // blending strength (0.0 ~ 1.0)

        LightParams.LightColor.x = (1.0f - alpha) * OriginalColor.x + alpha * rWave;
        LightParams.LightColor.y = (1.0f - alpha) * OriginalColor.y + alpha * gWave;
        LightParams.LightColor.z = (1.0f - alpha) * OriginalColor.z + alpha * bWave;
    }
}
UObject* ULightComponent::Duplicate() const
{
    ULightComponent* NewComp = FObjectFactory::ConstructObjectFrom<ULightComponent>(this);
    NewComp->DuplicateSubObjects(this);
    NewComp->PostDuplicate();
    return NewComp;
}

void ULightComponent::DuplicateSubObjects(const UObject* Source)
{
    UActorComponent::DuplicateSubObjects(Source);
}

void ULightComponent::PostDuplicate()
{
    UActorComponent::PostDuplicate();
}
float ULightComponent::ComputeLightPriority(const FVector& CameraPosition) const
{
    float distSqr = DistSquared(LightParams.LightPosition, CameraPosition);
    return LightParams.LightRadius * LightParams.LightRadius / (distSqr + 1.0f); // radius^2 / dist^2
}
