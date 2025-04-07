#include "EditorEngine.h"
#include "FireBallComponent.h"
#include "Engine/World.h"

UFireBallComponent::UFireBallComponent()
{
    GEngine->GetWorld()->AddPointLights(this);
}

UFireBallComponent::~UFireBallComponent()
{
    GEngine->GetWorld()->RemovePointLights(this);
}

void UFireBallComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UFireBallComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}
