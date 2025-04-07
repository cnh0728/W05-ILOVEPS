#pragma once
#include "Components/MovementComponent.h"
#include "UObject/ObjectFactory.h"

class UProjectileMovementComponent : public UMovementComponent
{
    DECLARE_CLASS(UProjectileMovementComponent, UMovementComponent)

public:
    UProjectileMovementComponent() = default;
    UProjectileMovementComponent(const UProjectileMovementComponent& other);
    virtual ~UProjectileMovementComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

    float GetInitialSpeed() const { return InitialSpeed; }
    void SetInitialSpeed(float InInitSpeed) { InitialSpeed = InInitSpeed; }

    float GetMaxSpeed() const { return MaxSpeed; }
    void SetMaxSpeed(float InMaxSpeed) { MaxSpeed = InMaxSpeed; }

    FVector GetAcceleration() const { return Acceleration; }
    void SetAcceleration(const FVector& InAcceleration) { Acceleration = InAcceleration; }

    float GetGravityScale() const { return GravityScale; }
    void SetGravityScale(float Scale) { GravityScale = Scale; }

    FVector GetVelocity() const { return Velocity; }
    void SetVelocity(const FVector& InVelocity) { Velocity = InVelocity; }

private:
    float InitialSpeed;
    float MaxSpeed;
    FVector Acceleration;
    float GravityScale;
    //FVector Velocity;

private:
    float Timer = 0.0f;
};

