#include "../../Engine.h"
#include <Globals.hxx>

namespace SDK {

    // Cache the renderview address — it doesn't change mid-session.
    // Re-resolve if it reads as 0 (e.g. after a game restart).
    static std::uint64_t s_CachedRenderview = 0;

    std::uint64_t Renderview::GetRenderview() {

        if (s_CachedRenderview != 0)
            return s_CachedRenderview;

        if (!Globals::Datamodel.Address)
            return 0;

        // Datamodel -> RenderView chain
        auto Rv1 = Driver->Read<std::uintptr_t>(Globals::Datamodel.Address + 0x1D0);
        if (!Rv1) return 0;
        auto Rv2 = Driver->Read<std::uintptr_t>(Rv1 + 0x8);
        if (!Rv2) return 0;
        auto Rv3 = Driver->Read<std::uintptr_t>(Rv2 + 0x28);
        if (!Rv3) return 0;

        s_CachedRenderview = Rv3;
        return s_CachedRenderview;
    }

    void Renderview::InvalidateCache() {
        s_CachedRenderview = 0;
    }

    void Renderview::InvalidateLighting() {

        auto rv = Globals::Renderview.GetRenderview();
        if (!rv) return;

        Driver->Write<bool>(rv + Offsets::RenderView::LightingValid, false);
    }

}
