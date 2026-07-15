#include "FontAwesome.h"

#include "../Fonts/FontAwesomeSolid.h"
#include "../ImGui/imgui.h"

namespace
{
    constexpr ImWchar kFontAwesomeRanges[] = { 0xE000, 0xF8FF, 0 };
}

ImFont* FontAwesome::LoadSolid(float sizePixels, bool mergeWithLastFont)
{
    ImFontConfig config{};
    config.FontDataOwnedByAtlas = false;
    config.MergeMode = mergeWithLastFont;
    config.PixelSnapH = true;
    config.GlyphOffset.y = 1.5f;

    return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
        const_cast<unsigned char*>(kFontAwesomeSolidData),
        static_cast<int>(kFontAwesomeSolidSize),
        sizePixels,
        &config,
        kFontAwesomeRanges
    );
}
