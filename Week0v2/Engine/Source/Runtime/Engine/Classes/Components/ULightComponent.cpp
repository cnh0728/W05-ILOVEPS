#include "ULightComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectFactory.h"

ULightComponent::ULightComponent() : UActorComponent()
{
    LightParams.LightPosition = FVector(0, 0, 100);
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
    if (UWorld* World = GetWorld())
    {
        World->RegisterLightComponent(this);
    }
}
ULightComponent::ULightComponent(const ULightComponent& Other)
    : UActorComponent(Other), LightParams(Other.LightParams)
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
    /*if (GetOwner())
    {
        LightParams.LightPosition = GetOwner()->GetActorLocation();
    }*/
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
