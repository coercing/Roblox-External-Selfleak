#pragma once

#include <cstddef>

#include <imgui/imgui.h>

namespace Menu::Widgets
{
    bool Panel(ImDrawList* drawList, const char* id, ImVec2 pos, ImVec2 size, const char* title, bool* expanded);
    void Checkbox(ImDrawList* drawList, const char* id, ImVec2 pos, const char* label, bool* value);
    void SliderFloat(ImDrawList* drawList, const char* id, ImVec2 pos, float width, const char* label, float* value, float min, float max, const char* format, float* visual);
    void SliderInt(ImDrawList* drawList, const char* id, ImVec2 pos, float width, const char* label, int* value, int min, int max, const char* format, float* visual);
    void Combo(ImDrawList* drawList, const char* id, ImVec2 pos, float rowWidth, const char* label, int* value, const char* const items[], int count);
    void Input(ImDrawList* drawList, const char* id, ImVec2 pos, float width, const char* label, char* buffer, std::size_t bufferSize);
    void ColorPicker(ImDrawList* drawList, const char* id, ImVec2 pos, float rowWidth, const char* label, float color[4]);
    void KeyBind(ImDrawList* drawList, const char* id, ImVec2 pos, float width, const char* label, int* key);
    void KeyBindEx(ImDrawList* drawList, const char* id, ImVec2 pos, float width, const char* label, int* key, int* mode);
    bool MenuButton(ImDrawList* drawList, const char* id, ImVec2 pos, ImVec2 size, const char* label, ImU32 accent = 0, bool isSmall = false);
}
