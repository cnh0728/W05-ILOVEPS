#pragma once
#include "Core/HAL/PlatformType.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Engine/Engine.h"
#include "Renderer/Renderer.h"
#include "Engine/ResourceMgr.h"


class FSceneMgr;
class UnrealEd;
class UImGuiManager;
class UWorld;
class FEditorViewportClient;
class SSplitterV;
class SSplitterH;
class SLevelEditor;

extern UWorld* GWorld;

enum class EFrameMode
{
    Normal,
    Depth,
};

class UEditorEngine : public UEngine
{
    DECLARE_CLASS(UEditorEngine, UEngine)
    
public:
    UEditorEngine();
    int32 Init(HWND hwnd);
    void Render();
    void Tick(float deltaSeconds);
    void Exit();
    float GetAspectRatio(IDXGISwapChain* swapChain) const;
    void Input();

    void PreparePIE();
    void StartPIE();
    void PausedPIE();
    void ResumingPIE();
    void StopPIE();

private:
    void WindowInit(HINSTANCE hInstance);

public:
    static FGraphicsDevice graphicDevice;
    static FRenderer renderer;
    static FResourceMgr resourceMgr;
    
    HWND hWnd;
    ELevelTick levelType = ELevelTick::LEVELTICK_ViewportsOnly;
private:
    UImGuiManager* UIMgr;
    UWorld* GWorld;
    SLevelEditor* LevelEditor;
    UnrealEd* UnrealEditor;
    FSceneMgr* SceneMgr;

    // EFrameMode FrameMode = EFrameMode::Normal;
    EFrameMode FrameMode = EFrameMode::Depth;
    
    bool bTestInput = false;
public:
    void SetFrameMode(EFrameMode ChangeMode) { FrameMode = ChangeMode; }
    
    EFrameMode GetFrameMode() { return FrameMode; }
    UWorld* GetWorld() const { return GWorld; }
    void SetWorld(UWorld* NewWorld) { GWorld = NewWorld; }
    SLevelEditor* GetLevelEditor() const { return LevelEditor; }
    UnrealEd* GetUnrealEditor() const { return UnrealEditor; }
    FSceneMgr* GetSceneManager() const { return SceneMgr; }
};
extern UEditorEngine* GEngine;
