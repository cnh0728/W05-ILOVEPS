#pragma once

#include "HAL/PlatformType.h"
#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"

//사용 X
enum class EFogType : uint8
{
    Linear,
    Height,
    Exponential,
    Custom
};

// Fog 파라미터 공통 구조체
struct FFogParams
{
    FMatrix InvViewProj;
    FVector CameraWorldPos;  // 카메라 위치 (World 기준)

    float LinearStart = 20.0f;   // 거리 기반 Fog 시작 거리
    float LinearEnd = 300.0f;    // 거리 기반 Fog 끝 거리

    float HeightStart = 0.0f;    // 높이 기반 Fog 시작 높이 (보통 바닥)
    float HeightEnd = 10.0f;    // 높이 기반 Fog 끝 높이 (보통 천장)
    float Padding0 = 0.0f;      // 4 bytes (패딩)

    FVector4 FogColor = FVector4(0.4f, 0.4f, 0.7f, 1.0f);  // 최종 Fog 색상
};

