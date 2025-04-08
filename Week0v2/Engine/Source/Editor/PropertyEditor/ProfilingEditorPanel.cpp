
#include "ProfilingEditorPanel.h"

#include "EditorEngine.h"
#include "LaunchEngineLoop.h"
#include "Profiling/PlatformTime.h"
#include "Profiling/StatRegistry.h"
#include "Renderer/PostProcess/CompositePostProcess.h"

void ProfilingEditorPanel::Render()
{
    ImGui::SetNextWindowPos(ImVec2(10, 50), ImGuiCond_Always);
    //ImGui::SetNextWindowBgAlpha(0.35f); // 반투명 배경
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1)); // 완전한 검정 배경
    if (ImGui::Begin("Performance", nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoBringToFrontOnFocus
                     ))
    {   
        // FPS 표시
        const auto& StatMap = FStatRegistry::GetStatMap();
        static TStatId Stat_Frame("MainFrame");
        auto Stats = FStatRegistry::GetFPSStats(Stat_Frame);
        float fps = static_cast<float>(FStatRegistry::GetFPS(Stat_Frame));
        float ms = static_cast<float>(FStatRegistry::GetLastMilliseconds(Stat_Frame));
        ImGui::Text("FPS: %.2f (%.3f)ms", fps,ms);
        //ImGui::Text("Picking Time %.4fms\nNum Attempts: %d\nAccumulated Time %.2fms",FStatRegistry::GetLastMilliseconds("Picking"),FStatRegistry::TotalPickCount,FStatRegistry::TotalPickTime);
        ImGui::Text("FPS (1s): %.2f", Stats.FPS_1Sec);
        ImGui::Text("FPS (5s): %.2f", Stats.FPS_5Sec);
        const char* modeNames[] = { "Composite", "FogOnly", "DepthOnly" };
        static int selectedMode = static_cast<int>(EPostProcessViewMode::Composite);
        if (ImGui::Combo("Post Process View Mode", &selectedMode, modeNames, IM_ARRAYSIZE(modeNames)))
        {
            UEditorEngine::renderer.FinalComposite->SetViewMode(static_cast<EPostProcessViewMode>(selectedMode));
        }
        // 드롭다운으로 StatMap 표시
        if (ImGui::CollapsingHeader("Stat Timings (ms)", ImGuiTreeNodeFlags_DefaultOpen))
        {
             // 함수로 접근한다고 가정
            for (const auto& Pair : StatMap)
            {
                const uint32 StatKey = Pair.Key;
                double Ms = Pair.Value;

                // DisplayIndex로 FName을 재생성 → 이름 얻기
                FName StatName(StatKey); // FName(uint32 DisplayIndex) 생성자 있어야 함
                FString NameString = StatName.ToString();

                ImGui::Text("%s: %.3f ms", GetData(NameString), Ms);
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();
}
void ProfilingEditorPanel::OnResize(HWND hWnd)
{
    /*
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;*/
}
