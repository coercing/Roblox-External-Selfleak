#include "Theme.h"

namespace Menu::Theme
{
    static float gAccentColor[4] = { 1.0f, 0.62f, 0.78f, 1.0f };
    static float gMenuAlpha = 1.0f;
    static float gOverlayDarkness = 0.65f;

    float Clamp(float value, float min, float max)
    {
        if (value < min)
        {
            return min;
        }
        if (value > max)
        {
            return max;
        }
        return value;
    }

    ImU32 Accent(int alpha)
    {
        return IM_COL32(
            static_cast<int>(gAccentColor[0] * 255.0f),
            static_cast<int>(gAccentColor[1] * 255.0f),
            static_cast<int>(gAccentColor[2] * 255.0f),
            static_cast<int>(alpha * gMenuAlpha)
        );
    }

    ImU32 AccentSoft(int alpha)
    {
        float softR = gAccentColor[0] + (1.0f - gAccentColor[0]) * 0.3f;
        float softG = gAccentColor[1] + (1.0f - gAccentColor[1]) * 0.3f;
        float softB = gAccentColor[2] + (1.0f - gAccentColor[2]) * 0.3f;
        return IM_COL32(
            static_cast<int>(softR * 255.0f),
            static_cast<int>(softG * 255.0f),
            static_cast<int>(softB * 255.0f),
            static_cast<int>(alpha * gMenuAlpha)
        );
    }

    ImU32 Base(int alpha)
    {
        return IM_COL32(17, 18, 22, static_cast<int>(alpha * gMenuAlpha));
    }

    ImU32 Surface(int alpha)
    {
        return IM_COL32(21, 22, 27, static_cast<int>(alpha * gMenuAlpha));
    }

    ImU32 SurfaceLight(int alpha)
    {
        return IM_COL32(26, 27, 33, static_cast<int>(alpha * gMenuAlpha));
    }

    ImU32 Control(int alpha)
    {
        return IM_COL32(12, 13, 17, static_cast<int>(alpha * gMenuAlpha));
    }

    ImU32 Outline(int alpha)
    {
        return IM_COL32(38, 39, 47, static_cast<int>(alpha * gMenuAlpha));
    }

    ImU32 Text(int alpha)
    {
        return IM_COL32(238, 238, 242, static_cast<int>(alpha * gMenuAlpha));
    }

    ImU32 MutedText(int alpha)
    {
        return IM_COL32(118, 120, 126, static_cast<int>(alpha * gMenuAlpha));
    }

    ImVec4 BaseVec(float alpha)
    {
        return ImVec4(0.067f, 0.071f, 0.086f, alpha);
    }

    ImVec4 SurfaceVec(float alpha)
    {
        return ImVec4(0.082f, 0.086f, 0.106f, alpha);
    }

    ImVec4 ControlVec(float alpha)
    {
        return ImVec4(0.047f, 0.051f, 0.067f, alpha);
    }

    ImVec4 AccentVec(float alpha)
    {
        return ImVec4(gAccentColor[0], gAccentColor[1], gAccentColor[2], alpha);
    }

    void SetAccentColor(const float color[4])
    {
        gAccentColor[0] = color[0];
        gAccentColor[1] = color[1];
        gAccentColor[2] = color[2];
        gAccentColor[3] = color[3];
    }

    void SetMenuAlpha(float alpha)
    {
        gMenuAlpha = alpha;
    }

    void ApplyStyle()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 12.0f;
        style.WindowBorderSize = 0.0f;
        style.FrameRounding = 12.0f;
        style.FrameBorderSize = 0.0f;
        style.ChildRounding = 12.0f;
        style.PopupRounding = 12.0f;
        style.ScrollbarRounding = 12.0f;
        style.GrabRounding = 12.0f;
        style.Colors[ImGuiCol_WindowBg] = BaseVec();
        style.Colors[ImGuiCol_ChildBg] = BaseVec();
        style.Colors[ImGuiCol_PopupBg] = SurfaceVec();
        style.Colors[ImGuiCol_Border] = ImGui::ColorConvertU32ToFloat4(Outline());
        style.Colors[ImGuiCol_FrameBg] = ControlVec();
        style.Colors[ImGuiCol_FrameBgHovered] = SurfaceVec();
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.12f, 0.082f, 0.104f, 1.0f);
        style.Colors[ImGuiCol_CheckMark] = AccentVec();
        style.Colors[ImGuiCol_SliderGrab] = AccentVec();
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.74f, 0.86f, 1.0f);
        style.Colors[ImGuiCol_Button] = ControlVec();
        style.Colors[ImGuiCol_ButtonHovered] = SurfaceVec();
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.12f, 0.082f, 0.104f, 1.0f);
        style.Colors[ImGuiCol_Header] = AccentVec(0.22f);
        style.Colors[ImGuiCol_HeaderHovered] = AccentVec(0.32f);
        style.Colors[ImGuiCol_HeaderActive] = AccentVec(0.42f);
    }

    float GetOverlayDarkness() { return gOverlayDarkness; }
    void SetOverlayDarkness(float val) { gOverlayDarkness = val; }
}
