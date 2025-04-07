#include "RotationMovementComponent.h"
#include "Engine/World.h"
#include "Define.h"
#include "UObject/ObjectFactory.h"
#include "GameFramework/Actor.h"

#include "Launch/EditorEngine.h"

URotationMovementComponent::URotationMovementComponent(const URotationMovementComponent& other)
    : InitialSpeed(other.InitialSpeed),
    bIsCounterClockwise(other.bIsCounterClockwise)
{
}

URotationMovementComponent::~URotationMovementComponent()
{
}

void URotationMovementComponent::InitializeComponent()
{
    Super::InitializeComponent();
    InitialSpeed = 100.0f;
    bIsCounterClockwise = true;
}

void URotationMovementComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    Timer = DeltaTime * 0.005f;

    float RotationSpeed = InitialSpeed * Timer;

    FVector CurrentRotation = GetOwner()->GetActorRotation();
    float YawDelta = bIsCounterClockwise ? RotationSpeed : -RotationSpeed;
    FVector NewRotation = CurrentRotation + FVector(0.f, 0.f, YawDelta);

    GetOwner()->SetActorRotation(NewRotation);
}

UObject* URotationMovementComponent::Duplicate() const
{
    URotationMovementComponent* ClonedComponent = FObjectFactory::ConstructObjectFrom<URotationMovementComponent>(this);
    ClonedComponent->DuplicateSubObjects(this);
    ClonedComponent->PostDuplicate();
    return ClonedComponent;
}

void URotationMovementComponent::DuplicateSubObjects(const UObject* Source)
{
    UMovementComponent::DuplicateSubObjects(Source);
}

void URotationMovementComponent::PostDuplicate()
{
    UMovementComponent::PostDuplicate();
}
