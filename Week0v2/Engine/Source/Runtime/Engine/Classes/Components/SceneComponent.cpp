#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Math/JungleMath.h"
#include "UObject/ObjectFactory.h"
#include "UTextUUID.h"
USceneComponent::USceneComponent() :RelativeLocation(FVector(0.f, 0.f, 0.f)), RelativeRotation(FVector(0.f, 0.f, 0.f)), RelativeScale3D(FVector(1.f, 1.f, 1.f))
{
}
USceneComponent::USceneComponent(const USceneComponent& Other)
    : UActorComponent(Other),
      AttachParent(nullptr), // 복제 시 복원
      RelativeLocation(Other.RelativeLocation),
      RelativeRotation(Other.RelativeRotation),
      QuatRotation(Other.QuatRotation),
      RelativeScale3D(Other.RelativeScale3D)
{
}
USceneComponent::~USceneComponent()
{
	if (uuidText) delete uuidText;
}
void USceneComponent::InitializeComponent()
{
    Super::InitializeComponent();

}

void USceneComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);
}


int USceneComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    int nIntersections = 0;
    return nIntersections;
}

FVector USceneComponent::GetForwardVector()
{
	FVector Forward = FVector(1.f, 0.f, 0.0f);
	Forward = JungleMath::FVectorRotate(Forward, QuatRotation);
	return Forward;
}

FVector USceneComponent::GetRightVector()
{
	FVector Right = FVector(0.f, 1.f, 0.0f);
	Right = JungleMath::FVectorRotate(Right, QuatRotation);
	return Right;
}

FVector USceneComponent::GetUpVector()
{
	FVector Up = FVector(0.f, 0.f, 1.0f);
	Up = JungleMath::FVectorRotate(Up, QuatRotation);
	return Up;
}


void USceneComponent::AddLocation(FVector _added)
{
	RelativeLocation = RelativeLocation + _added;

}

void USceneComponent::AddRotation(FVector _added)
{
	RelativeRotation = RelativeRotation + _added;

}

void USceneComponent::AddScale(FVector _added)
{
	RelativeScale3D = RelativeScale3D + _added;

}

FVector USceneComponent::GetWorldRotation()
{
	if (AttachParent)
	{
		return FVector(AttachParent->GetLocalRotation() + GetLocalRotation());
	}
	else
		return GetLocalRotation();
}

FVector USceneComponent::GetWorldScale()
{
	//if (AttachParent)
	//{
	//	return FVector(AttachParent->GetWorldScale() + GetLocalScale());
	//}
	//else
	//	return GetLocalScale();


    if (AttachParent)
    {
        FVector parentScale = AttachParent->GetWorldScale();
        FVector localScale = GetLocalScale();
        return FVector(parentScale.x * localScale.x, parentScale.y * localScale.y, parentScale.z * localScale.z);
    }
    else
    {
        return GetLocalScale();
    }
}

FVector USceneComponent::GetWorldLocation()
{
	if (AttachParent)
	{
		return FVector(AttachParent->GetWorldLocation() + GetLocalLocation());
	}
	else
		return GetLocalLocation();
}

FVector USceneComponent::GetLocalRotation()
{
	return JungleMath::QuaternionToEuler(QuatRotation);
}

void USceneComponent::SetRotation(FVector _newRot)
{
	RelativeRotation = _newRot;
	QuatRotation = JungleMath::EulerToQuaternion(_newRot);
}

void USceneComponent::SetupAttachment(USceneComponent* InParent)
{
    //if (
    //    InParent != AttachParent                                  // 설정하려는 Parent가 기존의 Parent와 다르거나
    //    && InParent != this                                       // InParent가 본인이 아니고
    //    && InParent != nullptr                                    // InParent가 유효한 포인터 이며
    //    && (
    //        AttachParent == nullptr                               // AttachParent도 유효하며
    //        || !AttachParent->AttachChildren.Contains(this)  // 이미 AttachParent의 자식이 아닌 경우
    //    ) 
    //) {
    //    SetAttachParent(InParent);
    //    InParent->AttachChildren.AddUnique(this);
    //}

    // 현재 컴포넌트의 소유자가 유효한지 확인
    if (!this->GetOwner())
        return;

    // InParent가 유효하고 자기 자신과 동일하지 않은지 확인합니다.
    if (!InParent || InParent == this)
        return;

    // InParent의 소유자도 유효해야 합니다.
    if (!InParent->GetOwner())
        return;

    // 부모의 소유자와 현재 컴포넌트의 소유자가 동일한지 확인합니다.
    if (InParent->GetOwner() != this->GetOwner())
        return;

    // 부모 체인을 순회하여 사이클(무한참조)이 발생하는지 확인합니다.
    USceneComponent* ParentCheck = InParent;
    while (ParentCheck)
    {
        if (ParentCheck == this)
        {
            // 사이클이 발견되면 연결하지 않습니다.
            return;
        }
        ParentCheck = ParentCheck->GetAttachParent();
    }

    // 기존 조건에 따라 AttachParent가 올바르게 설정되지 않았다면 연결합니다.
    if (InParent != AttachParent && (AttachParent == nullptr || !AttachParent->AttachChildren.Contains(this)))
    {
        SetAttachParent(InParent);
        InParent->AttachChildren.AddUnique(this);
    }
}


USceneComponent* USceneComponent::GetAttachParent() const
{
    return AttachParent;
}

void USceneComponent::SetAttachParent(USceneComponent* InParent)
{
    AttachParent = InParent;
}

UObject* USceneComponent::Duplicate() const
{
    USceneComponent* NewComp = FObjectFactory::ConstructObjectFrom<USceneComponent>(this);
    NewComp->DuplicateSubObjects(this); 
    NewComp->PostDuplicate();
    return NewComp;
}

void USceneComponent::DuplicateSubObjects(const UObject* Source)
{
    UActorComponent::DuplicateSubObjects(Source);
    // AttachParent는 AActor::DuplicateSubObjects에서 복원
}

void USceneComponent::PostDuplicate() {}