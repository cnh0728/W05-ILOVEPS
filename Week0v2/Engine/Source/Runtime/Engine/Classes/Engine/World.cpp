#include "World.h"

#include "FSceneBuilder.h"
#include "Actors/Player.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "Engine/FLoaderOBJ.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/SkySphereComponent.h"
#include "UnrealEd/SceneMgr.h"
#include "UObject/UObjectIterator.h"
#include "Level.h"
#include "Components/CubeComp.h"
#include "Fog/UFogComponent.h"


UWorld::UWorld(const UWorld& Other): UObject(Other)
                                   , defaultMapName(Other.defaultMapName)
                                   , Level(Other.Level)
                                   , WorldType(Other.WorldType)
                                   , EditorPlayer(Other.EditorPlayer)
{
}

void UWorld::InitWorld()
{
    // TODO: Load Scene
    FManagerOBJ::CreateStaticMesh("Assets/Cube.obj");
    FManagerOBJ::CreateStaticMesh("Assets/apple_mid.obj");
    Level = FObjectFactory::ConstructObject<ULevel>();
    CreateBaseObject();
    FogComponent = new UFogComponent();
    FogComponent->SetFogParams(5.0f,30.0f,0.0f,10.0f,FVector4(0.4f, 0.4f, 0.7f, 1.0f));
}

void UWorld::CreateTestScene()
{
    /*
    AStaticMeshActor* TempActor = SpawnActor<AStaticMeshActor>();
    TempActor->SetActorLabel(TEXT("OBJ_CUBE"));*/
    // 사과 배치
    SpawnAppleGrid(this, 10, 10, 1);

    // 방 구조체 배치 (중심: 0,0,0 / 크기: 500x500 / 높이: 300)
    SpawnBoxStructure(this, FVector(0, 0, -1.5f), FVector(500, 500, 0), 300.0f);
}
void UWorld::CreateBaseObject()
{
    if (EditorPlayer == nullptr)
    {
        EditorPlayer = FObjectFactory::ConstructObject<AEditorPlayer>();
    }
    
    if (LocalGizmo == nullptr)
    {
        LocalGizmo = FObjectFactory::ConstructObject<UTransformGizmo>();
    }


}


void UWorld::ReleaseBaseObject()
{
    if (LocalGizmo)
    {
        delete LocalGizmo;
        LocalGizmo = nullptr;
    }
    
    if (EditorPlayer)
    {
        delete EditorPlayer;
        EditorPlayer = nullptr;
    }

}

void UWorld::Tick(ELevelTick tickType, float deltaSeconds)
{
    if (tickType == LEVELTICK_ViewportsOnly)
    {
        if (EditorPlayer)
            EditorPlayer->Tick(deltaSeconds);
        if (LocalGizmo)
            LocalGizmo->Tick(deltaSeconds);
    }
    // SpawnActor()에 의해 Actor가 생성된 경우, 여기서 BeginPlay 호출
    if (tickType == LEVELTICK_All)
    {
        for (AActor* Actor : Level->PendingBeginPlayActors)
        {
            Actor->BeginPlay();
        }
        Level->PendingBeginPlayActors.Empty();

        // 매 틱마다 Actor->Tick(...) 호출
        for (AActor* Actor : Level->GetActors())
        {
            Actor->Tick(deltaSeconds);
        }
    }
}

void UWorld::Release()
{
	for (AActor* Actor : Level->GetActors())
	{
		Actor->EndPlay(EEndPlayReason::WorldTransition);
        TSet<UActorComponent*> Components = Actor->GetComponents();
	    for (UActorComponent* Component : Components)
	    {
	        GUObjectArray.MarkRemoveObject(Component);
	    }
	    GUObjectArray.MarkRemoveObject(Actor);
	}
    Level->GetActors().Empty();

	pickingGizmo = nullptr;
	ReleaseBaseObject();

    GUObjectArray.ProcessPendingDestroyObjects();
}

void UWorld::ClearScene()
{
    // 1. 모든 Actor Destroy
    
    for (AActor* actor : TObjectRange<AActor>())
    {
        DestroyActor(actor);
    }
    ReleaseBaseObject();
}

UObject* UWorld::Duplicate() const
{
    UWorld* CloneWorld = FObjectFactory::ConstructObjectFrom<UWorld>(this);
    GEngine->SetWorld(CloneWorld);
    CloneWorld->DuplicateSubObjects(this);
    CloneWorld->PostDuplicate();
    return CloneWorld;
}

void UWorld::DuplicateSubObjects(const UObject* SourceObj)
{
    UObject::DuplicateSubObjects(SourceObj);
    Level = Cast<ULevel>(Level->Duplicate());
    EditorPlayer = FObjectFactory::ConstructObject<AEditorPlayer>();
    //FogComponent = Cast<UFogComponent>(Cast<UWorld>(SourceObj)->GetFogComponent()->Duplicate());
    FogComponent = FObjectFactory::ConstructObject<UFogComponent>();
    FogComponent->SetFogParams(Cast<UWorld>(SourceObj)->GetFogComponent()->GetFogParams());
    
}

void UWorld::PostDuplicate()
{
    UObject::PostDuplicate();
}

void UWorld::ReloadScene(const FString& FileName)
{
    FString NewFile = GEngine->GetSceneManager()->LoadSceneFromFile(FileName);

    // if (SceneOctree && SceneOctree->GetRoot())
    //     SceneOctree->GetRoot()->TickBuffers(GCurrentFrame, 0);

    ClearScene(); // 기존 오브젝트 제거
    CreateBaseObject();
    GEngine->GetSceneManager()->ParseSceneData(NewFile);
}

bool UWorld::DestroyActor(AActor* ThisActor)
{
    if (ThisActor->GetWorld() == nullptr)
    {
        return false;
    }

    if (ThisActor->IsActorBeingDestroyed())
    {
        return true;
    }

    // 액터의 Destroyed 호출
    ThisActor->Destroyed();

    if (ThisActor->GetOwner())
    {
        ThisActor->SetOwner(nullptr);
    }

    TSet<UActorComponent*> Components = ThisActor->GetComponents();
    for (UActorComponent* Component : Components)
    {
        Component->DestroyComponent();
    }

    // World에서 제거
    Level->GetActors().Remove(ThisActor);

    // 제거 대기열에 추가
    GUObjectArray.MarkRemoveObject(ThisActor);
    return true;
}

void UWorld::SetPickingGizmo(UObject* Object)
{
	pickingGizmo = Cast<USceneComponent>(Object);
}

/*************************임시******************************/
bool UWorld::IsPIEWorld() const
{
    return false;
}

void UWorld::BeginPlay()
{

}

UWorld* UWorld::DuplicateWorldForPIE(UWorld* world)
{
    return new UWorld();
}
/**********************************************************/
void UWorld::RegisterLightComponent(ULightComponent* LightComp)
{
    if (LightComp && !WorldLightComponents.Contains(LightComp))
    {
        WorldLightComponents.Add(LightComp);
    }
}

void UWorld::UnregisterLightComponent(ULightComponent* LightComp)
{
    if (LightComp)
    {
        WorldLightComponents.Remove(LightComp);
    }
}