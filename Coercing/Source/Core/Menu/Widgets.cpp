#include "Widgets.h"

#include <windows.h>
#include <cstdio>
#include <cmath>
#include <cstring>

#include "Theme.h"

static const char* GetKeyName(int vk)
{
    switch (vk)
    {
    case 0: return "none";
    case VK_LBUTTON: return "mouse 1";
    case VK_RBUTTON: return "mouse 2";
    case VK_MBUTTON: return "mouse 3";
    case VK_XBUTTON1: return "mouse 4";
    case VK_XBUTTON2: return "mouse 5";
    case VK_BACK: return "back";
    case VK_TAB: return "tab";
    case VK_CLEAR: return "clear";
    case VK_RETURN: return "enter";
    case VK_SHIFT: return "shift";
    case VK_CONTROL: return "ctrl";
    case VK_MENU: return "alt";
    case VK_PAUSE: return "pause";
    case VK_CAPITAL: return "caps";
    case VK_ESCAPE: return "esc";
    case VK_SPACE: return "space";
    case VK_PRIOR: return "page up";
    case VK_NEXT: return "page down";
    case VK_END: return "end";
    case VK_HOME: return "home";
    case VK_LEFT: return "left";
    case VK_UP: return "up";
    case VK_RIGHT: return "right";
    case VK_DOWN: return "down";
    case VK_SNAPSHOT: return "print";
    case VK_INSERT: return "ins";
    case VK_DELETE: return "del";
    case VK_LWIN: return "win";
    case VK_RWIN: return "win";
    case VK_NUMLOCK: return "numlk";
    case VK_SCROLL: return "scroll";
    case VK_LSHIFT: return "shift";
    case VK_RSHIFT: return "shift";
    case VK_LCONTROL: return "ctrl";
    case VK_RCONTROL: return "ctrl";
    case VK_LMENU: return "alt";
    case VK_RMENU: return "alt";
    case VK_F1: return "f1";
    case VK_F2: return "f2";
    case VK_F3: return "f3";
    case VK_F4: return "f4";
    case VK_F5: return "f5";
    case VK_F6: return "f6";
    case VK_F7: return "f7";
    case VK_F8: return "f8";
    case VK_F9: return "f9";
    case VK_F10: return "f10";
    case VK_F11: return "f11";
    case VK_F12: return "f12";
    default:
        if (vk >= '0' && vk <= '9')
        {
            static char buf[2] = {};
            buf[0] = static_cast<char>(vk);
            return buf;
        }
        if (vk >= 'A' && vk <= 'Z')
        {
            static char buf[2] = {};
            buf[0] = static_cast<char>(vk);
            return buf;
        }
        return "?";
    }
}

