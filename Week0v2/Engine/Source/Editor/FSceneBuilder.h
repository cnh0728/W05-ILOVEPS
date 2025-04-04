#pragma once
#include "HAL/PlatformType.h"

class UWorld;
struct FVector;
void SpawnBoxStructure(
    UWorld* World,
    const FVector& center,
    const FVector& extent,       // 바닥 기준 반지름
    float wallHeight,
    float thickness = 1.0f,
    const FWString& meshName = L"Cube.obj");
void SpawnAppleGrid(UWorld* World,int x,int y,int z);