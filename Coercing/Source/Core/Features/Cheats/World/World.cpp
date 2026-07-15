#include "Engine/Engine.h"
#include "Globals.hxx"
#include "Core/Features/Cache/Cache.h"
#include <iostream>

namespace World {

    struct OriginalValues {
        bool Initialized = false;

        // Lighting originals
        float FogEnd = 0.f;
        SDK::Vector3 FogColor{};
        float Exposure = 0.f;
        float FOV = 0.f;
    };

    static OriginalValues Original;

    static uintptr_t FindRenderView() {
        if (!Globals::VisualEngine.Address) return 0;
        if (Globals::VisualEngine.Address <= Offsets::RenderView::VisualEngine) return 0;
        uintptr_t rv = Globals::VisualEngine.Address - Offsets::RenderView::VisualEngine;
        if (rv > 0x1000) return rv;
        return 0;
    }

    void SaveOriginals() {
        if (Original.Initialized) return;

        if (Globals::Lighting.Address)
        {
            Original.FogEnd = Driver->Read<float>(Globals::Lighting.Address + Offsets::Lighting::FogEnd);
            Original.FogColor = Driver->Read<SDK::Vector3>(Globals::Lighting.Address + Offsets::Lighting::FogColor);
            Original.Exposure = Driver->Read<float>(Globals::Lighting.Address + Offsets::Lighting::ExposureCompensation);

            if (Globals::Camera.Address)
                Original.FOV = Driver->Read<float>(Globals::Camera.Address + Offsets::Camera::FieldOfView) / 0.0174533f;

            Original.Initialized = true;
        }
    }

    static void InvalidateLighting() {
        uintptr_t rv = FindRenderView();
        if (rv) {
            Driver->Write<bool>(rv + Offsets::RenderView::LightingValid, false);
            return;
        }
        if (Globals::Lighting.Address) {
            bool gs = Driver->Read<bool>(Globals::Lighting.Address + Offsets::Lighting::GlobalShadows);
            Driver->Write<bool>(Globals::Lighting.Address + Offsets::Lighting::GlobalShadows, !gs);
            Driver->Write<bool>(Globals::Lighting.Address + Offsets::Lighting::GlobalShadows, gs);
        }
    }

    void RestoreAll() {
        SaveOriginals();

        if (!Original.Initialized) return;

        if (Globals::Lighting.Address)
        {
            SDK::Lighting::SetFog(Globals::Lighting.Address, Original.FogEnd, Original.FogColor);
            SDK::Lighting::SetExposure(Globals::Lighting.Address, Original.Exposure);
            InvalidateLighting();
        }

        if (Globals::Camera.Address)
            SDK::Lighting::SetFOV(Globals::Camera.Address, Original.FOV);

        // Disable all World features
        Globals::World::Fog         = false;
        Globals::World::Exposure    = false;
        Globals::World::FOV         = false;
    }

    void FogChanger() {
        bool wasFog = false;

        while (true)
        {
            bool isFog = Globals::World::Fog;

            if (isFog)
            {
                SDK::Lighting::SetFog(Globals::Lighting.Address, Globals::World::Fog_Distance, {Globals::World::Colors::Fog[0], Globals::World::Colors::Fog[1], Globals::World::Colors::Fog[2]} );
                InvalidateLighting();
            }
            else if (wasFog && Original.Initialized)
            {
                SDK::Lighting::SetFog(Globals::Lighting.Address, Original.FogEnd, Original.FogColor);
                InvalidateLighting();
            }

            wasFog = isFog;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));        
        }
    }
    void ExposureChanger() {
        bool wasExposure = false;

        while (true)
        {
            bool isExposure = Globals::World::Exposure;

            if (isExposure)
            {
                SDK::Lighting::SetExposure(Globals::Lighting.Address, Globals::World::ExposureI);
                InvalidateLighting();
            }
            else if (wasExposure && Original.Initialized)
            {
                SDK::Lighting::SetExposure(Globals::Lighting.Address, Original.Exposure);
                InvalidateLighting();
            }

            wasExposure = isExposure;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void FOVChanger() {
        bool wasFOV = false;

        while (true)
        {
            bool isFOV = Globals::World::FOV;

            if (isFOV)
            {
				SDK::Lighting::SetFOV(Globals::Camera.Address, Globals::World::FOV_Distance);
            }
            else if (wasFOV && Original.Initialized)
            {
                SDK::Lighting::SetFOV(Globals::Camera.Address, Original.FOV);
            }

            wasFOV = isFOV;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

    }

    void RunService() {

        SaveOriginals();

        std::thread(FogChanger).detach();
        std::thread(ExposureChanger).detach();
        std::thread(FOVChanger).detach();
    }
}
