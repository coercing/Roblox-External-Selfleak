#pragma once

#include <imgui/imgui.h>

namespace Menu::Theme
{
    float Clamp(float value, float min, float max);
    ImU32 Accent(int alpha = 255);
    ImU32 AccentSoft(int alpha = 255);
    ImU32 Base(int alpha = 255);
    ImU32 Surface(int alpha = 255);
    ImU32 SurfaceLight(int alpha = 255);
    ImU32 Control(int alpha = 255);
    ImU32 Outline(int alpha = 255);
    ImU32 Text(int alpha = 255);
    ImU32 MutedText(int alpha = 255);
    ImVec4 BaseVec(float alpha = 1.0f);
    ImVec4 SurfaceVec(float alpha = 1.0f);
    ImVec4 ControlVec(float alpha = 1.0f);
    ImVec4 AccentVec(float alpha = 1.0f);
    void SetAccentColor(const float color[4]);
    void SetMenuAlpha(float alpha);
    float GetOverlayDarkness();
    void SetOverlayDarkness(float val);
    void ApplyStyle();
}
