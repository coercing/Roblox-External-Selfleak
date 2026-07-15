#include "UiFont.h"

#include "../ImGui/imgui.h"
#include "SourceSans3SemiBold.h"
#include "../Fontawesome/FontAwesome.h"

#include "../Source/Core/Graphics/Fonts/Verdana.h"

namespace
{
    static ImFont* gFonts[3] = {};

    bool LoadFromFile(int index, const char* path, float sizePixels)
    {
        ImFontConfig config{};
        config.FontDataOwnedByAtlas = false;

        gFonts[index] = ImGui::GetIO().Fonts->AddFontFromFileTTF(path, sizePixels, &config);
        if (gFonts[index])
            FontAwesome::LoadSolid(16.0f, true);
        return gFonts[index] != nullptr;
    }

    bool LoadEmbedded(int index, const unsigned char* data, int size, float sizePixels)
    {
        ImFontConfig config{};
        config.FontDataOwnedByAtlas = false;

        gFonts[index] = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
            const_cast<unsigned char*>(data), size, sizePixels, &config);
        if (gFonts[index])
            FontAwesome::LoadSolid(16.0f, true);
        return gFonts[index] != nullptr;
    }

    ImFont* LoadSemiBold(float sizePixels)
    {
        ImFontConfig config{};
        config.FontDataOwnedByAtlas = false;

        ImFont* font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
            const_cast<unsigned char*>(kSourceSans3SemiBoldData),
            static_cast<int>(kSourceSans3SemiBoldSize),
            sizePixels,
            &config
        );
        if (font)
            FontAwesome::LoadSolid(16.0f, true);
        return font;
    }
}

ImFont* UiFont::LoadAll()
{
    // Load system Tahoma fonts from C:\Windows\Fonts\ (present on every Windows PC)
    if (!LoadFromFile(0, "C:\\Windows\\Fonts\\tahoma.ttf", 16.0f))
        gFonts[0] = nullptr;

    if (!LoadFromFile(1, "C:\\Windows\\Fonts\\tahomabd.ttf", 16.0f))
        gFonts[1] = nullptr;

    // Embedded Verdana Pro Bold (the third font was a custom path, now embedded)
    LoadEmbedded(2, Verdana, static_cast<int>(sizeof(Verdana)), 16.0f);

    ImFont* defaultFont = gFonts[0] ? gFonts[0] : (gFonts[1] ? gFonts[1] : LoadSemiBold(16.0f));

    if (defaultFont)
        ImGui::GetIO().FontDefault = defaultFont;

    return defaultFont;
}

void UiFont::SwitchTo(int index)
{
    if (index >= 0 && index < 3 && gFonts[index])
        ImGui::GetIO().FontDefault = gFonts[index];
}

int UiFont::GetFontCount()
{
    return 3;
}

ImFont* UiFont::GetFont(int index)
{
    if (index >= 0 && index < 3)
        return gFonts[index];
    return nullptr;
}
