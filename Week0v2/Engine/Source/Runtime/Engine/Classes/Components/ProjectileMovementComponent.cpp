#include "ProjectileMovementComponent.h"
#include "Engine/World.h"
#include "Define.h"
#include "UObject/ObjectFactory.h"
#include "GameFramework/Actor.h"

#include "Launch/EditorEngine.h"

UProjectileMovementComponent::UProjectileMovementComponent(const UProjectileMovementComponent& other) : InitialSpeed(1000.0f), MaxSpeed(10000.0f), Acceleration(FVector(1.0f, 0.f, 0.f)), GravityScale(1.0f)
{
}

UProjectileMovementComponent::~UProjectileMovementComponent()
{
}

void UProjectileMovementComponent::InitializeComponent()
{
    Super::InitializeComponent();
    InitialSpeed = 10.0f;
    MaxSpeed = 100.0f;
    Acceleration = FVector(1.0f, 0.f, 0.f);
    GravityScale = 1.0f;
    Velocity = FVector(0.f, 1.f, 0.f);

}

void UProjectileMovementComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    // test
    Timer += DeltaTime * 0.005f;

    Acceleration = FVector(0.f, 0.f, -9.8f * GravityScale);
    Velocity =FVector(
        Velocity.x + Acceleration.x * Timer,
        Velocity.y + Acceleration.y * Timer,
        Velocity.z + Acceleration.z * Timer
    );
    if (Velocity.Magnitude() > MaxSpeed)
    {
        Velocity = Velocity.Normalize() * MaxSpeed;
    }
    
    FVector NewLocation = GetOwner()->GetActorLocation() + Velocity * Timer;
    GetOwner()->SetActorLocation(NewLocation);
}

UObject* UProjectileMovementComponent::Duplicate() const
{
    UProjectileMovementComponent* ClonedActor = FObjectFactory::ConstructObjectFrom<UProjectileMovementComponent>(this);
    ClonedActor->DuplicateSubObjects(this);
    ClonedActor->PostDuplicate();
    return ClonedActor;
}

void UProjectileMovementComponent::DuplicateSubObjects(const UObject* Source)
{
    UMovementComponent::DuplicateSubObjects(Source);
}

void UProjectileMovementComponent::PostDuplicate()
{
    UMovementComponent::PostDuplicate();
}
