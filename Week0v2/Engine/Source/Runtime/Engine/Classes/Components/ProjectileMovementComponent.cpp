#include "ProjectileMovementComponent.h"
#include "Engine/World.h"
#include "Define.h"
#include "UObject/ObjectFactory.h"
#include "GameFramework/Actor.h"

#include "Launch/EditorEngine.h"

UProjectileMovementComponent::UProjectileMovementComponent(const UProjectileMovementComponent& other)
    : InitialSpeed(other.InitialSpeed),
    MaxSpeed(other.MaxSpeed),
    Acceleration(other.Acceleration),
    GravityScale(other.GravityScale),
    Velocity(other.Velocity)
{
}

UProjectileMovementComponent::~UProjectileMovementComponent()
{
}

void UProjectileMovementComponent::InitializeComponent()
{
    Super::InitializeComponent();

    InitialSpeed = 100.0f;
    MaxSpeed = 1000.0f;
    Acceleration = FVector(0.1f, 0.0f, 0.0f);
    Velocity = Acceleration * InitialSpeed;

    GravityScale = 0.0f;
}

void UProjectileMovementComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    Timer = DeltaTime / 1000.0f;
    FVector Gravity = FVector(0.f, 0.f, -9.8f) * GravityScale;
    Velocity = Velocity + Gravity * Timer;

    if (Velocity.Magnitude() > MaxSpeed)
    {
        Velocity = Velocity.Normalize() * MaxSpeed;
    }

    FVector NewLocation = GetOwner()->GetActorLocation() + Velocity * Timer;
    GetOwner()->SetActorLocation(NewLocation);
}

UObject* UProjectileMovementComponent::Duplicate() const
{
    UProjectileMovementComponent* ClonedComponent = FObjectFactory::ConstructObjectFrom<UProjectileMovementComponent>(this);
    ClonedComponent->DuplicateSubObjects(this);
    ClonedComponent->PostDuplicate();
    return ClonedComponent;
}

void UProjectileMovementComponent::DuplicateSubObjects(const UObject* Source)
{
    UMovementComponent::DuplicateSubObjects(Source);
}

void UProjectileMovementComponent::PostDuplicate()
{
    UMovementComponent::PostDuplicate();
}
