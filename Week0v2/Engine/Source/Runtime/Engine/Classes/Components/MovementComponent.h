#pragma once
#include "Components/ActorComponent.h"
#include "UObject/ObjectFactory.h"

class UMovementComponent : public UActorComponent
{
    DECLARE_CLASS(UMovementComponent, UActorComponent)

public:
    UMovementComponent() = default;
    virtual ~UMovementComponent() override;
    UMovementComponent(const UMovementComponent& other);

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

protected:
};

