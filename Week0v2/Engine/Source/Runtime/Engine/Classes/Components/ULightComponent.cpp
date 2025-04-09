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
    : UActorComponent(Other), LightParams(Other.LightParams),OriginalColor(Other.OriginalColor)
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
    if (true | bCycleColor) {
        ColorCycleTime += DeltaTime / 1000.0f; // Convert ms to sec
        float cycleDuration = 3.0f; // 3 seconds for full RGB rotation
        float t = fmod(ColorCycleTime, cycleDuration);

        float r = 0.0f, g = 0.0f, b = 0.0f;
        if (t < 1.0f)
        {
            r = 1.0f - t;
            g = t;
            b = 0.0f;
        }
        else if (t < 2.0f)
        {
            r = 0.0f;
            g = 2.0f - t;
            b = t - 1.0f;
        }
        else
        {
            r = t - 2.0f;
            g = 0.0f;
            b = 3.0f - t;
        }

        LightParams.LightColor.x = fmodf(OriginalColor.x + r, 1.0f);
        LightParams.LightColor.y = fmodf(OriginalColor.y + g, 1.0f);
        LightParams.LightColor.z = fmodf(OriginalColor.z + b, 1.0f);

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
