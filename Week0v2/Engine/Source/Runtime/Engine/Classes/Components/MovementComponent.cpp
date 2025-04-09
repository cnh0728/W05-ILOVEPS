#include "MovementComponent.h"
#include "Engine/World.h"
#include "Define.h"

#include "UObject/ObjectFactory.h"
#include "GameFramework/Actor.h"

#include "Launch/EditorEngine.h"

UMovementComponent::UMovementComponent(const UMovementComponent& other)
{
}

UMovementComponent::~UMovementComponent()
{
}

void UMovementComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UMovementComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

}

UObject* UMovementComponent::Duplicate() const
{
    UMovementComponent* ClonedActor = FObjectFactory::ConstructObjectFrom<UMovementComponent>(this);
    ClonedActor->DuplicateSubObjects(this);
    ClonedActor->PostDuplicate();
    return ClonedActor;
}

void UMovementComponent::DuplicateSubObjects(const UObject* Source)
{
    UActorComponent::DuplicateSubObjects(Source);
}

void UMovementComponent::PostDuplicate()
{
    UActorComponent::PostDuplicate();
}
