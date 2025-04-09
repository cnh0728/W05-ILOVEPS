#pragma once
#include "Components/MovementComponent.h"
#include "UObject/ObjectFactory.h"

class URotationMovementComponent : public UMovementComponent
{
    DECLARE_CLASS(URotationMovementComponent, UMovementComponent)

public:
    URotationMovementComponent() = default;
    URotationMovementComponent(const URotationMovementComponent& other);
    virtual ~URotationMovementComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

    float GetInitialSpeed() const { return InitialSpeed; }
    void SetInitialSpeed(float InInitSpeed) { InitialSpeed = InInitSpeed; }

    bool IsCounterClockwise() const { return bIsCounterClockwise; }
    void SetCounterClockwise(bool InIsCounterClockwise) { bIsCounterClockwise = InIsCounterClockwise; }
private:
    float InitialSpeed;
    bool bIsCounterClockwise = true;

    float Timer = 0.0f;
};

