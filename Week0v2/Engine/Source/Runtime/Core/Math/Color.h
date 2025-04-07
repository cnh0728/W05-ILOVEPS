#pragma once
#include "MathUtility.h"
#include "Vector.h"
#include "Vector4.h"
#include "HAL/PlatformType.h"

struct FLinearColor
{
    union 
    {
        struct
        {
            float R;
            float G;
            float B;
            float A;
        };
        float RGBA[4];
    };

    FLinearColor() : R(0), G(0), B(0), A(1) {}
    FLinearColor(const FLinearColor& Color) : R(Color.R), G(Color.G), B(Color.B), A(Color.A) {}
    FLinearColor(const FVector& Vector) : R(Vector.x), G(Vector.y), B(Vector.z), A(1.0f) {}
    FLinearColor(const FVector4& Vector4) : R(Vector4.x), G(Vector4.y), B(Vector4.z), A(Vector4.w) {}
    FLinearColor(float InR, float InG, float InB, float InA = 1.0f) : R(InR), G(InG), B(InB), A(InA) {}

    float& Component(int32 Index)
    {
        // @todo Implement Check Macro and Check btw 0~3
        return RGBA[Index];
    }
    const float& Component(int32 Index) const
    {
        return RGBA[Index];
    }

    FLinearColor CopyWithNewOpacity(float NewOpacity)
    {
        return {R, G, B, NewOpacity};
    }

    float Dist(const FLinearColor& V1, const FLinearColor& V2)
    {
        return FMath::Sqrt(FMath::Square(V1.R - V2.R) + FMath::Square(V1.G - V2.G) + FMath::Square(V1.B - V2.B));
    }

    bool Equals(const FLinearColor& ColorB, float Tolerance = KINDA_SMALL_NUMBER) const
    {
        return FMath::Abs(R - ColorB.R) <= Tolerance &&
            FMath::Abs(G - ColorB.G) <= Tolerance &&
            FMath::Abs(B - ColorB.B) <= Tolerance &&
            FMath::Abs(A - ColorB.A) <= Tolerance;
    }

    FLinearColor GetClamped(float InMin, float InMax) const
    {
        return FLinearColor(
            FMath::Clamp(R, InMin, InMax),
            FMath::Clamp(G, InMin, InMax),
            FMath::Clamp(B, InMin, InMax),
            FMath::Clamp(A, InMin, InMax)
        );
    }

    bool IsAlmostBlack() const
    {
        return FMath::Abs(R) <= KINDA_SMALL_NUMBER && FMath::Abs(G) <= KINDA_SMALL_NUMBER && FMath::Abs(B) <= KINDA_SMALL_NUMBER;
    }

    FLinearColor& operator=(const FLinearColor& Other)
    {
        if (this != &Other)
        {
            R = Other.R;
            G = Other.G;
            B = Other.B;
            A = Other.A;
        }
        return *this;
    }
    // Overload operators for color manipulation
    FLinearColor operator+(const FLinearColor& Other) const
    {
        return {R + Other.R, G + Other.G, B + Other.B, A + Other.A};
    }
    FLinearColor operator-(const FLinearColor& Other) const
    {
        return {R - Other.R, G - Other.G, B - Other.B, A - Other.A};
    }
    FLinearColor operator*(float Scalar) const
    {
        return {R * Scalar, G * Scalar, B * Scalar, A};
    }
    FLinearColor operator/(float Scalar) const
    {
        return {R / Scalar, G / Scalar, B / Scalar, A};
    }
    bool operator==(const FLinearColor& Other) const
    {
        return Equals(Other);
    }
    bool operator!=(const FLinearColor& Other) const
    {
        return !Equals(Other);
    }

    static const FLinearColor White;
};

const FLinearColor FLinearColor::White = FLinearColor(1.0f, 1.0f, 1.0f);