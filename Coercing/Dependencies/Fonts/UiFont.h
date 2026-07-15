#pragma once

struct ImFont;

namespace UiFont
{
    ImFont* LoadAll();
    void SwitchTo(int index);
    int GetFontCount();
    ImFont* GetFont(int index);
}
