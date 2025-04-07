Texture2D Textures : register(t0);
SamplerState Sampler : register(s0);

cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 M;
    row_major float4x4 VP;
    row_major float4x4 MInverse;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
};

struct FMaterial
{
    float3 DiffuseColor;
    float TransparencyScalar;
    float3 AmbientColor;
    float DensityScalar;
    float3 SpecularColor;
    float SpecularScalar;
    float3 EmissiveColor;
    float MaterialPad0;
};

cbuffer MaterialConstants : register(b1)
{
    FMaterial Material;
}

cbuffer LightingConstants : register(b2)
{
    float3 LightDirection; // 조명 방향 (단위 벡터; 빛이 들어오는 방향의 반대 사용)
    float LightPad0; // 16바이트 정렬용 패딩
    float3 LightColor; // 조명 색상 (예: (1, 1, 1))
    float LightPad1; // 16바이트 정렬용 패딩
    float AmbientFactor; // ambient 계수 (예: 0.1)
    float3 LightPad2; // 16바이트 정렬 맞춤 추가 패딩
};

cbuffer FlagConstants : register(b3)
{
    bool IsLit;
    float3 flagPad0;
}

cbuffer SubMeshConstants : register(b4)
{
    bool IsSelectedSubMesh;
    float3 SubMeshPad0;
}

cbuffer TextureConstants : register(b5)
{
    float2 UVOffset;
    float2 TexturePad0;
}

struct PS_INPUT
{
    float4 position : SV_POSITION; // 변환된 화면 좌표
    float3 worldPos : TEXCOORD0;
    float4 color : COLOR; // 전달할 색상
    float3 normal : NORMAL; // 정규화된 노멀 벡터
    bool normalFlag : TEXCOORD1; // 노멀 유효성 플래그 (1.0: 유효, 0.0: 무효)
    float2 texcoord : TEXCOORD2;
    int materialIndex : MATERIAL_INDEX;
};


struct PS_OUTPUT
{
    float4 position : SV_Target0;  // 월드 좌표 (R32G32B32A32_FLOAT)  
    float4 normal   : SV_Target1;  // 정규화 노멀 (R16G16B16A16_FLOAT)  
    float4 albedo   : SV_Target2;  // 선형 공간 Albedo (R8G8B8A8_UNORM_SRGB)  
    float4 material : SV_Target3;  // 메탈릭/러프니스 (R8G8B8A8_UNORM)  
};

float noise(float3 p)
{
    return frac(sin(dot(p, float3(12.9898, 78.233, 37.719))) * 43758.5453);
}

float4 PaperTexture(float3 originalColor)
{
    // 입력 색상을 0~1 범위로 제한
    float3 color = saturate(originalColor);
    
    float3 paperColor = float3(0.95, 0.95, 0.95);
    float blendFactor = 0.5;
    float3 mixedColor = lerp(color, paperColor, blendFactor);
    
    // 정적 grain 효과
    float grain = noise(color * 10.0) * 0.1;
    
    // 거친 질감 효과: 두 단계의 노이즈 레이어를 결합
    float rough1 = (noise(color * 20.0) - 0.5) * 0.15; // 첫 번째 레이어: 큰 규모의 노이즈
    float rough2 = (noise(color * 40.0) - 0.5) * 0.01; // 두 번째 레이어: 세부 질감 추가
    float rough = rough1 + rough2;
    
    // vignette 효과: 중앙에서 멀어질수록 어두워지는 효과
    float vignetting = smoothstep(0.4, 1.0, length(color.xy - 0.5) * 2.0);
    
    // 최종 색상 계산
    float3 finalColor = mixedColor + grain + rough - vignetting * 0.1;
    return float4(saturate(finalColor), 1.0);
}

PS_OUTPUT mainPS(PS_INPUT input)
{
    PS_OUTPUT output;

    // 1. position 버퍼 (월드 좌표)
    output.position = float4(input.worldPos, 1.0);

    // 2. Normal 버퍼 (범위 변환 -> Texture는 음수저장이 안되기 때문에 나중에 사용할때 역변환해서 사용)  
    output.normal = float4(input.normal * 0.5 + 0.5, 1.0);  

    // 3. Albedo 버퍼 (sRGB -> Linear 변환)
    float3 texColor = Textures.Sample(Sampler, input.texcoord + UVOffset);
    float3 baseColor = texColor.g == 0?Material.DiffuseColor : texColor + Material.DiffuseColor;
    baseColor = saturate(baseColor);

    //선택 상태 하이라이트
    if (isSelected)
    {
        baseColor += float3(0.2f, 0.2f, 0.0f); // 노란색 틴트로 하이라이트
        if (IsSelectedSubMesh) baseColor = float3(1, 1, 1);
    }
    
    // sRGB → Linear 변환  
    float3 linearColor = pow(baseColor, 2.2);
    output.albedo = float4(linearColor, Material.TransparencyScalar);

    // 4. Material 버퍼 (Metallic/Roughness)
    // 현재 Material 구조에 Metallic/Roughness가 없으므로 임시값 사용
    output.material = float4(0.0, 0.5, 0.0, 1.0); //R:Metallic, G:Roughness

    // 5. 라이팅 계산 (조명 영향 적용)

    if (input.normalFlag)
    {
        float3 N = normalize(input.normal);
        float3 L = normalize(LightDirection);
        float3 V = normalize(float3(0, 0, 1) - input.worldPos);
        
        // 기본 디퓨즈 계산
        float diffuse = saturate(dot(N, L));
        
        // 스페큘러 계산 (간단한 Blinn-Phong)
        float3 H = normalize(L + V);
        float specular = pow(saturate(dot(N, H)), Material.SpecularScalar * 32) * Material.SpecularScalar;
        
        // 라이느 누적
        float3 ambient = Material.AmbientColor * AmbientFactor;
        float3 diffuseLight = diffuse * LightColor;
        float3 specularLight = specular * Material.SpecularColor * LightColor;

        //Albedo에 라이팅 적용
        output.albedo.rgb = ambient + (diffuseLight * linearColor) + specularLight + Material.EmissiveColor;
    }
    return output;
}