namespace Menu::Widgets
{
    bool Panel(ImDrawList* drawList, const char* id, ImVec2 pos, ImVec2 size, const char* title, bool* expanded)
    {
        drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), Theme::Surface(255), 4.0f);
        drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), Theme::Outline(180), 4.0f);
        drawList->AddLine(ImVec2(pos.x, pos.y + 30.0f), ImVec2(pos.x + size.x, pos.y + 30.0f), Theme::Outline(145), 1.0f);
        drawList->AddText(ImVec2(pos.x + 12.0f, pos.y + 8.0f), Theme::Text(), title);

        ImGui::PushID(id);
        ImVec2 buttonPos(pos.x + size.x - 28.0f, pos.y + 5.0f);
        ImGui::SetCursorScreenPos(buttonPos);
        ImGui::InvisibleButton("##panel_toggle", ImVec2(22.0f, 20.0f));
        bool hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked())
        {
            *expanded = !*expanded;
        }
        ImGui::PopID();

        ImU32 arrowColor = hovered ? Theme::Accent() : Theme::Text();
        drawList->AddText(ImVec2(pos.x + size.x - 22.0f, pos.y + 8.0f), arrowColor, *expanded ? (const char*)u8"\uf0d7" : (const char*)u8"\uf0da");
        return *expanded;
    }

    void Checkbox(ImDrawList* drawList, const char* id, ImVec2 pos, const char* label, bool* value)
    {
        const float boxSize = 17.0f;
        ImVec2 textSize = ImGui::CalcTextSize(label);
        ImGui::PushID(id);
        ImGui::SetCursorScreenPos(pos);
        ImGui::InvisibleButton("##checkbox", ImVec2(boxSize + 9.0f + textSize.x, boxSize));
        bool hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked())
        {
            *value = !*value;
        }
        ImGui::PopID();

        ImU32 boxColor = *value ? Theme::Accent(230) : Theme::Control();
        ImU32 outlineColor = *value ? Theme::AccentSoft(180) : hovered ? Theme::Accent(115) : Theme::Outline();
        ImU32 labelColor = (*value || hovered) ? Theme::Text() : Theme::MutedText();
        drawList->AddRectFilled(ImVec2(pos.x + 0.5f, pos.y + 0.5f), ImVec2(pos.x + boxSize - 0.5f, pos.y + boxSize - 0.5f), boxColor, 4.0f);
        drawList->AddRect(ImVec2(pos.x + 0.5f, pos.y + 0.5f), ImVec2(pos.x + boxSize - 0.5f, pos.y + boxSize - 0.5f), outlineColor, 4.0f);
        if (*value)
        {
            drawList->AddLine(ImVec2(pos.x + 4.5f, pos.y + 8.0f), ImVec2(pos.x + 7.0f, pos.y + 10.5f), Theme::Base(), 2.0f);
            drawList->AddLine(ImVec2(pos.x + 7.0f, pos.y + 10.5f), ImVec2(pos.x + 12.5f, pos.y + 5.5f), Theme::Base(), 2.0f);
        }
        drawList->AddText(ImVec2(pos.x + boxSize + 9.0f, pos.y + (boxSize - textSize.y) * 0.5f), labelColor, label);
    }

    void SliderFloat(ImDrawList* drawList, const char* id, ImVec2 pos, float width, const char* label, float* value, float min, float max, const char* format, float* visual)
    {
        char valueText[32]{};
        std::snprintf(valueText, sizeof(valueText), format, *value);

        ImGui::PushID(id);
        ImGui::SetCursorScreenPos(pos);
        ImGui::InvisibleButton("##slider", ImVec2(width, 34.0f));
        bool hovered = ImGui::IsItemHovered();
        bool active = ImGui::IsItemActive();
        if (active)
        {
            float t = Theme::Clamp((ImGui::GetIO().MousePos.x - pos.x) / width, 0.0f, 1.0f);
            *value = min + (max - min) * t;
        }
        ImGui::PopID();

        float t = Theme::Clamp((*value - min) / (max - min), 0.0f, 1.0f);
        *visual += (t - *visual) * (1.0f - std::exp(-ImGui::GetIO().DeltaTime * 18.0f));
        ImVec2 valueSize = ImGui::CalcTextSize(valueText);
        ImVec2 trackMin(pos.x, pos.y + 24.0f);
        ImVec2 trackMax(pos.x + width, pos.y + 32.0f);
        ImU32 labelColor = (hovered || active) ? Theme::Text() : Theme::MutedText();
        float fillX = trackMin.x + width * Theme::Clamp(*visual, 0.0f, 1.0f);

        drawList->AddText(pos, labelColor, label);
        drawList->AddText(ImVec2(pos.x + width - valueSize.x, pos.y), labelColor, valueText);
        drawList->AddRectFilled(trackMin, trackMax, Theme::Control(), 4.0f);
        drawList->AddRect(trackMin, trackMax, Theme::Outline(), 4.0f);
        drawList->AddRectFilled(trackMin, ImVec2(fillX > trackMin.x + 5.0f ? fillX : trackMin.x + 5.0f, trackMax.y), Theme::Accent(215), 4.0f);
        drawList->AddCircleFilled(ImVec2(fillX, (trackMin.y + trackMax.y) * 0.5f), active ? 6.0f : 5.0f, Theme::AccentSoft(), 24);
    }

    void SliderInt(ImDrawList* drawList, const char* id, ImVec2 pos, float width, const char* label, int* value, int min, int max, const char* format, float* visual)
    {
        char valueText[32]{};
        std::snprintf(valueText, sizeof(valueText), format, *value);

        ImGui::PushID(id);
        ImGui::SetCursorScreenPos(pos);
        ImGui::InvisibleButton("##slider", ImVec2(width, 34.0f));
        bool hovered = ImGui::IsItemHovered();
        bool active = ImGui::IsItemActive();
        if (active)
        {
            float t = Theme::Clamp((ImGui::GetIO().MousePos.x - pos.x) / width, 0.0f, 1.0f);
            *value = min + static_cast<int>((max - min) * t + 0.5f);
        }
        ImGui::PopID();

        float t = Theme::Clamp((static_cast<float>(*value) - static_cast<float>(min)) / static_cast<float>(max - min), 0.0f, 1.0f);
        *visual += (t - *visual) * (1.0f - std::exp(-ImGui::GetIO().DeltaTime * 18.0f));
        ImVec2 valueSize = ImGui::CalcTextSize(valueText);
        ImVec2 trackMin(pos.x, pos.y + 24.0f);
        ImVec2 trackMax(pos.x + width, pos.y + 32.0f);
        ImU32 labelColor = (hovered || active) ? Theme::Text() : Theme::MutedText();
        float fillX = trackMin.x + width * Theme::Clamp(*visual, 0.0f, 1.0f);

        drawList->AddText(pos, labelColor, label);
        drawList->AddText(ImVec2(pos.x + width - valueSize.x, pos.y), labelColor, valueText);
        drawList->AddRectFilled(trackMin, trackMax, Theme::Control(), 4.0f);
        drawList->AddRect(trackMin, trackMax, Theme::Outline(), 4.0f);
        drawList->AddRectFilled(trackMin, ImVec2(fillX > trackMin.x + 5.0f ? fillX : trackMin.x + 5.0f, trackMax.y), Theme::Accent(215), 4.0f);
        drawList->AddCircleFilled(ImVec2(fillX, (trackMin.y + trackMax.y) * 0.5f), active ? 6.0f : 5.0f, Theme::AccentSoft(), 24);
    }

    void Combo(ImDrawList* drawList, const char* id, ImVec2 pos, float rowWidth, const char* label, int* value, const char* const items[], int count)
    {
        static ImGuiID animatedComboId = 0;
        static float comboPopupAlpha = 0.0f;
        const float height = 24.0f;
        const float itemHeight = 23.0f;
        const float spacing = 6.0f;
        ImVec2 labelSize = ImGui::CalcTextSize(label);
        ImVec2 fieldMin(pos.x, pos.y + labelSize.y + spacing);
        ImVec2 fieldMax(fieldMin.x + rowWidth, fieldMin.y + height);

        ImGui::PushID(id);
        ImGuiID comboId = ImGui::GetID("##combo");
        ImGui::SetCursorScreenPos(fieldMin);
        ImGui::InvisibleButton("##combo", ImVec2(rowWidth, height));
        bool hovered = ImGui::IsItemHovered();
        bool popupOpen = ImGui::IsPopupOpen("##popup", ImGuiPopupFlags_None);
        if (ImGui::IsItemClicked())
        {
            ImGui::OpenPopup("##popup");
            animatedComboId = comboId;
            comboPopupAlpha = 0.0f;
            popupOpen = true;
        }
        if (popupOpen && animatedComboId != comboId)
        {
            animatedComboId = comboId;
            comboPopupAlpha = 0.0f;
        }
        if (animatedComboId == comboId)
        {
            float target = popupOpen ? 1.0f : 0.0f;
            comboPopupAlpha += (target - comboPopupAlpha) * (1.0f - std::exp(-ImGui::GetIO().DeltaTime * 18.0f));
        }

        drawList->AddText(pos, (hovered || popupOpen) ? Theme::Text() : Theme::MutedText(), label);
        drawList->AddRectFilled(fieldMin, fieldMax, Theme::Control(), 4.0f);
        drawList->AddRect(fieldMin, fieldMax, Theme::Outline(), 4.0f);
        drawList->AddText(ImVec2(fieldMin.x + 8.0f, fieldMin.y + (height - ImGui::GetFontSize()) * 0.5f - 1.0f), Theme::Text(), items[*value]);
        ImVec2 arrowSize = ImGui::CalcTextSize((const char*)u8"\uf107");
        drawList->AddText(ImVec2(fieldMax.x - 13.0f - arrowSize.x * 0.5f, fieldMin.y + (height - arrowSize.y) * 0.5f), popupOpen ? Theme::Accent(230) : Theme::MutedText(180), (const char*)u8"\uf107");

        ImGui::SetNextWindowPos(ImVec2(fieldMin.x, fieldMax.y + 4.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(rowWidth, 8.0f + itemHeight * count));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, Theme::Clamp(comboPopupAlpha, 0.05f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, Theme::SurfaceVec());
        ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorConvertU32ToFloat4(Theme::Outline()));
        if (ImGui::BeginPopup("##popup"))
        {
            ImDrawList* popupDrawList = ImGui::GetWindowDrawList();
            for (int i = 0; i < count; ++i)
            {
                ImVec2 itemPos = ImGui::GetCursorScreenPos();
                ImGui::InvisibleButton(items[i], ImVec2(rowWidth - 8.0f, itemHeight));
                bool itemHovered = ImGui::IsItemHovered();
                bool itemSelected = *value == i;
                if (itemHovered)
                {
                    popupDrawList->AddRectFilled(ImVec2(itemPos.x + 2.0f, itemPos.y + 2.0f), ImVec2(itemPos.x + rowWidth - 10.0f, itemPos.y + itemHeight - 2.0f), Theme::SurfaceLight(118), 3.0f);
                }
                popupDrawList->AddText(ImVec2(itemPos.x + 8.0f, itemPos.y + (itemHeight - ImGui::GetFontSize()) * 0.5f - 1.0f), itemSelected ? Theme::Accent() : itemHovered ? Theme::Text() : Theme::MutedText(), items[i]);
                if (ImGui::IsItemClicked())
                {
                    *value = i;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(4);
        ImGui::PopID();
    }

    void Input(ImDrawList* drawList, const char* id, ImVec2 pos, float width, const char* label, char* buffer, std::size_t bufferSize)
    {
        drawList->AddText(pos, Theme::MutedText(), label);
        ImGui::PushID(id);
        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + 22.0f));
        ImGui::SetNextItemWidth(width);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 5.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, Theme::ControlVec());
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, Theme::SurfaceVec());
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.12f, 0.082f, 0.104f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(Theme::Text()));
        ImGui::InputText("##input", buffer, bufferSize);
        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(2);
        ImGui::PopID();
    }

    void ColorPicker(ImDrawList* drawList, const char* id, ImVec2 pos, float rowWidth, const char* label, float color[4])
    {
        static ImGuiID animatedColorId = 0;
        static float colorPopupAnim = 0.0f;
        ImVec2 textSize = ImGui::CalcTextSize(label);
        ImVec2 swatchMin(pos.x + rowWidth - 34.0f, pos.y - 3.0f);
        if (!(label[0] == '#' && label[1] == '#'))
        {
            drawList->AddText(ImVec2(pos.x, pos.y + (18.0f - textSize.y) * 0.5f), Theme::MutedText(), label);
        }

        ImGui::PushID(id);
        ImGuiID colorId = ImGui::GetID("##color");
        ImGui::SetCursorScreenPos(swatchMin);
        ImGui::ColorButton("##button", ImVec4(color[0], color[1], color[2], color[3]), ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoTooltip, ImVec2(26.0f, 16.0f));
        if (ImGui::IsItemClicked())
        {
            ImGui::OpenPopup("##popup");
            animatedColorId = colorId;
            colorPopupAnim = 0.0f;
        }
        bool colorPopupOpen = ImGui::IsPopupOpen("##popup", ImGuiPopupFlags_None);
        if (colorPopupOpen && animatedColorId != colorId)
        {
            animatedColorId = colorId;
            colorPopupAnim = 0.0f;
        }
        if (animatedColorId == colorId)
        {
            float target = colorPopupOpen ? 1.0f : 0.0f;
            colorPopupAnim += (target - colorPopupAnim) * (1.0f - std::exp(-ImGui::GetIO().DeltaTime * 18.0f));
        }
        drawList->AddRect(swatchMin, ImVec2(swatchMin.x + 26.0f, swatchMin.y + 16.0f), Theme::Outline(), 3.0f);
        ImVec2 swatchMax(swatchMin.x + 26.0f, swatchMin.y + 16.0f);
        ImGui::SetNextWindowPos(ImVec2(swatchMin.x, swatchMax.y + 4.0f), ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 4.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, Theme::Clamp(colorPopupAnim, 0.05f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, Theme::SurfaceVec());
        ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorConvertU32ToFloat4(Theme::Outline()));
        if (ImGui::BeginPopup("##popup"))
        {
            ImGui::ColorPicker4("##picker", color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview);
            ImGui::EndPopup();
        }
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(3);
        ImGui::PopID();
    }

    void KeyBind(ImDrawList* drawList, const char* id, ImVec2 pos, float width, const char* label, int* key)
    {
        static int listeningId = 0;
        ImGuiID instanceId = ImGui::GetID(id);

        ImVec2 btnMin(pos.x + width - 90.0f, pos.y - 1.0f);
        ImVec2 btnMax(btnMin.x + 90.0f, btnMin.y + 22.0f);

        bool isListening = (listeningId == static_cast<int>(instanceId) && listeningId != 0);

        if (isListening)
        {
            for (int vk = 1; vk < 256; ++vk)
            {
                if (vk == VK_LBUTTON || vk == VK_RBUTTON) continue;
                if (GetAsyncKeyState(vk) & 1)
                {
                    *key = vk;
                    listeningId = 0;
                    isListening = false;
                    break;
                }
            }
            if (GetAsyncKeyState(VK_LBUTTON) & 1)
            {
                POINT pt;
                GetCursorPos(&pt);
                RECT region{ static_cast<LONG>(btnMin.x), static_cast<LONG>(btnMin.y), static_cast<LONG>(btnMax.x), static_cast<LONG>(btnMax.y) };
                if (!PtInRect(&region, pt))
                {
                    listeningId = 0;
                    isListening = false;
                }
            }
        }

        drawList->AddText(pos, Theme::MutedText(), label);

        ImGui::PushID(id);
        ImGui::SetCursorScreenPos(btnMin);
        ImGui::InvisibleButton("##key", ImVec2(90.0f, 22.0f));
        bool hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked() && !isListening)
        {
            listeningId = static_cast<int>(instanceId);
            isListening = true;
        }
        ImGui::PopID();

        ImU32 bgColor = isListening ? Theme::Accent(100) : (hovered ? Theme::SurfaceLight(200) : Theme::Control());
        ImU32 borderColor = isListening ? Theme::Accent() : Theme::Outline();
        ImU32 textColor = isListening ? Theme::Accent() : (hovered ? Theme::Text() : Theme::MutedText());

        drawList->AddRectFilled(btnMin, btnMax, bgColor, 4.0f);
        drawList->AddRect(btnMin, btnMax, borderColor, 4.0f);

        const char* displayText = isListening ? "..." : GetKeyName(*key);
        ImVec2 textSize = ImGui::CalcTextSize(displayText);
        drawList->AddText(ImVec2(btnMin.x + (90.0f - textSize.x) * 0.5f, btnMin.y + (22.0f - textSize.y) * 0.5f), textColor, displayText);
    }

    void KeyBindEx(ImDrawList* drawList, const char* id, ImVec2 pos, float width, const char* label, int* key, int* mode)
    {
        static int listeningId = 0;
        ImGuiID instanceId = ImGui::GetID(id);

        float modeBtnSize = 18.0f;
        float keyBtnWidth = 72.0f;
        float gap = 4.0f;
        float totalWidth = modeBtnSize + gap + keyBtnWidth;
        float startX = pos.x + width - totalWidth;

        ImVec2 modeBtnMin(startX, pos.y - 1.0f);
        ImVec2 modeBtnMax(modeBtnMin.x + modeBtnSize, modeBtnMin.y + 22.0f);
        ImVec2 keyBtnMin(modeBtnMax.x + gap, pos.y - 1.0f);
        ImVec2 keyBtnMax(keyBtnMin.x + keyBtnWidth, keyBtnMin.y + 22.0f);

        bool isListening = (listeningId == static_cast<int>(instanceId) && listeningId != 0);

        if (isListening)
        {
            for (int vk = 1; vk < 256; ++vk)
            {
                if (vk == VK_LBUTTON || vk == VK_RBUTTON) continue;
                if (GetAsyncKeyState(vk) & 1)
                {
                    *key = vk;
                    listeningId = 0;
                    isListening = false;
                    break;
                }
            }
            if (GetAsyncKeyState(VK_LBUTTON) & 1)
            {
                POINT pt;
                GetCursorPos(&pt);
                RECT region{ static_cast<LONG>(keyBtnMin.x), static_cast<LONG>(keyBtnMin.y), static_cast<LONG>(keyBtnMax.x), static_cast<LONG>(keyBtnMax.y) };
                if (!PtInRect(&region, pt))
                {
                    listeningId = 0;
                    isListening = false;
                }
            }
        }

        drawList->AddText(pos, Theme::MutedText(), label);

        ImGui::PushID(id);

        ImGui::SetCursorScreenPos(keyBtnMin);
        ImGui::InvisibleButton("##key", ImVec2(keyBtnWidth, 22.0f));
        bool keyHovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked() && !isListening)
        {
            listeningId = static_cast<int>(instanceId);
            isListening = true;
        }

        ImGui::SetCursorScreenPos(modeBtnMin);
        ImGui::InvisibleButton("##mode", ImVec2(modeBtnSize, 22.0f));
        bool modeHovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked())
        {
            *mode = (*mode + 1) % 3;
        }

        ImGui::PopID();

        ImU32 keyBg = isListening ? Theme::Accent(100) : (keyHovered ? Theme::SurfaceLight(200) : Theme::Control());
        ImU32 keyBorder = isListening ? Theme::Accent() : Theme::Outline();
        ImU32 keyText = isListening ? Theme::Accent() : (keyHovered ? Theme::Text() : Theme::MutedText());

        drawList->AddRectFilled(keyBtnMin, keyBtnMax, keyBg, 4.0f);
        drawList->AddRect(keyBtnMin, keyBtnMax, keyBorder, 4.0f);

        const char* displayText = isListening ? "..." : GetKeyName(*key);
        ImVec2 textSize = ImGui::CalcTextSize(displayText);
        drawList->AddText(ImVec2(keyBtnMin.x + (keyBtnWidth - textSize.x) * 0.5f, keyBtnMin.y + (22.0f - textSize.y) * 0.5f), keyText, displayText);

        ImU32 modeColor = modeHovered ? Theme::Accent() : Theme::MutedText(180);
        const char* modeText = (*mode == 1) ? "H" : (*mode == 2) ? "A" : "T";
        ImVec2 modeTextSize = ImGui::CalcTextSize(modeText);
        drawList->AddRectFilled(modeBtnMin, modeBtnMax, Theme::Control(), 4.0f);
        drawList->AddRect(modeBtnMin, modeBtnMax, modeHovered ? Theme::Accent() : Theme::Outline(), 4.0f);
        drawList->AddText(ImVec2(modeBtnMin.x + (modeBtnSize - modeTextSize.x) * 0.5f, modeBtnMin.y + (22.0f - modeTextSize.y) * 0.5f), modeColor, modeText);
    }

    bool MenuButton(ImDrawList* drawList, const char* id, ImVec2 pos, ImVec2 size, const char* label, ImU32 accent, bool isSmall)
    {
        ImGui::PushID(id);
        ImGui::SetCursorScreenPos(pos);
        ImGui::InvisibleButton("##btn", size);
        bool hovered = ImGui::IsItemHovered();
        bool active = ImGui::IsItemActive();
        bool clicked = ImGui::IsItemClicked();
        ImGui::PopID();

        if (!accent) accent = Theme::Accent(215);
        ImU32 bg = active ? Theme::Accent(180) : (hovered ? Theme::Accent(100) : Theme::Control());
        ImU32 border = active ? accent : (hovered ? Theme::Accent(160) : Theme::Outline());
        ImU32 textCol = (hovered || active) ? Theme::Text() : Theme::MutedText();

        drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bg, 4.0f);
        drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), border, 4.0f);
        ImVec2 ts = ImGui::CalcTextSize(label);
        float tx = pos.x + (size.x - ts.x) * 0.5f;
        float ty = pos.y + (size.y - ts.y) * 0.5f;
        drawList->AddText(ImVec2(tx, ty), textCol, label);

        return clicked;
    }
}
