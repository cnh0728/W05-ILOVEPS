#include "FSceneBuilder.h"

#include "Components/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ULightComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

void SpawnAppleGrid(UWorld* World, int x, int y, int z)
{
    const float spacing = 5.0f; // 사과 간 간격

    for (int i = 0; i < x; i++)
    {
        for (int j = 0; j < y; j++)
        {
            for (int k = 0; k < z; k++)
            {
                AActor* appleActor = World->SpawnActor<AActor>();
                appleActor->SetActorLabel(TEXT("OBJ_Apple"));
                appleActor->AddComponent<UProjectileMovementComponent>();

                UStaticMeshComponent* apple = FObjectFactory::ConstructObject<UStaticMeshComponent>();
                apple->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"apple_mid.obj"));
                apple->SetLocation(FVector(i * spacing, j * spacing, k * spacing));
                appleActor->AddComponent(apple);

                ULightComponent* LightComponent = appleActor->AddComponent<ULightComponent>();

                // 랜덤 색상 생성
                float r = static_cast<float>(rand()) / RAND_MAX;
                float g = static_cast<float>(rand()) / RAND_MAX;
                float b = static_cast<float>(rand()) / RAND_MAX;
                FVector color(r, g, b);

                LightComponent->SetLight(4.0f, color, 2.0f);
                LightComponent->SetColorCycling(true);
            }
        }
    }
}
void SpawnBoxStructure(
    UWorld* World,
    const FVector& center,
    const FVector& extent,       // 바닥 기준 반지름
    float wallHeight,
    float thickness,
    const FWString& meshName)
{
    AActor* Actor = World->SpawnActor<AActor>();
    Actor->SetActorLabel(TEXT("OBJ_Room"));
    // 바닥
    UStaticMeshComponent* floor = FObjectFactory::ConstructObject<UStaticMeshComponent>();
    floor->SetStaticMesh(FManagerOBJ::GetStaticMesh(meshName));
    floor->SetLocation(FVector(center.x, center.y, center.z));
    floor->SetScale(FVector(extent.x, extent.y, thickness));
    Actor->AddComponent(floor);
    // 천장
    UStaticMeshComponent* ceiling = FObjectFactory::ConstructObject<UStaticMeshComponent>();
    ceiling->SetStaticMesh(FManagerOBJ::GetStaticMesh(meshName));
    ceiling->SetLocation(FVector(center.x, center.y, center.z + wallHeight));
    ceiling->SetScale(FVector(extent.x, extent.y, thickness));
    Actor = World->SpawnActor<AActor>();
    Actor->AddComponent(ceiling);

    // 벽 1 (앞)
    UStaticMeshComponent* wall1 = FObjectFactory::ConstructObject<UStaticMeshComponent>();
    wall1->SetStaticMesh(FManagerOBJ::GetStaticMesh(meshName));
    wall1->SetLocation(FVector(center.x + extent.x, center.y, center.z + wallHeight * 0.5f));
    wall1->SetScale(FVector(thickness, extent.y, wallHeight * 0.5f));
    Actor = World->SpawnActor<AActor>();
    Actor->AddComponent(wall1);

    // 벽 2 (뒤)
    UStaticMeshComponent* wall2 = FObjectFactory::ConstructObject<UStaticMeshComponent>();
    wall2->SetStaticMesh(FManagerOBJ::GetStaticMesh(meshName));
    wall2->SetLocation(FVector(center.x - extent.x, center.y, center.z + wallHeight * 0.5f));
    wall2->SetScale(FVector(thickness, extent.y, wallHeight * 0.5f));
    Actor = World->SpawnActor<AActor>();
    Actor->AddComponent(wall2);

    // 벽 3 (왼쪽)
    UStaticMeshComponent* wall3 = FObjectFactory::ConstructObject<UStaticMeshComponent>();
    wall3->SetStaticMesh(FManagerOBJ::GetStaticMesh(meshName));
    wall3->SetLocation(FVector(center.x, center.y + extent.y, center.z + wallHeight * 0.5f));
    wall3->SetScale(FVector(extent.x, thickness, wallHeight * 0.5f));
    Actor = World->SpawnActor<AActor>();
    Actor->AddComponent(wall3);

    // 벽 4 (오른쪽)
    UStaticMeshComponent* wall4 = FObjectFactory::ConstructObject<UStaticMeshComponent>();
    wall4->SetStaticMesh(FManagerOBJ::GetStaticMesh(meshName));
    wall4->SetLocation(FVector(center.x, center.y - extent.y, center.z + wallHeight * 0.5f));
    wall4->SetScale(FVector(extent.x, thickness, wallHeight * 0.5f));
    Actor = World->SpawnActor<AActor>();
    Actor->AddComponent(wall4);
}
