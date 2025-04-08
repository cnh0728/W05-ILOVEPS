#include "EditorEngine.h"
#include "FireBallComponent.h"
#include "Engine/World.h"

UFireBallComponent::UFireBallComponent()
{
    GEngine->GetWorld()->AddPointLights(this);
}

UFireBallComponent::UFireBallComponent(const UFireBallComponent& Other) :Intensity(Other.Intensity), Radius(Other.Radius), RadiusFallOff(Other.RadiusFallOff), Color(Other.Color)
{
    GEngine->GetWorld()->AddPointLights(this);
}

UFireBallComponent::~UFireBallComponent()
{
    // 이걸 여기 두는게 아닌 것 같음.
    //GEngine->GetWorld()->RemovePointLights(this);
}

void UFireBallComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UFireBallComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

UObject* UFireBallComponent::Duplicate() const
{
    UFireBallComponent* NewComp = FObjectFactory::ConstructObjectFrom<UFireBallComponent>(this);
    NewComp->DuplicateSubObjects(this);
    NewComp->PostDuplicate();
    return NewComp;
}

void UFireBallComponent::DuplicateSubObjects(const UObject* Source)
{
    UStaticMeshComponent::DuplicateSubObjects(Source);
}

void UFireBallComponent::PostDuplicate() {
}