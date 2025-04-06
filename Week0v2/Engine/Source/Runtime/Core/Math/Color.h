#pragma once
#include "Vector4.h"

union FLinearColor  
{  
    float R, G, B, A;  

    // 기본 생성자  
    FLinearColor() : R(0), G(0), B(0), A(0) {}  
    FLinearColor(float InR, float InG, float InB, float InA = 1.0f)  
        : R(InR), G(InG), B(InB), A(InA) {}  

    // FColor(8비트 sRGB) → FLinearColor 변환  
    explicit FLinearColor(const FColor& Color);  
};  

static const FLinearColor Black = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
