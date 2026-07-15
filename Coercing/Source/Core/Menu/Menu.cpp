#include "Menu.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include <windows.h>

#include <imgui/imgui.h>
#include <Fonts/UiFont.h>
#include <Globals.hxx>
#include <imgui/addons/imgui_addons.h>
#include "Theme.h"
#include "Widgets.h"
#include <Core/Config/Config.h>

extern bool g_MenuOpen;

namespace Menu
{
    // ── Snapshot struct mirroring Globals ──
    static struct {
        bool aimbotEnabled = false, aimbotSticky = false, knockedCheck = false, visCheck = false;
        int aimbotType = 0;
        int hitPart = 0;
        int aimbotKey = 0, aimbotMode = 0;
        float mSmoothX = 0.f, mSmoothY = 0.f, mSens = 0.f;
        float cSmoothX = 0.f, cSmoothY = 0.f;
        bool prediction = false;
        float predX = 0.f, predY = 0.f;
        bool shake = false;
        float shakeX = 0.f, shakeY = 0.f;
        bool drawFov = false;
        float fovColor[4] = { 1.f, 1.f, 1.f, 1.f };
        float fovSize = 100.f;
        bool fovSpin = false;
        int fovSpinSpeed = 1;
        bool useFov = false;
        bool silentEnabled = false, silentSticky = false, silentKnocked = false, silentVis = false, silentSpoof = false, silentRaycast = false;
        int silentPart = 0;
        int silentKey = 0, silentMode = 0;
        bool silentDrawFov = false;
        float silentFovColor[4] = { 1.f, 1.f, 1.f, 1.f };
        bool silentGunFov = false;
        float silentFov = 100.f, silentFovDb = 100.f, silentFovTac = 100.f, silentFovRev = 100.f;
        bool silentFovSpin = false;
        int silentFovSpinSpeed = 1;
        bool silentUseFov = false;
        bool visEnabled = false;
        bool box = false, boxFill = false, boxFillGrad = false, boxFillGradRot = false;
        int boxType = 0, boxFillType = 0, boxFillSpeed = 1;
        float boxColor[4] = { 1,1,1,1 };
        float boxFillTop[4] = { 1,1,1,1 }, boxFillBot[4] = { 1,1,1,1 };
        bool healthbar = false;
        int healthbarType = 0, healthbarGap = 2, healthbarThick = 2;
        float healthColor[4] = { 1,1,1,1 };
        float healthTop[4] = { 1,1,1,1 }, healthMid[4] = { 1,1,1,1 }, healthBot[4] = { 1,1,1,1 };
        bool health = false;
        float healthTextColor[4] = { 1,1,1,1 };
        bool name = false;
        int nameType = 0;
        float nameColor[4] = { 1,1,1,1 };
        bool distance = false;
        float distColor[4] = { 1,1,1,1 };
        bool rigType = false;
        float rigColor[4] = { 1,1,1,1 };
        bool tool = false;
        float toolColor[4] = { 1,1,1,1 };
		bool skeleton = false;
		float skelColor[4] = { 1,1,1,1 };
		bool friendCheck = false;
        float friendColor[4] = { 0,1,0,1 };
        float renderDist = 1000.f;
        bool exclClient = false, exclTeam = false;
        bool fog = false;
        float fogColor[4] = { 1,1,1,1 }, fogDist = 500.f;
        bool exposure = false;
        float exposureVal = 0.f;
        bool worldFov = false;
        float worldFovVal = 90.f;

        bool streamproof = false, hideConsole = false;
        bool explorer = false;
        int menuToggleKey = 0, panicKey = 0;
        bool flyEnabled = false;
        int flyKey = 0, flyMode = 0;
        bool speedEnabled = false;
        int speedType = 0;
        int speedKey = 0, speedMode = 0;
        bool jumpPowerEnabled = false;
        int jumpPowerKey = 0, jumpPowerMode = 0;
        bool macroEnabled = false;
        int macroType = 0;
        int macroKey = 0, macroMode = 0;
        bool spinbotEnabled = false;
        int spinbotType = 0;
        float spinbotSpeed = 0.067f;
        int flyType = 0;
        bool flyStealth = false;
        bool flyBypass = false;
        float flyBypassStrength = 5.f;
        float flyBypassInterval = 5.f;
        float flyBypassDuration = 0.15f;
        float flySpeed = 50.f;
        float speedVal = 50.f;
        float jumpPowerVal = 50.f;
        int perfMode = 0;
        float accentColor[4] = { 1.0f, 0.62f, 0.78f, 1.0f };
        int fontIndex = 0;
        float vMSmoothX = 0, vMSmoothY = 0, vMSens = 0;
        float vCSmoothX = 0, vCSmoothY = 0;
        float vPredX = 0, vPredY = 0;
        float vShakeX = 0, vShakeY = 0;
        float vFovSize = 100;
        float vSilentFov = 100, vSilentFovDb = 100, vSilentFovTac = 100, vSilentFovRev = 100;
        float vRenderDist = 250;
        float vHitEffectsDuration = 1.5f;
        float vHeadDotSize = 3;
        float vRadarSize = 200;
        float vRadarRange = 250;
        float vRadarOpacity = 0.5f;
        float vOverlayDarkness = 0.5f;
        float vBgCount = 50;
        float vBgSpeed = 1;
        float vFogDist = 500;
        float vExposure = 0;
        float vWorldFov = 90;

        float vFovSpinSpeed = 1, vSilentFovSpinSpeed = 1, vHealthbarGap = 2, vHealthbarThick = 2;
		float vBoxFillSpeed = 1;
        float vFlySpeed = 50, vFlyBypassStrength = 5, vFlyBypassInterval = 5, vFlyBypassDuration = 0.15f;
        float vSpeedVal = 50, vJumpPowerVal = 50;
        float vSpinbotSpeed = 0.067f;

        // ESP upgrades
        bool snapline = false;
        int snaplinePos = 0;
        float snaplineCol[4] = { 0.372549f, 0.784314f, 0.992157f, 0.78f };
        bool headDot = false;
        float headDotSize = 3.f;
        float headDotCol[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
        // Watermark
        bool watermark = false;
        bool wmShowPlayers = true;
        bool wmShowGame = true;

        // Fullbright
        bool fullbright = false;

        // Skybox
        bool skyEnabled = false;
        int  skyPreset = 0;
        char skyCustomId[64] = "15983996673";

        // Radar
        bool radEnabled = false;
        float radSize = 200.f, radRange = 150.f, radOpacity = 0.7f;
        int radFlags = 3;

        // Chams
        bool chams = false;
        float chamsColor[4] = { 1.f, 0.f, 0.f, 0.4f };

        // Visuals upgrades
        bool lookDir = false;
        float lookDirColor[4] = { 1.f,1.f,1.f,1.f };

        // Hit effects
        int hitEffectsType = 0;
        float hitEffectsDuration = 1.5f;
        float hitEffectsColor[4] = { 1.f, 1.f, 1.f, 0.8f };
        bool hitNotifications = false;

        // Shot tracers
        bool shotTracers = false;
        int shotTracerType = 0;
        float shotTracerColor[4] = { 1.f, 1.f, 1.f, 1.f };
    } st;

    static float menuAlpha = 0.f;
    static float menuScale = 0.85f;

    static int bgEffect = 0;
    static int particleCount = 50;
    static float particleSpeed = 1.f;

    static int activeTab = 0;
    static int visibleCrumbTab = 0, previousCrumbTab = 0;
    static float crumbFade = 1.0f;
    static float tabFade[6] = { 1.0f, 0.45f, 0.45f, 0.45f, 0.45f, 0.45f };
    static bool leftPanelOpen[6] = { true, true, true, true, true, true };
    static bool rightPanelOpen[6] = { true, true, true, true, true, true };

    static float leftScroll[6] = { 0, 0, 0, 0, 0, 0 };
    static float rightScroll[6] = { 0, 0, 0, 0, 0, 0 };

    // Right-side sub-panels for Combat tab
    static bool silentPanelOpen = true;
    static float silentScroll = 0;

    // ── Helper: read all Globals into st ──
    static void SyncGlobalsToSt()
    {
        // Aimbot
        st.aimbotEnabled   = Globals::Aimbot::Enabled;
        st.aimbotSticky    = Globals::Aimbot::AimbotSticky;
        st.knockedCheck    = Globals::Aimbot::KnockedCheck;
        st.visCheck        = Globals::Aimbot::VisibilityCheck;
        st.aimbotType      = Globals::Aimbot::Aimbot_type;
        st.hitPart         = Globals::Aimbot::HitPart;
        st.aimbotKey       = ImGuiKeyToVK(Globals::Aimbot::Aimbot_Key);
        st.aimbotMode      = Globals::Aimbot::Aimbot_Mode;
        st.mSmoothX        = Globals::Aimbot::Mouse::Smoothing_X;
        st.mSmoothY        = Globals::Aimbot::Mouse::Smoothing_Y;
        st.mSens           = Globals::Aimbot::Mouse::Mouse_Sensitivty;
        st.cSmoothX        = Globals::Aimbot::Camera::Smoothing_X;
        st.cSmoothY        = Globals::Aimbot::Camera::Smoothing_Y;
        st.prediction      = Globals::Aimbot::Prediction;
        st.predX           = Globals::Aimbot::Prediction_X;
        st.predY           = Globals::Aimbot::Prediction_Y;
        st.shake           = Globals::Aimbot::Shake;
        st.shakeX          = Globals::Aimbot::ShakeX;
        st.shakeY          = Globals::Aimbot::ShakeY;
        st.drawFov         = Globals::Aimbot::DrawFov;
        std::copy_n(Globals::Aimbot::FovColor, 4, st.fovColor);
        st.fovSize         = Globals::Aimbot::FovSize;
        st.fovSpin         = Globals::Aimbot::FovSpin;
        st.fovSpinSpeed    = Globals::Aimbot::FovSpinSpeed;
        st.useFov          = Globals::Aimbot::useFov;

        // Silent
        st.silentEnabled   = Globals::Silent::Enabled;
        st.silentSticky    = Globals::Silent::StickyAim;
        st.silentKnocked   = Globals::Silent::KnockedCheck;
        st.silentVis       = Globals::Silent::VisibilityCheck;
        st.silentSpoof     = Globals::Silent::SpoofMouse;
        st.silentRaycast   = Globals::Silent::Raycast;
        st.silentPart      = Globals::Silent::AimPart;
        st.silentKey       = ImGuiKeyToVK(Globals::Silent::Silent_Key);
        st.silentMode      = Globals::Silent::Silent_Mode;
        st.silentDrawFov   = Globals::Silent::DrawFov;
        std::copy_n(Globals::Silent::FovColor, 4, st.silentFovColor);
        st.silentGunFov    = Globals::Silent::GunBasedFov;
        st.silentFov       = Globals::Silent::Fov;
        st.silentFovDb     = Globals::Silent::FovDoubleBarrel;
        st.silentFovTac    = Globals::Silent::FovTacticalShotgun;
        st.silentFovRev    = Globals::Silent::FovRevolver;
        st.silentFovSpin   = Globals::Silent::FovSpin;
        st.silentFovSpinSpeed = Globals::Silent::FovSpinSpeed;
        st.silentUseFov    = Globals::Silent::UseFov;

        // Visuals
        st.visEnabled      = Globals::Visuals::Enabled;
        st.box             = Globals::Visuals::Box;
        st.boxFill         = Globals::Visuals::Box_Fill;
        st.boxFillGrad     = Globals::Visuals::Box_Fill_Gradient;
        st.boxFillGradRot  = Globals::Visuals::Box_Fill_Gradient_Rotate;
        st.boxType         = Globals::Visuals::Box_Type;
        st.boxFillType     = Globals::Visuals::Box_Fill_Type;
        st.boxFillSpeed    = Globals::Visuals::BoxFillSpeed;
        std::copy_n(Globals::Visuals::Colors::Box, 4, st.boxColor);
        std::copy_n(Globals::Visuals::Colors::BoxFill_Top, 4, st.boxFillTop);
        std::copy_n(Globals::Visuals::Colors::BoxFill_Bottom, 4, st.boxFillBot);
        st.healthbar       = Globals::Visuals::Healthbar;
        st.healthbarType   = Globals::Visuals::Healthbar_Type;
        st.healthbarGap    = Globals::Visuals::Gap;
        st.healthbarThick  = Globals::Visuals::Thickness;
        std::copy_n(Globals::Visuals::Colors::Healthbar, 4, st.healthColor);
        std::copy_n(Globals::Visuals::Colors::Healthbar_Top, 4, st.healthTop);
        std::copy_n(Globals::Visuals::Colors::Healthbar_Middle, 4, st.healthMid);
        std::copy_n(Globals::Visuals::Colors::Healthbar_Bottom, 4, st.healthBot);
        st.health          = Globals::Visuals::Health;
        std::copy_n(Globals::Visuals::Colors::Health, 4, st.healthTextColor);
        st.name            = Globals::Visuals::Name;
        st.nameType        = Globals::Visuals::Name_Type;
        std::copy_n(Globals::Visuals::Colors::Name, 4, st.nameColor);
        st.distance        = Globals::Visuals::Distance;
        std::copy_n(Globals::Visuals::Colors::Distance, 4, st.distColor);
        st.rigType         = Globals::Visuals::Rig_Type;
        std::copy_n(Globals::Visuals::Colors::Rig_Type, 4, st.rigColor);
        st.tool            = Globals::Visuals::Tool;
        std::copy_n(Globals::Visuals::Colors::Tool, 4, st.toolColor);
		st.skeleton        = Globals::Visuals::Skeleton;
		std::copy_n(Globals::Visuals::Colors::Skeleton, 4, st.skelColor);
		st.chams           = Globals::Visuals::Chams;
		std::copy_n(Globals::Visuals::ChamsColor, 4, st.chamsColor);
		st.renderDist      = Globals::Visuals::Render_Distance;
        st.exclClient      = Globals::Settings::Client_Check;
        st.exclTeam        = Globals::Settings::Team_Check;
        st.friendCheck     = Globals::Settings::FriendCheck;
        std::copy_n(Globals::Visuals::Colors::Friend, 4, st.friendColor);

        // World
        st.fog             = Globals::World::Fog;
        std::copy_n(Globals::World::Colors::Fog, 4, st.fogColor);
        st.fogDist         = Globals::World::Fog_Distance;
        st.exposure        = Globals::World::Exposure;
        st.exposureVal     = Globals::World::ExposureI;
        st.worldFov        = Globals::World::FOV;
        st.worldFovVal     = Globals::World::FOV_Distance;

        // Settings
        st.streamproof     = Globals::Settings::Streamproof;
        st.hideConsole     = Globals::Settings::Hide_Console;
        st.explorer        = Globals::Misc::Explorer;
        st.menuToggleKey   = ImGuiKeyToVK(Globals::Settings::Menu_Toggle_Key);
        st.panicKey        = ImGuiKeyToVK(Globals::Settings::Exit_Key);
        st.flyEnabled         = Globals::Misc::Fly;
        st.flyKey             = ImGuiKeyToVK(Globals::Misc::Fly_Key);
        st.flyMode            = Globals::Misc::Fly_Mode;
        st.flyType            = Globals::Misc::Fly_Type;
        st.flyStealth         = Globals::Misc::Fly_Stealth;
        st.flyBypass          = Globals::Misc::Fly_BypassPulse;
        st.flyBypassStrength  = Globals::Misc::Fly_BypassStrength;
        st.flyBypassInterval  = Globals::Misc::Fly_BypassInterval;
        st.flyBypassDuration  = Globals::Misc::Fly_BypassDuration;
        st.speedEnabled    = Globals::Movement::Speed;
        st.speedType       = Globals::Movement::Speed_Type;
        st.speedKey        = ImGuiKeyToVK(Globals::Movement::Speed_Key);
        st.speedMode       = Globals::Movement::Speed_Mode;
        st.jumpPowerEnabled = Globals::Movement::JumpPower;
        st.jumpPowerKey    = ImGuiKeyToVK(Globals::Movement::JumpPower_Key);
        st.jumpPowerMode   = Globals::Movement::JumpPower_Mode;
        st.macroEnabled    = Globals::Movement::Macro;
        st.macroType       = Globals::Movement::Macro_Type;
        st.macroKey        = ImGuiKeyToVK(Globals::Movement::Macro_Key);
        st.macroMode       = Globals::Movement::Macro_Mode;
        st.spinbotEnabled  = Globals::Movement::Spinbot;
        st.spinbotType     = Globals::Movement::Spinbot_Type;
        st.spinbotSpeed    = Globals::Movement::Spinbot_Speed;
        st.flySpeed        = Globals::Misc::Fly_Speed;
        st.speedVal        = Globals::Movement::Speed_Value;
        st.jumpPowerVal    = Globals::Movement::JumpPower_Value;
        st.perfMode        = Globals::Settings::Performance_Mode;
        std::copy_n(Globals::Settings::AccentColor, 4, st.accentColor);
        st.fontIndex       = Globals::Settings::FontIndex;

        st.radEnabled   = Globals::Radar::Enabled;
        st.radSize      = Globals::Radar::Size;
        st.radRange     = Globals::Radar::Range;
        st.radOpacity   = Globals::Radar::Opacity;
        st.radFlags     = Globals::Radar::EnemyFlags;

        st.lookDir      = Globals::Visuals::LookDirection;
        std::copy_n(Globals::Visuals::LookDirColor, 4, st.lookDirColor);

        st.snapline     = Globals::Visuals::Snapline;
        st.snaplinePos  = Globals::Visuals::SnaplinePosition;
        st.headDot      = Globals::Visuals::HeadDot;
        st.headDotSize  = Globals::Visuals::HeadDotSize;
        std::copy_n(Globals::Visuals::Colors::Snapline, 4, st.snaplineCol);
        std::copy_n(Globals::Visuals::Colors::HeadDot, 4, st.headDotCol);
        st.watermark        = Globals::Watermark::Enabled;
        st.wmShowPlayers    = Globals::Watermark::ShowPlayers;
        st.wmShowGame       = Globals::Watermark::ShowGame;

        st.hitEffectsType       = Globals::Visuals::HitEffectsType;
        st.hitEffectsDuration   = Globals::Visuals::HitEffectsDuration;
        std::copy_n(Globals::Visuals::HitEffectsColor, 4, st.hitEffectsColor);
        st.hitNotifications     = Globals::Visuals::HitNotifications;
        st.shotTracers          = Globals::Visuals::ShotTracers;
        st.shotTracerType       = Globals::Visuals::ShotTracerType;
        std::copy_n(Globals::Visuals::ShotTracerColor, 4, st.shotTracerColor);
    }

    // ── Helper: write all st back to Globals ──
    static void SyncStToGlobals()
    {
        // Aimbot
        Globals::Aimbot::Enabled         = st.aimbotEnabled;
        Globals::Aimbot::AimbotSticky    = st.aimbotSticky;
        Globals::Aimbot::KnockedCheck    = st.knockedCheck;
        Globals::Aimbot::VisibilityCheck = st.visCheck;
        Globals::Aimbot::Aimbot_type     = st.aimbotType;
        Globals::Aimbot::HitPart         = st.hitPart;
        Globals::Aimbot::Aimbot_Key      = VKToImGuiKey(st.aimbotKey);
        Globals::Aimbot::Aimbot_Mode     = static_cast<ImKeyBindMode>(st.aimbotMode);
        Globals::Aimbot::Mouse::Smoothing_X  = st.mSmoothX;
        Globals::Aimbot::Mouse::Smoothing_Y  = st.mSmoothY;
        Globals::Aimbot::Mouse::Mouse_Sensitivty = st.mSens;
        Globals::Aimbot::Camera::Smoothing_X = st.cSmoothX;
        Globals::Aimbot::Camera::Smoothing_Y = st.cSmoothY;
        Globals::Aimbot::Prediction      = st.prediction;
        Globals::Aimbot::Prediction_X    = st.predX;
        Globals::Aimbot::Prediction_Y    = st.predY;
        Globals::Aimbot::Shake           = st.shake;
        Globals::Aimbot::ShakeX          = st.shakeX;
        Globals::Aimbot::ShakeY          = st.shakeY;
        Globals::Aimbot::DrawFov         = st.drawFov;
        std::copy_n(st.fovColor, 4, Globals::Aimbot::FovColor);
        Globals::Aimbot::FovSize         = st.fovSize;
        Globals::Aimbot::FovSpin         = st.fovSpin;
        Globals::Aimbot::FovSpinSpeed    = st.fovSpinSpeed;
        Globals::Aimbot::useFov          = st.useFov;

        // Silent
        Globals::Silent::Enabled         = st.silentEnabled;
        Globals::Silent::StickyAim       = st.silentSticky;
        Globals::Silent::KnockedCheck    = st.silentKnocked;
        Globals::Silent::VisibilityCheck = st.silentVis;
        Globals::Silent::SpoofMouse      = st.silentSpoof;
        Globals::Silent::Raycast         = st.silentRaycast;
        Globals::Silent::AimPart         = st.silentPart;
        Globals::Silent::Silent_Key      = VKToImGuiKey(st.silentKey);
        Globals::Silent::Silent_Mode     = static_cast<ImKeyBindMode>(st.silentMode);
        Globals::Silent::DrawFov         = st.silentDrawFov;
        std::copy_n(st.silentFovColor, 4, Globals::Silent::FovColor);
        Globals::Silent::GunBasedFov     = st.silentGunFov;
        Globals::Silent::Fov             = st.silentFov;
        Globals::Silent::FovDoubleBarrel = st.silentFovDb;
        Globals::Silent::FovTacticalShotgun = st.silentFovTac;
        Globals::Silent::FovRevolver     = st.silentFovRev;
        Globals::Silent::FovSpin         = st.silentFovSpin;
        Globals::Silent::FovSpinSpeed    = st.silentFovSpinSpeed;
        Globals::Silent::UseFov          = st.silentUseFov;

        // Visuals
        Globals::Visuals::Enabled               = st.visEnabled;
        Globals::Visuals::Box                   = st.box;
        Globals::Visuals::Box_Fill              = st.boxFill;
        Globals::Visuals::Box_Fill_Gradient     = st.boxFillGrad;
        Globals::Visuals::Box_Fill_Gradient_Rotate = st.boxFillGradRot;
        Globals::Visuals::Box_Type              = st.boxType;
        Globals::Visuals::Box_Fill_Type         = st.boxFillType;
        Globals::Visuals::BoxFillSpeed          = st.boxFillSpeed;
        std::copy_n(st.boxColor, 4, Globals::Visuals::Colors::Box);
        std::copy_n(st.boxFillTop, 4, Globals::Visuals::Colors::BoxFill_Top);
        std::copy_n(st.boxFillBot, 4, Globals::Visuals::Colors::BoxFill_Bottom);
        Globals::Visuals::Healthbar             = st.healthbar;
        Globals::Visuals::Healthbar_Type        = st.healthbarType;
        Globals::Visuals::Gap                   = st.healthbarGap;
        Globals::Visuals::Thickness             = st.healthbarThick;
        std::copy_n(st.healthColor, 4, Globals::Visuals::Colors::Healthbar);
        std::copy_n(st.healthTop, 4, Globals::Visuals::Colors::Healthbar_Top);
        std::copy_n(st.healthMid, 4, Globals::Visuals::Colors::Healthbar_Middle);
        std::copy_n(st.healthBot, 4, Globals::Visuals::Colors::Healthbar_Bottom);
        Globals::Visuals::Health                = st.health;
        std::copy_n(st.healthTextColor, 4, Globals::Visuals::Colors::Health);
        Globals::Visuals::Name                  = st.name;
        Globals::Visuals::Name_Type             = st.nameType;
        std::copy_n(st.nameColor, 4, Globals::Visuals::Colors::Name);
        Globals::Visuals::Distance              = st.distance;
        std::copy_n(st.distColor, 4, Globals::Visuals::Colors::Distance);
        Globals::Visuals::Rig_Type              = st.rigType;
        std::copy_n(st.rigColor, 4, Globals::Visuals::Colors::Rig_Type);
        Globals::Visuals::Tool                  = st.tool;
        std::copy_n(st.toolColor, 4, Globals::Visuals::Colors::Tool);
		Globals::Visuals::Skeleton              = st.skeleton;
		std::copy_n(st.skelColor, 4, Globals::Visuals::Colors::Skeleton);
		Globals::Visuals::Chams                 = st.chams;
		std::copy_n(st.chamsColor, 4, Globals::Visuals::ChamsColor);
		Globals::Visuals::Render_Distance       = st.renderDist;
        Globals::Settings::Client_Check         = st.exclClient;
        Globals::Settings::Team_Check           = st.exclTeam;
        Globals::Settings::FriendCheck          = st.friendCheck;
        std::copy_n(st.friendColor, 4, Globals::Visuals::Colors::Friend);

        // World
        Globals::World::Fog             = st.fog;
        std::copy_n(st.fogColor, 4, Globals::World::Colors::Fog);
        Globals::World::Fog_Distance    = st.fogDist;
        Globals::World::Exposure        = st.exposure;
        Globals::World::ExposureI       = st.exposureVal;
        Globals::World::FOV             = st.worldFov;
        Globals::World::FOV_Distance    = st.worldFovVal;

        // Settings
        Globals::Settings::Streamproof     = st.streamproof;
        Globals::Settings::Hide_Console    = st.hideConsole;
        Globals::Misc::Explorer            = st.explorer;
        Globals::Settings::Menu_Toggle_Key = VKToImGuiKey(st.menuToggleKey);
        Globals::Settings::Exit_Key        = VKToImGuiKey(st.panicKey);
        Globals::Misc::Fly                 = st.flyEnabled;
        Globals::Misc::Fly_Key             = VKToImGuiKey(st.flyKey);
        Globals::Misc::Fly_Mode            = static_cast<ImKeyBindMode>(st.flyMode);
        Globals::Misc::Fly_Type            = st.flyType;
        Globals::Misc::Fly_Stealth         = st.flyStealth;
        Globals::Misc::Fly_BypassPulse     = st.flyBypass;
        Globals::Misc::Fly_BypassStrength  = st.flyBypassStrength;
        Globals::Misc::Fly_BypassInterval  = st.flyBypassInterval;
        Globals::Misc::Fly_BypassDuration  = st.flyBypassDuration;
        Globals::Movement::Speed           = st.speedEnabled;
        Globals::Movement::Speed_Type      = st.speedType;
        Globals::Movement::Speed_Key       = VKToImGuiKey(st.speedKey);
        Globals::Movement::Speed_Mode      = static_cast<ImKeyBindMode>(st.speedMode);
        Globals::Movement::JumpPower       = st.jumpPowerEnabled;
        Globals::Movement::JumpPower_Key   = VKToImGuiKey(st.jumpPowerKey);
        Globals::Movement::JumpPower_Mode  = static_cast<ImKeyBindMode>(st.jumpPowerMode);
        Globals::Movement::Macro           = st.macroEnabled;
        Globals::Movement::Macro_Type      = st.macroType;
        Globals::Movement::Macro_Key       = VKToImGuiKey(st.macroKey);
        Globals::Movement::Macro_Mode      = static_cast<ImKeyBindMode>(st.macroMode);
        Globals::Movement::Spinbot         = st.spinbotEnabled;
        Globals::Movement::Spinbot_Type    = st.spinbotType;
        Globals::Movement::Spinbot_Speed   = st.spinbotSpeed;
        Globals::Misc::Fly_Speed            = st.flySpeed;
        Globals::Movement::Speed_Value      = st.speedVal;
        Globals::Movement::JumpPower_Value  = st.jumpPowerVal;
        Globals::Settings::Performance_Mode = st.perfMode;
        std::copy_n(st.accentColor, 4, Globals::Settings::AccentColor);
        Globals::Settings::FontIndex       = st.fontIndex;

        Globals::Radar::Enabled  = st.radEnabled;
        Globals::Radar::Size     = st.radSize;
        Globals::Radar::Range    = st.radRange;
        Globals::Radar::Opacity  = st.radOpacity;
        Globals::Radar::EnemyFlags = st.radFlags;

        Globals::Visuals::LookDirection   = st.lookDir;
        std::copy_n(st.lookDirColor, 4, Globals::Visuals::LookDirColor);

        Globals::Visuals::Snapline         = st.snapline;
        Globals::Visuals::SnaplinePosition = st.snaplinePos;
        Globals::Visuals::HeadDot          = st.headDot;
        Globals::Visuals::HeadDotSize      = st.headDotSize;
        std::copy_n(st.snaplineCol, 4, Globals::Visuals::Colors::Snapline);
        std::copy_n(st.headDotCol, 4, Globals::Visuals::Colors::HeadDot);

        Globals::Visuals::HitEffectsType       = st.hitEffectsType;
        Globals::Visuals::HitEffectsDuration   = st.hitEffectsDuration;
        std::copy_n(st.hitEffectsColor, 4, Globals::Visuals::HitEffectsColor);
        Globals::Visuals::HitNotifications     = st.hitNotifications;
        Globals::Visuals::ShotTracers          = st.shotTracers;
        Globals::Visuals::ShotTracerType       = st.shotTracerType;
        std::copy_n(st.shotTracerColor, 4, Globals::Visuals::ShotTracerColor);

        Globals::Watermark::Enabled        = st.watermark;
        Globals::Watermark::ShowPlayers    = st.wmShowPlayers;
        Globals::Watermark::ShowGame       = st.wmShowGame;
    }

    static void DrawBreadcrumb(ImDrawList* dl, ImVec2 wp, float sepY, float sepH, const char* name, float alpha, float yOff)
    {
        int ta = static_cast<int>(232 * alpha);
        int aa = static_cast<int>(255 * alpha);
        ImVec2 ns = ImGui::CalcTextSize(name);
        ImVec2 ms = ImGui::CalcTextSize("Main");
        float tx = 16.f;
        float dx = tx + ns.x + 8.f;
        float mx = dx + 8.f;
        float cy = sepY + sepH * 0.5f + yOff;
        float fcy = sepY + sepH * 0.5f;
        dl->AddText(ImVec2(wp.x + tx, wp.y + cy - ns.y * 0.5f), Theme::Text(ta), name);
        dl->AddCircleFilled(ImVec2(wp.x + dx, wp.y + fcy), 1.75f, Theme::Accent(aa), 12);
        dl->AddText(ImVec2(wp.x + mx, wp.y + fcy - ms.y * 0.5f), Theme::Accent(aa), "Main");
    }

    void ApplyStyle() { Theme::ApplyStyle(); }

    void Render()
    {
        // ── Snapshot Globals → st at start of frame ──
        SyncGlobalsToSt();

        const float w = 800.f, h = 530.f;
        const char* tabs[] = { "Combat", "Visuals", "Effects", "Movement", "Settings", "Configs" };
        const char* icons[] = { "\xef\x81\x9b", "\xef\x81\xae", "\xef\x82\xac", "\xef\x9c\x8c", "\xef\x80\x93", "\xef\x83\x87" };
        const float marginR = 24.f, iconGap = 5.f, tabGap = 8.f, tabPadX = 5.f, tabPadY = 3.f;
        const float topY = 9.f, sepY = 43.f, sepH = 34.f, footH = 28.f;
        const float dt = ImGui::GetIO().DeltaTime;

        {
            float rate = g_MenuOpen ? 6.0f : 7.5f;
            menuAlpha += ((g_MenuOpen ? 1.0f : 0.0f) - menuAlpha) * (1.0f - std::exp(-rate * dt));
        }
        if (g_MenuOpen)
            menuScale += (1.0f - menuScale) * (1.0f - std::exp(-10.0f * dt));
        if (menuAlpha > 0.999f) { menuAlpha = 1.0f; menuScale = 1.0f; }
        if (!g_MenuOpen && menuAlpha < 0.001f)
        {
            menuAlpha = 0.0f;
            menuScale = 0.85f;
            SyncStToGlobals();
            return;
        }

        // ── Background particle effects ──
        if (bgEffect != 0)
        {
            ImDrawList* bg = ImGui::GetBackgroundDrawList();
            ImVec2 screen = ImGui::GetIO().DisplaySize;
            float t = static_cast<float>(ImGui::GetTime());
            float a = menuAlpha;
            int pr = static_cast<int>(st.accentColor[0] * 255.f);
            int pg = static_cast<int>(st.accentColor[1] * 255.f);
            int pb = static_cast<int>(st.accentColor[2] * 255.f);

            if (bgEffect == 1)
            {
                struct Flake { float x, y, vy, radius, wobble, opacity; };
                static std::vector<Flake> flakes;
                static int lastCount = -1;
                if (lastCount != particleCount)
                {
                    lastCount = particleCount;
                    flakes.resize(particleCount);
                    for (auto& f : flakes)
                    {
                        f.x       = (static_cast<float>(std::rand()) / RAND_MAX) * 3840.f;
                        f.y       = (static_cast<float>(std::rand()) / RAND_MAX) * 2160.f;
                        f.vy      = 12.f + (static_cast<float>(std::rand()) / RAND_MAX) * 18.f;
                        f.radius  = 1.0f + (static_cast<float>(std::rand()) / RAND_MAX) * 1.8f;
                        f.wobble  = (static_cast<float>(std::rand()) / RAND_MAX) * 6.28f;
                        f.opacity = 0.35f + (static_cast<float>(std::rand()) / RAND_MAX) * 0.5f;
                    }
                }
                float wind = sinf(t * 0.18f) * 22.f + sinf(t * 0.07f) * 14.f;
                for (auto& fl : flakes)
                {
                    if (a > 0.01f)
                    {
                        float sway = sinf(t * 0.55f + fl.wobble) * 9.f + cosf(t * 0.28f + fl.wobble * 0.5f) * 5.f;
                        fl.y += fl.vy * particleSpeed * dt;
                        fl.x += (wind + sway) * dt;
                        if (fl.y > screen.y + 8.f)  { fl.y = -8.f; fl.x = (static_cast<float>(std::rand()) / RAND_MAX) * screen.x; }
                        if (fl.x > screen.x + 12.f) fl.x = -12.f;
                        if (fl.x < -12.f)            fl.x = screen.x + 12.f;
                    }
                    int alpha = static_cast<int>(fl.opacity * a * 200);
                    bg->AddCircleFilled(ImVec2(fl.x, fl.y), fl.radius, IM_COL32(pr, pg, pb, alpha));
                    if (fl.radius > 2.0f)
                        bg->AddCircleFilled(ImVec2(fl.x, fl.y), fl.radius * 2.0f, IM_COL32(pr, pg, pb, alpha / 6));
                }
            }
            else if (bgEffect == 2)
            {
                struct Drop { float x, y, len, vy, opacity; };
                static std::vector<Drop> drops;
                static int lastCount2 = -1;
                if (lastCount2 != particleCount)
                {
                    lastCount2 = particleCount;
                    drops.resize(particleCount);
                    for (auto& d : drops)
                    {
                        d.x       = (static_cast<float>(std::rand()) / RAND_MAX) * 3840.f;
                        d.y       = (static_cast<float>(std::rand()) / RAND_MAX) * 2160.f;
                        d.len     = 8.f  + (static_cast<float>(std::rand()) / RAND_MAX) * 16.f;
                        d.vy      = 280.f + (static_cast<float>(std::rand()) / RAND_MAX) * 180.f;
                        d.opacity = 0.3f + (static_cast<float>(std::rand()) / RAND_MAX) * 0.5f;
                    }
                }
                for (auto& d : drops)
                {
                    if (a > 0.01f)
                    {
                        d.y += d.vy * particleSpeed * dt;
                        d.x += 60.f * particleSpeed * dt;
                        if (d.y > screen.y + d.len) { d.y = -d.len; d.x = (static_cast<float>(std::rand()) / RAND_MAX) * screen.x; }
                        if (d.x > screen.x + 20.f)   d.x = -20.f;
                    }
                    int alpha = static_cast<int>(d.opacity * a * 200);
                    bg->AddLine(ImVec2(d.x, d.y), ImVec2(d.x + d.len * 0.2f, d.y + d.len),
                        IM_COL32(pr, pg, pb, alpha), 1.0f);
                }
            }
            else if (bgEffect == 3)
            {
                struct Node { float x, y, vx, vy; };
                static std::vector<Node> nodes;
                static int lastCount3 = -1;
                if (lastCount3 != particleCount)
                {
                    lastCount3 = particleCount;
                    nodes.resize(particleCount);
                    for (auto& n : nodes)
                    {
                        n.x  = (static_cast<float>(std::rand()) / RAND_MAX) * 3840.f;
                        n.y  = (static_cast<float>(std::rand()) / RAND_MAX) * 2160.f;
                        n.vx = ((static_cast<float>(std::rand()) / RAND_MAX) - 0.5f) * 30.f;
                        n.vy = ((static_cast<float>(std::rand()) / RAND_MAX) - 0.5f) * 30.f;
                    }
                }
                if (a > 0.01f)
                {
                    for (auto& n : nodes)
                    {
                        n.x += n.vx * particleSpeed * dt;
                        n.y += n.vy * particleSpeed * dt;
                        if (n.x < 0.f || n.x > screen.x) n.vx = -n.vx;
                        if (n.y < 0.f || n.y > screen.y) n.vy = -n.vy;
                    }
                }
                const float maxDist = 160.f;
                for (int i = 0; i < static_cast<int>(nodes.size()); i++)
                {
                    for (int j = i + 1; j < static_cast<int>(nodes.size()); j++)
                    {
                        float dx = nodes[i].x - nodes[j].x;
                        float dy = nodes[i].y - nodes[j].y;
                        float dist = sqrtf(dx * dx + dy * dy);
                        if (dist < maxDist)
                        {
                            float fade = 1.0f - dist / maxDist;
                            int alpha = static_cast<int>(fade * a * 120);
                            bg->AddLine(ImVec2(nodes[i].x, nodes[i].y),
                                ImVec2(nodes[j].x, nodes[j].y),
                                IM_COL32(pr, pg, pb, alpha), 1.0f);
                        }
                    }
                }
                for (auto& n : nodes)
                    bg->AddCircleFilled(ImVec2(n.x, n.y), 2.5f,
                        IM_COL32(pr, pg, pb, static_cast<int>(a * 180)));
            }
            else if (bgEffect == 4)
            {
                struct Bubble { float x, y, vx, vy, radius, opacity, life, maxLife; };
                static std::vector<Bubble> bubbles;
                static int lastCount4 = -1;
                if (lastCount4 != particleCount)
                {
                    lastCount4 = particleCount;
                    bubbles.resize(particleCount);
                    for (auto& p : bubbles)
                    {
                        p.maxLife = 2.f + (static_cast<float>(std::rand()) / RAND_MAX) * 4.f;
                        p.life    = (static_cast<float>(std::rand()) / RAND_MAX) * p.maxLife;
                        p.x       = (static_cast<float>(std::rand()) / RAND_MAX) * 3840.f;
                        p.y       = (static_cast<float>(std::rand()) / RAND_MAX) * 2160.f;
                        p.vx      = ((static_cast<float>(std::rand()) / RAND_MAX) - 0.5f) * 40.f;
                        p.vy      = ((static_cast<float>(std::rand()) / RAND_MAX) - 0.5f) * 40.f;
                        p.radius  = 1.5f + (static_cast<float>(std::rand()) / RAND_MAX) * 3.f;
                        p.opacity = 0.4f + (static_cast<float>(std::rand()) / RAND_MAX) * 0.6f;
                    }
                }
                for (auto& p : bubbles)
                {
                    if (a > 0.01f)
                    {
                        p.life += particleSpeed * dt;
                        if (p.life > p.maxLife)
                        {
                            p.life = 0.f;
                            p.x    = (static_cast<float>(std::rand()) / RAND_MAX) * screen.x;
                            p.y    = (static_cast<float>(std::rand()) / RAND_MAX) * screen.y;
                            p.vx   = ((static_cast<float>(std::rand()) / RAND_MAX) - 0.5f) * 40.f;
                            p.vy   = ((static_cast<float>(std::rand()) / RAND_MAX) - 0.5f) * 40.f;
                        }
                        p.x += p.vx * particleSpeed * dt;
                        p.y += p.vy * particleSpeed * dt;
                    }
                    float lifeRatio = p.life / p.maxLife;
                    float lifeFade  = lifeRatio < 0.2f ? lifeRatio / 0.2f
                        : lifeRatio > 0.8f ? (1.f - lifeRatio) / 0.2f : 1.f;
                    int alpha = static_cast<int>(p.opacity * lifeFade * a * 220);
                    bg->AddCircleFilled(ImVec2(p.x, p.y), p.radius, IM_COL32(pr, pg, pb, alpha));
                    bg->AddCircleFilled(ImVec2(p.x, p.y), p.radius * 2.5f, IM_COL32(pr, pg, pb, alpha / 5));
                }
            }
        }

        // ── Scale style alpha by menu alpha ──
        ImGuiStyle& style = ImGui::GetStyle();
        float savedAlpha = style.Alpha;
        style.Alpha = savedAlpha * menuAlpha;
        Theme::SetMenuAlpha(menuAlpha);
        Theme::SetAccentColor(st.accentColor);

        static int prevFontIndex = -1;
        if (st.fontIndex != prevFontIndex)
        {
            prevFontIndex = st.fontIndex;
            UiFont::SwitchTo(st.fontIndex);
        }

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
        float menuX = (viewportSize.x - w) * 0.5f;
        float menuY = (viewportSize.y - h) * 0.5f;
        ImGui::SetNextWindowPos(ImVec2(menuX, menuY), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(w, h));
        ImGui::Begin("##root", nullptr, flags);

        // Drag the ImGui window by clicking empty space inside it
        static bool menuDragging = false;
        static ImVec2 menuDragOffset;
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
        {
            menuDragging = true;
            ImVec2 mp = ImGui::GetMousePos(), wp2 = ImGui::GetWindowPos();
            menuDragOffset = ImVec2(mp.x - wp2.x, mp.y - wp2.y);
        }
        if (menuDragging)
        {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                ImVec2 mp = ImGui::GetMousePos();
                ImGui::SetWindowPos(ImVec2(mp.x - menuDragOffset.x, mp.y - menuDragOffset.y));
            }
            else
            {
                menuDragging = false;
            }
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 wp = ImGui::GetWindowPos();

        for (float x = wp.x + 12.f; x <= wp.x + w - 12.f; x += 48.f)
            dl->AddLine(ImVec2(x, wp.y), ImVec2(x, wp.y + h), Theme::Outline(72));
        for (float y = wp.y + 12.f; y <= wp.y + h - 12.f; y += 48.f)
            dl->AddLine(ImVec2(wp.x, y), ImVec2(wp.x + w, y), Theme::Outline(72));

        dl->AddRectFilled(ImVec2(wp.x, wp.y + sepY), ImVec2(wp.x + w, wp.y + sepY + sepH), Theme::SurfaceLight(238));
        dl->AddRectFilled(ImVec2(wp.x, wp.y + h - footH), ImVec2(wp.x + w, wp.y + h), Theme::SurfaceLight());
        dl->AddRect(wp, ImVec2(wp.x + w, wp.y + h), Theme::Outline(165), 12.f);

        ImVec2 fts = ImGui::CalcTextSize("slotted roblox external");
        dl->AddText(ImVec2(wp.x + 16, wp.y + h - footH + (footH - fts.y) * 0.5f), Theme::MutedText(), "slotted roblox external");
        dl->AddText(ImVec2(wp.x + 16, wp.y + 12), Theme::Text(), "scare.lol");

        if (visibleCrumbTab != activeTab) { previousCrumbTab = visibleCrumbTab; visibleCrumbTab = activeTab; crumbFade = 0.f; }
        crumbFade += (1.f - crumbFade) * (1.f - std::exp(-dt * 12.f));
        if (crumbFade < 0.98f)
            DrawBreadcrumb(dl, wp, sepY, sepH, tabs[previousCrumbTab], 1.f - crumbFade, -5.f * crumbFade);
        DrawBreadcrumb(dl, wp, sepY, sepH, tabs[visibleCrumbTab], crumbFade, 5.f * (1.f - crumbFade));

        float totalW = 0, itemHs = 0;
        float iws[6]{}; ImVec2 ics[6], txs[6];
        for (int i = 0; i < 6; ++i) {
            ics[i] = ImGui::CalcTextSize(icons[i]);
            txs[i] = ImGui::CalcTextSize(tabs[i]);
            iws[i] = ics[i].x + iconGap + txs[i].x + tabPadX * 2.f;
            totalW += iws[i] + (i < 5 ? tabGap : 0);
            float maxIconText = ics[i].y > txs[i].y ? ics[i].y : txs[i].y;
            itemHs = itemHs > maxIconText ? itemHs : maxIconText;
        }
        itemHs += tabPadY * 2.f;
        float startX = w - marginR - totalW;
        if (startX < 24.f) startX = 24.f;
        float cx = startX, cy = topY + itemHs * 0.5f;
        for (int i = 0; i < 6; ++i) {
            float tgt = (activeTab == i) ? 1.f : 0.45f;
            tabFade[i] += (tgt - tabFade[i]) * (1.f - std::exp(-dt * 14.f));
            float amt = Theme::Clamp((tabFade[i] - 0.45f) / 0.55f, 0.f, 1.f);
            float ix = cx;
            if (amt > 0.01f)
                dl->AddRectFilled(ImVec2(wp.x + ix, wp.y + topY), ImVec2(wp.x + ix + iws[i], wp.y + topY + itemHs), Theme::Accent(static_cast<int>(180 * amt)), 5);
            ImVec4 ic = ImGui::ColorConvertU32ToFloat4(Theme::MutedText());
            ImVec4 ac = Theme::AccentVec();
            ImVec4 cv = ImVec4(ic.x + (ac.x - ic.x) * amt, ic.y + (ac.y - ic.y) * amt, ic.z + (ac.z - ic.z) * amt, 1.f * menuAlpha);
            ImU32 col = ImGui::ColorConvertFloat4ToU32(cv);
            ImGui::PushID(i);
            ImGui::SetCursorPos(ImVec2(ix, topY));
            if (ImGui::InvisibleButton("##tab", ImVec2(iws[i], itemHs))) activeTab = i;
            ImGui::PopID();
            float cxx = ix + tabPadX;
            dl->AddText(ImVec2(wp.x + cxx, wp.y + cy - ics[i].y * 0.5f), col, icons[i]);
            dl->AddText(ImVec2(wp.x + cxx + ics[i].x + iconGap, wp.y + cy - txs[i].y * 0.5f), col, tabs[i]);
            cx += iws[i] + tabGap;
        }

        const float cTop = sepY + sepH + 12.f;
        const float cLeft = 16.f, colGap = 8.f;
        const float pW = (w - cLeft * 2.f - colGap) * 0.5f;
        const float pH = h - footH - cTop - 8.f;
        ImVec2 lt(wp.x + cLeft, wp.y + cTop);
        ImVec2 rt(lt.x + pW + colGap, lt.y);

        auto KeyBindRow = [&](ImVec2 panelPos, float panelW, float& yy, const char* label, int* key, int* mode = nullptr) {
            if (mode)
                Widgets::KeyBindEx(dl, label, ImVec2(panelPos.x + 20, panelPos.y + yy), panelW - 40.f, label, key, mode);
            else
                Widgets::KeyBind(dl, label, ImVec2(panelPos.x + 20, panelPos.y + yy), panelW - 40.f, label, key);
            yy += 28.f;
        };

        auto BeginPanel = [&](ImVec2 pos, ImVec2 size, float& scrollOff) -> bool {
            ImGui::PushClipRect(ImVec2(pos.x + 6, pos.y + 38), ImVec2(pos.x + size.x - 6, pos.y + size.y - 4), false);
            return true;
        };

        auto EndPanel = [&](ImVec2 pos, ImVec2 size, float& scrollOff, float contentEndY) {
            ImGui::PopClipRect();
            float visibleH = size.y - 42.f;
            float contentH = contentEndY - scrollOff - 44.f;
            if (contentH > visibleH && visibleH > 0.f) {
                float maxScroll = contentH - visibleH;
                ImVec2 cMin(pos.x + 6, pos.y + 38);
                ImVec2 cMax(pos.x + size.x - 6, pos.y + size.y - 4);
                if (ImGui::IsMouseHoveringRect(cMin, cMax)) {
                    float wheel = ImGui::GetIO().MouseWheel;
                    if (wheel != 0.f) scrollOff += wheel * 28.f;
                }
                if (scrollOff > 0.f) scrollOff = 0.f;
                if (scrollOff < -maxScroll) scrollOff = -maxScroll;
                float trackX = pos.x + size.x - 8.f;
                float trackY1 = pos.y + 38.f;
                float trackY2 = pos.y + size.y - 4.f;
                float trackH = trackY2 - trackY1;
                float thumbH = (visibleH / contentH) * trackH;
                float thumbPos = (-scrollOff / maxScroll) * (trackH - thumbH);
                dl->AddRectFilled(ImVec2(trackX, trackY1), ImVec2(trackX + 4.f, trackY2), Theme::Outline(100), 2.f);
                dl->AddRectFilled(ImVec2(trackX, trackY1 + thumbPos), ImVec2(trackX + 4.f, trackY1 + thumbPos + thumbH), Theme::Accent(160), 2.f);
                ImGui::PushID(&scrollOff);
                ImGui::SetCursorScreenPos(ImVec2(trackX, trackY1));
                ImGui::InvisibleButton("##sbar", ImVec2(6.f, trackH));
                bool sbarActive = ImGui::IsItemActive();
                if (sbarActive) {
                    float t = (ImGui::GetIO().MousePos.y - trackY1 - thumbH * 0.5f) / (trackH - thumbH);
                    if (t < 0.f) t = 0.f;
                    if (t > 1.f) t = 1.f;
                    scrollOff = -t * maxScroll;
                }
                bool sbarHovered = ImGui::IsItemHovered();
                if (sbarHovered) {
                    dl->AddRectFilled(ImVec2(trackX, trackY1), ImVec2(trackX + 4.f, trackY2), Theme::Accent(60), 2.f);
                }
                ImGui::PopID();
            } else {
                scrollOff = 0.f;
            }
        };

        // ---- TAB: Combat (0) ----
        if (activeTab == 0)
        {
            {
                bool& exp = leftPanelOpen[0];
                Widgets::Panel(dl, "aimbot_panel", lt, ImVec2(pW, pH), "Aimbot", &exp);
                if (exp) {
                    BeginPanel(lt, ImVec2(pW, pH), leftScroll[0]);
                    float yy = 44.f + leftScroll[0];

                    dl->AddText(ImVec2(lt.x + 20, lt.y + yy + 4), Theme::MutedText(200), "General"); yy += 22.f;
                    Widgets::Checkbox(dl, "aim_enabled", ImVec2(lt.x + 20, lt.y + yy), "Enabled", &st.aimbotEnabled); yy += 30.f;
                    KeyBindRow(lt, pW, yy, "Aimbot Key", &st.aimbotKey, &st.aimbotMode);
                    Widgets::Checkbox(dl, "aim_sticky", ImVec2(lt.x + 20, lt.y + yy), "Sticky aim", &st.aimbotSticky); yy += 30.f;
                    Widgets::Checkbox(dl, "aim_knocked", ImVec2(lt.x + 20, lt.y + yy), "Knocked check", &st.knockedCheck); yy += 30.f;
                    Widgets::Checkbox(dl, "aim_vis", ImVec2(lt.x + 20, lt.y + yy), "Visibility check", &st.visCheck); yy += 30.f;

                    dl->AddLine(ImVec2(lt.x + 20, lt.y + yy + 4), ImVec2(lt.x + pW - 20, lt.y + yy + 4), Theme::Outline(60)); yy += 14.f;
                    dl->AddText(ImVec2(lt.x + 20, lt.y + yy + 4), Theme::MutedText(200), "Targeting"); yy += 22.f;
                    static const char* aimTypes[] = { "Mouse", "Camera" };
                    Widgets::Combo(dl, "aim_type", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Type", &st.aimbotType, aimTypes, 2); yy += 48.f;
                    static const char* parts[] = { "Head", "HumanoidRootPart", "UpperTorso", "LowerTorso", "Left Hand", "Right Hand",
                        "Left Upper Arm", "Right Upper Arm", "Left Upper Leg", "Right Upper Leg", "Left Foot", "Right Foot" };
                    Widgets::Combo(dl, "aim_part", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Aim Part", &st.hitPart, parts, 12); yy += 48.f;

                    dl->AddLine(ImVec2(lt.x + 20, lt.y + yy + 4), ImVec2(lt.x + pW - 20, lt.y + yy + 4), Theme::Outline(60)); yy += 14.f;
                    dl->AddText(ImVec2(lt.x + 20, lt.y + yy + 4), Theme::MutedText(200), "Smoothing"); yy += 22.f;
                    if (st.aimbotType == 0) {
                        Widgets::SliderFloat(dl, "aim_msx", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Mouse X", &st.mSmoothX, 0.f, 12.f, "%.1f", &st.vMSmoothX); yy += 38.f;
                        Widgets::SliderFloat(dl, "aim_msy", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Mouse Y", &st.mSmoothY, 0.f, 12.f, "%.1f", &st.vMSmoothY); yy += 38.f;
                        Widgets::SliderFloat(dl, "aim_msens", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Sensitivity", &st.mSens, 0.f, 5.f, "%.1f", &st.vMSens); yy += 38.f;
                    } else {
                        Widgets::SliderFloat(dl, "aim_csx", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Camera X", &st.cSmoothX, 0.f, 12.f, "%.1f", &st.vCSmoothX); yy += 38.f;
                        Widgets::SliderFloat(dl, "aim_csy", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Camera Y", &st.cSmoothY, 0.f, 12.f, "%.1f", &st.vCSmoothY); yy += 38.f;
                    }

                    dl->AddLine(ImVec2(lt.x + 20, lt.y + yy + 4), ImVec2(lt.x + pW - 20, lt.y + yy + 4), Theme::Outline(60)); yy += 14.f;
                    dl->AddText(ImVec2(lt.x + 20, lt.y + yy + 4), Theme::MutedText(200), "Prediction & Shake"); yy += 22.f;
                    Widgets::Checkbox(dl, "aim_pred", ImVec2(lt.x + 20, lt.y + yy), "Prediction", &st.prediction); yy += 30.f;
                    if (st.prediction) {
                        Widgets::SliderFloat(dl, "aim_px", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Predict X", &st.predX, 0.f, 20.f, "%.1f", &st.vPredX); yy += 38.f;
                        Widgets::SliderFloat(dl, "aim_py", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Predict Y", &st.predY, 0.f, 20.f, "%.1f", &st.vPredY); yy += 38.f;
                    }
                    Widgets::Checkbox(dl, "aim_shake", ImVec2(lt.x + 20, lt.y + yy), "Shake", &st.shake); yy += 30.f;
                    if (st.shake) {
                        Widgets::SliderFloat(dl, "aim_sx", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Shake X", &st.shakeX, -5.f, 5.f, "%.1f", &st.vShakeX); yy += 38.f;
                        Widgets::SliderFloat(dl, "aim_sy", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Shake Y", &st.shakeY, -5.f, 5.f, "%.1f", &st.vShakeY); yy += 38.f;
                    }

                    if (st.aimbotEnabled) {
                        dl->AddLine(ImVec2(lt.x + 20, lt.y + yy + 4), ImVec2(lt.x + pW - 20, lt.y + yy + 4), Theme::Outline(60)); yy += 14.f;
                        dl->AddText(ImVec2(lt.x + 20, lt.y + yy + 4), Theme::MutedText(200), "FOV"); yy += 22.f;
                        Widgets::Checkbox(dl, "aim_fov_draw", ImVec2(lt.x + 20, lt.y + yy), "Draw FOV", &st.drawFov);
                        if (st.drawFov) { Widgets::ColorPicker(dl, "aim_fov_col", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "##fovc", st.fovColor); } yy += 30.f;
                        if (st.drawFov) {
                            Widgets::SliderFloat(dl, "aim_fov_sz", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Size", &st.fovSize, 1.f, 500.f, "%.0f", &st.vFovSize); yy += 38.f;
                            Widgets::Checkbox(dl, "aim_fov_spin", ImVec2(lt.x + 20, lt.y + yy), "Spin", &st.fovSpin); yy += 30.f;
                            if (st.fovSpin) { Widgets::SliderInt(dl, "aim_fov_spd", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Speed", &st.fovSpinSpeed, 1, 5, "%d", &st.vFovSpinSpeed); yy += 38.f; }
                            Widgets::Checkbox(dl, "aim_use_fov", ImVec2(lt.x + 20, lt.y + yy), "Use FOV", &st.useFov); yy += 30.f;
                        }
                    }
                    EndPanel(lt, ImVec2(pW, pH), leftScroll[0], yy);
                }
            }

            // Right side: 3 stacked panels
            float rpY = rt.y;

            // ── Silent Aim ──
            {
                const ImVec2 sp(rt.x, rpY);
                Widgets::Panel(dl, "sil_panel", sp, ImVec2(pW, pH), "Silent Aim", &silentPanelOpen);
                if (silentPanelOpen) {
                    BeginPanel(sp, ImVec2(pW, pH), silentScroll);
                    float yy = 44.f + silentScroll;
                    Widgets::Checkbox(dl, "sil_enabled", ImVec2(sp.x + 20, sp.y + yy), "Enabled", &st.silentEnabled); yy += 30.f;
                    KeyBindRow(sp, pW, yy, "Silent Key", &st.silentKey, &st.silentMode);
                    Widgets::Checkbox(dl, "sil_sticky", ImVec2(sp.x + 20, sp.y + yy), "Sticky aim", &st.silentSticky); yy += 30.f;
                    Widgets::Checkbox(dl, "sil_knocked", ImVec2(sp.x + 20, sp.y + yy), "Knocked check", &st.silentKnocked); yy += 30.f;
                    Widgets::Checkbox(dl, "sil_vis", ImVec2(sp.x + 20, sp.y + yy), "Visibility check", &st.silentVis); yy += 30.f;
                    Widgets::Checkbox(dl, "sil_spoof", ImVec2(sp.x + 20, sp.y + yy), "Mouse spoof", &st.silentSpoof); yy += 30.f;
                    Widgets::Checkbox(dl, "sil_raycast", ImVec2(sp.x + 20, sp.y + yy), "Raycast", &st.silentRaycast); yy += 30.f;
                    static const char* parts[] = { "Head", "HumanoidRootPart", "UpperTorso", "LowerTorso", "Left Hand", "Right Hand",
                        "Left Upper Arm", "Right Upper Arm", "Left Upper Leg", "Right Upper Leg", "Left Foot", "Right Foot" };
                    Widgets::Combo(dl, "sil_part", ImVec2(sp.x + 20, sp.y + yy), pW - 40.f, "Aim Part", &st.silentPart, parts, 12); yy += 48.f;
                    if (st.silentEnabled) {
                        yy += 4.f;
                        Widgets::Checkbox(dl, "sil_fov_draw", ImVec2(sp.x + 20, sp.y + yy), "Draw FOV", &st.silentDrawFov);
                        if (st.silentDrawFov) { Widgets::ColorPicker(dl, "sil_fov_col", ImVec2(sp.x + 20, sp.y + yy), pW - 40.f, "##sfovc", st.silentFovColor); } yy += 30.f;
                        if (st.silentDrawFov) {
                            Widgets::Checkbox(dl, "sil_gun_fov", ImVec2(sp.x + 20, sp.y + yy), "Gun based FOV", &st.silentGunFov); yy += 30.f;
                            if (st.silentGunFov) {
                                Widgets::SliderFloat(dl, "sil_fov_def", ImVec2(sp.x + 20, sp.y + yy), pW - 40.f, "Default", &st.silentFov, 0.f, 300.f, "%.0f", &st.vSilentFov); yy += 38.f;
                                Widgets::SliderFloat(dl, "sil_fov_db", ImVec2(sp.x + 20, sp.y + yy), pW - 40.f, "Double Barrel", &st.silentFovDb, 0.f, 300.f, "%.0f", &st.vSilentFovDb); yy += 38.f;
                                Widgets::SliderFloat(dl, "sil_fov_tac", ImVec2(sp.x + 20, sp.y + yy), pW - 40.f, "Tactical", &st.silentFovTac, 0.f, 300.f, "%.0f", &st.vSilentFovTac); yy += 38.f;
                                Widgets::SliderFloat(dl, "sil_fov_rev", ImVec2(sp.x + 20, sp.y + yy), pW - 40.f, "Revolver", &st.silentFovRev, 0.f, 300.f, "%.0f", &st.vSilentFovRev); yy += 38.f;
                            } else {
                                Widgets::SliderFloat(dl, "sil_fov_static", ImVec2(sp.x + 20, sp.y + yy), pW - 40.f, "Static FOV", &st.silentFov, 0.f, 500.f, "%.0f", &st.vSilentFov); yy += 38.f;
                            }
                            Widgets::Checkbox(dl, "sil_fov_spin", ImVec2(sp.x + 20, sp.y + yy), "Spin", &st.silentFovSpin); yy += 30.f;
                            if (st.silentFovSpin) { Widgets::SliderInt(dl, "sil_fov_spd", ImVec2(sp.x + 20, sp.y + yy), pW - 40.f, "Spin Speed", &st.silentFovSpinSpeed, 1, 5, "%d", &st.vSilentFovSpinSpeed); yy += 38.f; }
                            Widgets::Checkbox(dl, "sil_use_fov", ImVec2(sp.x + 20, sp.y + yy), "Use FOV", &st.silentUseFov); yy += 30.f;
                        }
                    }
                    EndPanel(sp, ImVec2(pW, pH), silentScroll, yy);
                }
                rpY += pH + 8.f;
            }
        }

        // ---- TAB: Visuals (1) ----
        if (activeTab == 1)
        {
            {
                bool& exp = leftPanelOpen[1];
                Widgets::Panel(dl, "vis_panel", lt, ImVec2(pW, pH), "Visuals", &exp);
                if (exp) {
                    BeginPanel(lt, ImVec2(pW, pH), leftScroll[1]);
                    float yy = 44.f + leftScroll[1];
                    Widgets::Checkbox(dl, "vis_enabled", ImVec2(lt.x + 20, lt.y + yy), "Enabled", &st.visEnabled); yy += 30.f;
                    Widgets::Checkbox(dl, "vis_box", ImVec2(lt.x + 20, lt.y + yy), "Box", &st.box);
                    if (st.box) { Widgets::ColorPicker(dl, "box_c", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "##boxc", st.boxColor); } yy += 30.f;
                    if (st.box) {
                        Widgets::Checkbox(dl, "vis_box_fill", ImVec2(lt.x + 20, lt.y + yy), "Box Fill", &st.boxFill);
                        if (st.boxFill) { Widgets::ColorPicker(dl, "boxft_c", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "##boxft", st.boxFillTop); } yy += 30.f;
                    }
                    Widgets::Checkbox(dl, "vis_hb", ImVec2(lt.x + 20, lt.y + yy), "Healthbar", &st.healthbar);
                    if (st.healthbarType == 0) { Widgets::ColorPicker(dl, "hb_c", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "##hbc", st.healthColor); } yy += 30.f;
                    Widgets::Checkbox(dl, "vis_health", ImVec2(lt.x + 20, lt.y + yy), "Health", &st.health);
                    Widgets::ColorPicker(dl, "health_c", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "##healthc", st.healthTextColor); yy += 30.f;
                    Widgets::Checkbox(dl, "vis_name", ImVec2(lt.x + 20, lt.y + yy), "Name", &st.name);
                    Widgets::ColorPicker(dl, "name_c", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "##namec", st.nameColor); yy += 30.f;
                    Widgets::Checkbox(dl, "vis_dist", ImVec2(lt.x + 20, lt.y + yy), "Distance", &st.distance);
                    Widgets::ColorPicker(dl, "dist_c", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "##distc", st.distColor); yy += 30.f;
                    Widgets::Checkbox(dl, "vis_rig", ImVec2(lt.x + 20, lt.y + yy), "Rig Type", &st.rigType);
                    Widgets::ColorPicker(dl, "rig_c", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "##rigc", st.rigColor); yy += 30.f;
                    Widgets::Checkbox(dl, "vis_tool", ImVec2(lt.x + 20, lt.y + yy), "Tool", &st.tool);
                    Widgets::ColorPicker(dl, "tool_c", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "##toolc", st.toolColor); yy += 30.f;
                    Widgets::Checkbox(dl, "vis_skel", ImVec2(lt.x + 20, lt.y + yy), "Skeleton", &st.skeleton);
                    Widgets::ColorPicker(dl, "skel_c", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "##skelc", st.skelColor); yy += 30.f;
                    dl->AddLine(ImVec2(lt.x + 20, lt.y + yy + 4), ImVec2(lt.x + pW - 20, lt.y + yy + 4), Theme::Outline(60)); yy += 14.f;
                    dl->AddText(ImVec2(lt.x + 20, lt.y + yy + 4), Theme::MutedText(200), "Chams");
                    yy += 22.f;
                    Widgets::Checkbox(dl, "vis_chams", ImVec2(lt.x + 20, lt.y + yy), "Filled Chams", &st.chams);
                    Widgets::ColorPicker(dl, "chams_c", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "##chamsc", st.chamsColor); yy += 30.f;
                    EndPanel(lt, ImVec2(pW, pH), leftScroll[1], yy);
                }
            }

            {                bool& exp = rightPanelOpen[1];
                Widgets::Panel(dl, "vis_opts_panel", rt, ImVec2(pW, pH), "Options", &exp);
                if (exp) {
                    BeginPanel(rt, ImVec2(pW, pH), rightScroll[1]);
                    float yy = 44.f + rightScroll[1];
                    Widgets::Checkbox(dl, "vis_excl_client", ImVec2(rt.x + 20, rt.y + yy), "Exclude Client", &st.exclClient); yy += 30.f;
                    Widgets::Checkbox(dl, "vis_excl_team", ImVec2(rt.x + 20, rt.y + yy), "Exclude Team", &st.exclTeam); yy += 30.f;
                    Widgets::Checkbox(dl, "vis_friend_check", ImVec2(rt.x + 20, rt.y + yy), "Friend Check", &st.friendCheck);
                    Widgets::ColorPicker(dl, "friend_c", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "##friendc", st.friendColor); yy += 30.f;
                    Widgets::SliderFloat(dl, "vis_rdist", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Render Distance", &st.renderDist, 0.f, 5000.f, "%.0f", &st.vRenderDist); yy += 38.f;
                    if (st.healthbar) {
                        static const char* hbStyles[] = { "Static", "Gradient" };
                        Widgets::Combo(dl, "vis_hb_style", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Healthbar Style", &st.healthbarType, hbStyles, 2); yy += 48.f;
                        Widgets::SliderInt(dl, "vis_hb_gap", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Healthbar Gap", &st.healthbarGap, 1, 5, "%d", &st.vHealthbarGap); yy += 38.f;
                        Widgets::SliderInt(dl, "vis_hb_thick", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Healthbar Thickness", &st.healthbarThick, 1, 5, "%d", &st.vHealthbarThick); yy += 38.f;
                    }
                    if (st.box) {
                        static const char* boxStyles[] = { "Bounding", "Corner" };
                        Widgets::Combo(dl, "vis_box_style", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Box Style", &st.boxType, boxStyles, 2); yy += 48.f;
                    }
                    if (st.boxFill) {
                        Widgets::Checkbox(dl, "vis_fill_grad", ImVec2(rt.x + 20, rt.y + yy), "Fill Gradient", &st.boxFillGrad); yy += 30.f;
                        if (st.boxFillGrad) {
                            Widgets::ColorPicker(dl, "boxfb_c", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "##boxfb", st.boxFillBot); yy += 22.f;
                            Widgets::Checkbox(dl, "vis_fill_rot", ImVec2(rt.x + 20, rt.y + yy), "Fill Rotation", &st.boxFillGradRot); yy += 30.f;
                        }
                        if (st.boxFillGradRot) {
                            static const char* rotTypes[] = { "Side", "Bottom", "Spin" };
                            Widgets::Combo(dl, "vis_rot_type", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Rotation Type", &st.boxFillType, rotTypes, 3); yy += 48.f;
                            Widgets::SliderInt(dl, "vis_rot_spd", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Rotation Speed", &st.boxFillSpeed, 1, 5, "%d", &st.vBoxFillSpeed); yy += 38.f;
                        }
                    }
                    if (st.name) {
                        static const char* nameTypes[] = { "Name", "Display Name", "Name & Display Name" };
                        Widgets::Combo(dl, "vis_name_type", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Name Display", &st.nameType, nameTypes, 3); yy += 48.f;
                    }
                    dl->AddLine(ImVec2(rt.x + 20, rt.y + yy + 4), ImVec2(rt.x + pW - 20, rt.y + yy + 4), Theme::Outline(60)); yy += 14.f;
                    dl->AddText(ImVec2(rt.x + 20, rt.y + yy + 4), Theme::MutedText(200), "ESP Upgrades");
                    yy += 22.f;

                    Widgets::Checkbox(dl, "vis_snapline", ImVec2(rt.x + 20, rt.y + yy), "Snapline", &st.snapline);
                    Widgets::ColorPicker(dl, "snapline_c", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "##snaplinec", st.snaplineCol); yy += 30.f;
                    if (st.snapline) {
                        static const char* snapPos[] = { "Bottom", "Center", "Top" };
                        Widgets::Combo(dl, "vis_snap_pos", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Position", &st.snaplinePos, snapPos, 3); yy += 48.f;
                    }
                    Widgets::Checkbox(dl, "vis_headdot", ImVec2(rt.x + 20, rt.y + yy), "Head Dot", &st.headDot);
                    Widgets::ColorPicker(dl, "headdot_c", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "##headdotc", st.headDotCol); yy += 30.f;
                    if (st.headDot) { Widgets::SliderFloat(dl, "vis_hdsize", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Head Dot Size", &st.headDotSize, 1.f, 10.f, "%.0f", &st.vHeadDotSize); yy += 38.f; }
                    Widgets::Checkbox(dl, "vis_lookdir", ImVec2(rt.x + 20, rt.y + yy), "Look Direction", &st.lookDir);
                    Widgets::ColorPicker(dl, "lookdir_c", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "##lookdir", st.lookDirColor); yy += 30.f;

                    dl->AddLine(ImVec2(rt.x + 20, rt.y + yy + 4), ImVec2(rt.x + pW - 20, rt.y + yy + 4), Theme::Outline(60)); yy += 14.f;
                    dl->AddText(ImVec2(rt.x + 20, rt.y + yy + 4), Theme::MutedText(200), "Radar");
                    yy += 22.f;

                    Widgets::Checkbox(dl, "rad_enabled", ImVec2(rt.x + 20, rt.y + yy), "Enabled", &st.radEnabled); yy += 30.f;
                    Widgets::SliderFloat(dl, "rad_size", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Size", &st.radSize, 100.f, 400.f, "%.0f", &st.vRadarSize); yy += 38.f;
                    Widgets::SliderFloat(dl, "rad_range", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Range", &st.radRange, 50.f, 500.f, "%.0f", &st.vRadarRange); yy += 38.f;
                    Widgets::SliderFloat(dl, "rad_opacity", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Opacity", &st.radOpacity, 0.1f, 1.f, "%.2f", &st.vRadarOpacity); yy += 38.f;

                    dl->AddLine(ImVec2(rt.x + 20, rt.y + yy + 4), ImVec2(rt.x + pW - 20, rt.y + yy + 4), Theme::Outline(60)); yy += 14.f;

                    EndPanel(rt, ImVec2(pW, pH), rightScroll[1], yy);
                }
            }
        }

        // ---- TAB: Effects (2) ----
        if (activeTab == 2)
        {
            {
                bool& exp = leftPanelOpen[2];
                Widgets::Panel(dl, "env_panel", lt, ImVec2(pW, pH), "Environment", &exp);
                if (exp) {
                    BeginPanel(lt, ImVec2(pW, pH), leftScroll[2]);
                    float yy = 44.f + leftScroll[2];
                    Widgets::Checkbox(dl, "w_fog", ImVec2(lt.x + 20, lt.y + yy), "Fog", &st.fog);
                    Widgets::ColorPicker(dl, "fog_c", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "##fogc", st.fogColor); yy += 30.f;
                    if (st.fog) { Widgets::SliderFloat(dl, "w_fog_dist", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Fog Distance", &st.fogDist, 0.f, 1000.f, "%.0f", &st.vFogDist); yy += 38.f; }
                    Widgets::Checkbox(dl, "w_exp", ImVec2(lt.x + 20, lt.y + yy), "Exposure", &st.exposure); yy += 30.f;
                    if (st.exposure) { Widgets::SliderFloat(dl, "w_exp_val", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Exposure Value", &st.exposureVal, -3.f, 3.f, "%.1f", &st.vExposure); yy += 38.f; }
                    Widgets::Checkbox(dl, "w_fov", ImVec2(lt.x + 20, lt.y + yy), "FOV", &st.worldFov); yy += 30.f;
                    if (st.worldFov) { Widgets::SliderFloat(dl, "w_fov_val", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "FOV Value", &st.worldFovVal, 70.f, 120.f, "%.0f", &st.vWorldFov); yy += 38.f; }
                    EndPanel(lt, ImVec2(pW, pH), leftScroll[2], yy);
                }
            }

            {
                bool& exp = rightPanelOpen[2];
                Widgets::Panel(dl, "hitfx_panel", rt, ImVec2(pW, pH), "Hit Effects", &exp);
                if (exp) {
                    BeginPanel(rt, ImVec2(pW, pH), rightScroll[2]);
                    float yy = 44.f + rightScroll[2];
                    static const char* hitTypes[] = { "Off", "Hitmarker", "Skeleton", "Chams", "Filled" };
                    Widgets::Combo(dl, "vis_hit_type", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Type", &st.hitEffectsType, hitTypes, 5); yy += 48.f;
                    if (st.hitEffectsType > 0) {
                        Widgets::SliderFloat(dl, "vis_hit_dur", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Duration", &st.hitEffectsDuration, 0.5f, 3.f, "%.1f", &st.vHitEffectsDuration); yy += 38.f;
                        Widgets::Checkbox(dl, "vis_hit_notif", ImVec2(rt.x + 20, rt.y + yy), "Notifications", &st.hitNotifications);
                        Widgets::ColorPicker(dl, "hit_col", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "##hitc", st.hitEffectsColor); yy += 30.f;
                    }
                    dl->AddLine(ImVec2(rt.x + 20, rt.y + yy + 4), ImVec2(rt.x + pW - 20, rt.y + yy + 4), Theme::Outline(60)); yy += 14.f;
                    dl->AddText(ImVec2(rt.x + 20, rt.y + yy + 4), Theme::MutedText(200), "Shot Tracers");
                    yy += 22.f;
                    Widgets::Checkbox(dl, "vis_shot_tracers", ImVec2(rt.x + 20, rt.y + yy), "Enabled", &st.shotTracers);
                    if (st.shotTracers) { Widgets::ColorPicker(dl, "tracer_col", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "##tracerc", st.shotTracerColor); } yy += 30.f;
                    if (st.shotTracers) {
                        static const char* tracerTypes[] = { "Line", "Beam", "Dots" };
                        Widgets::Combo(dl, "vis_tracer_type", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Type", &st.shotTracerType, tracerTypes, 3); yy += 48.f;
                    }
                    EndPanel(rt, ImVec2(pW, pH), rightScroll[2], yy);
                }
            }
        }

        // ---- TAB: Movement (3) ----
        if (activeTab == 3)
        {
            {
                bool& exp = leftPanelOpen[3];
                Widgets::Panel(dl, "mov_left", lt, ImVec2(pW, pH), "Movement", &exp);
                if (exp) {
                    BeginPanel(lt, ImVec2(pW, pH), leftScroll[3]);
                    float yy = 44.f + leftScroll[3];
                    Widgets::Checkbox(dl, "mov_fly", ImVec2(lt.x + 20, lt.y + yy), "Fly", &st.flyEnabled); yy += 30.f;
                    KeyBindRow(lt, pW, yy, "Fly Key", &st.flyKey, &st.flyMode);
                    if (st.flyEnabled) {
                        static const char* flyTypes[] = { "Velocity", "CFrame", "Anchored", "Multi-Part" };
                        Widgets::Combo(dl, "mov_fly_type", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Type", &st.flyType, flyTypes, 4); yy += 48.f;
                        Widgets::SliderFloat(dl, "mov_fly_spd", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Fly Speed", &st.flySpeed, 10.f, 200.f, "%.0f", &st.vFlySpeed); yy += 38.f;
                        Widgets::Checkbox(dl, "mov_fly_stealth", ImVec2(lt.x + 20, lt.y + yy), "Stealth", &st.flyStealth); yy += 30.f;
                        Widgets::Checkbox(dl, "mov_fly_bypass", ImVec2(lt.x + 20, lt.y + yy), "Bypass Pulse", &st.flyBypass); yy += 30.f;
                        if (st.flyBypass) {
                            Widgets::SliderFloat(dl, "mov_fly_bp_str", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Strength", &st.flyBypassStrength, 1.f, 20.f, "%.0f", &st.vFlyBypassStrength); yy += 38.f;
                            Widgets::SliderFloat(dl, "mov_fly_bp_int", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Interval (s)", &st.flyBypassInterval, 0.5f, 10.f, "%.1f", &st.vFlyBypassInterval); yy += 38.f;
                            Widgets::SliderFloat(dl, "mov_fly_bp_dur", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Duration (ms)", &st.flyBypassDuration, 0.05f, 0.5f, "%.0f", &st.vFlyBypassDuration); yy += 38.f;
                        }
                    }
                    dl->AddLine(ImVec2(lt.x + 20, lt.y + yy + 4), ImVec2(lt.x + pW - 20, lt.y + yy + 4), Theme::Outline(60)); yy += 12.f;
                    Widgets::Checkbox(dl, "mov_speed", ImVec2(lt.x + 20, lt.y + yy), "Speed", &st.speedEnabled); yy += 30.f;
                    KeyBindRow(lt, pW, yy, "Speed Key", &st.speedKey, &st.speedMode);
                    if (st.speedEnabled) {
                        static const char* speedTypes[] = { "Walkspeed", "Velocity", "CFrame" };
                        Widgets::Combo(dl, "mov_speed_type", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Speed Type", &st.speedType, speedTypes, 3); yy += 48.f;
                        Widgets::SliderFloat(dl, "mov_speed_val", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Speed Value", &st.speedVal, 16.f, 200.f, "%.0f", &st.vSpeedVal); yy += 38.f;
                    }
                    dl->AddLine(ImVec2(lt.x + 20, lt.y + yy + 4), ImVec2(lt.x + pW - 20, lt.y + yy + 4), Theme::Outline(60)); yy += 12.f;
                    Widgets::Checkbox(dl, "mov_jump", ImVec2(lt.x + 20, lt.y + yy), "Jump Power", &st.jumpPowerEnabled); yy += 30.f;
                    KeyBindRow(lt, pW, yy, "Jump Power Key", &st.jumpPowerKey, &st.jumpPowerMode);
                    if (st.jumpPowerEnabled) { Widgets::SliderFloat(dl, "mov_jump_val", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Jump Power Value", &st.jumpPowerVal, 0.f, 200.f, "%.0f", &st.vJumpPowerVal); yy += 38.f; }
                    EndPanel(lt, ImVec2(pW, pH), leftScroll[3], yy);
                }
            }

            {
                bool& exp = rightPanelOpen[3];
                Widgets::Panel(dl, "mov_right", rt, ImVec2(pW, pH), "Extras", &exp);
                if (exp) {
                    BeginPanel(rt, ImVec2(pW, pH), rightScroll[3]);
                    float yy = 44.f + rightScroll[3];
                    dl->AddText(ImVec2(rt.x + 20, rt.y + yy + 4), Theme::MutedText(200), "Spinbot"); yy += 22.f;
                    Widgets::Checkbox(dl, "mov_spinbot", ImVec2(rt.x + 20, rt.y + yy), "Enabled", &st.spinbotEnabled); yy += 30.f;
                    if (st.spinbotEnabled) {
                        static const char* spinTypes[] = { "Y-Spin", "Jitter", "Backwards", "Random" };
                        Widgets::Combo(dl, "mov_spin_type", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Type", &st.spinbotType, spinTypes, 4); yy += 48.f;
                        Widgets::SliderFloat(dl, "mov_spin_spd", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Speed", &st.spinbotSpeed, 0.01f, 0.5f, "%.3f", &st.vSpinbotSpeed); yy += 38.f;
                    }
                    dl->AddLine(ImVec2(rt.x + 20, rt.y + yy + 4), ImVec2(rt.x + pW - 20, rt.y + yy + 4), Theme::Outline(60)); yy += 14.f;
                    dl->AddText(ImVec2(rt.x + 20, rt.y + yy + 4), Theme::MutedText(200), "Macro"); yy += 22.f;
                    Widgets::Checkbox(dl, "mov_macro", ImVec2(rt.x + 20, rt.y + yy), "Enabled", &st.macroEnabled); yy += 30.f;
                    KeyBindRow(rt, pW, yy, "Macro Key", &st.macroKey, &st.macroMode);
                    static const char* macroTypes[] = { "IOIOIO", "Mouse Wheel" };
                    Widgets::Combo(dl, "mov_macro_type", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Type", &st.macroType, macroTypes, 2); yy += 48.f;
                    EndPanel(rt, ImVec2(pW, pH), rightScroll[3], yy);
                }
            }
        }

        // ---- TAB: Settings (4) ----
        if (activeTab == 4)
        {
            {
                bool& exp = leftPanelOpen[4];
                Widgets::Panel(dl, "set_panel", lt, ImVec2(pW, pH), "Settings", &exp);
                if (exp) {
                    BeginPanel(lt, ImVec2(pW, pH), leftScroll[4]);
                    float yy = 44.f + leftScroll[4];
                    Widgets::Checkbox(dl, "set_stream", ImVec2(lt.x + 20, lt.y + yy), "Streamproof", &st.streamproof); yy += 30.f;
                    Widgets::Checkbox(dl, "set_hide_console", ImVec2(lt.x + 20, lt.y + yy), "Hide Console", &st.hideConsole); yy += 30.f;
                    Widgets::Checkbox(dl, "set_watermark", ImVec2(lt.x + 20, lt.y + yy), "Watermark", &st.watermark); yy += 30.f;
                    if (st.watermark) {
                        Widgets::Checkbox(dl, "set_wm_players", ImVec2(lt.x + 34, lt.y + yy), "Players", &st.wmShowPlayers); yy += 30.f;
                        Widgets::Checkbox(dl, "set_wm_game", ImVec2(lt.x + 34, lt.y + yy), "Game", &st.wmShowGame); yy += 30.f;
                    }
                    Widgets::Checkbox(dl, "set_explorer", ImVec2(lt.x + 20, lt.y + yy), "Explorer", &st.explorer); yy += 30.f;
                    KeyBindRow(lt, pW, yy, "Menu Toggle Key", &st.menuToggleKey);
                    KeyBindRow(lt, pW, yy, "Panic Key", &st.panicKey);
                    static const char* perfModes[] = { "Low", "Medium", "High" };
                    Widgets::Combo(dl, "set_perf", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Performance Type", &st.perfMode, perfModes, 3); yy += 48.f;
                    EndPanel(lt, ImVec2(pW, pH), leftScroll[4], yy);
                }
            }

            {
                bool& exp = rightPanelOpen[4];
                Widgets::Panel(dl, "appear_panel", rt, ImVec2(pW, pH), "Appearance", &exp);
                if (exp) {
                    BeginPanel(rt, ImVec2(pW, pH), rightScroll[4]);
                    float yy = 44.f + rightScroll[4];
                    ImGui::PopClipRect();
                    Widgets::ColorPicker(dl, "set_accent", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Accent Color", st.accentColor); yy += 34.f;
                    ImGui::PushClipRect(ImVec2(rt.x + 4, rt.y + 38), ImVec2(rt.x + pW - 4, rt.y + pH - 4), false);
                    static const char* fonts[] = { "Tahoma", "Tahoma Bold", "Verdana Pro Bold" };
                    Widgets::Combo(dl, "set_font", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Font", &st.fontIndex, fonts, 3); yy += 40.f;
                    // Background effect controls
                    dl->AddText(ImVec2(rt.x + 20, rt.y + yy + 4), Theme::MutedText(200), "Background");
                    dl->AddLine(ImVec2(rt.x + 20, rt.y + yy + 24), ImVec2(rt.x + pW - 20, rt.y + yy + 24), Theme::Outline(100));
                    yy += 30.f;
                    static const char* effects[] = { "None", "Snow", "Rain", "Network", "Bubbles" };
                    Widgets::Combo(dl, "bg_effect", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Effect", &bgEffect, effects, 5); yy += 48.f;
                    if (bgEffect != 0) {
                        yy += 4.f;
                        Widgets::SliderInt(dl, "bg_count", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Count", &particleCount, 10, 200, "%d", &st.vBgCount); yy += 38.f;
                        Widgets::SliderFloat(dl, "bg_speed", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Speed", &particleSpeed, 0.1f, 5.0f, "%.1f", &st.vBgSpeed); yy += 38.f;

                    }
                    float overlayDark = Menu::Theme::GetOverlayDarkness();
                    Widgets::SliderFloat(dl, "bg_overlay", ImVec2(rt.x + 20, rt.y + yy), pW - 40.f, "Overlay Darkness", &overlayDark, 0.0f, 1.0f, "%.2f", &st.vOverlayDarkness); yy += 32.f;
                    Menu::Theme::SetOverlayDarkness(overlayDark);
                    EndPanel(rt, ImVec2(pW, pH), rightScroll[4], yy);
                }
            }
        }

        // ---- TAB: Configs (5) ----
        if (activeTab == 5)
        {
            static char cfgName[64] = "";
            static char msg[128] = "";
            static float msgTimer = 0.f;

            auto SetMsg = [&](const char* text) {
                strncpy_s(msg, sizeof(msg), text, _TRUNCATE);
                msgTimer = 3.f;
            };

            {
                bool& exp = leftPanelOpen[5];
                Widgets::Panel(dl, "cfg_manage", lt, ImVec2(pW, pH), "Manage", &exp);
                if (exp) {
                    BeginPanel(lt, ImVec2(pW, pH), leftScroll[5]);
                    float yy = 44.f + leftScroll[5];

                    Widgets::Input(dl, "cfg_input", ImVec2(lt.x + 20, lt.y + yy), pW - 40.f, "Config Name", cfgName, sizeof(cfgName));
                    yy += 56.f;

                    float btnW = (pW - 52.f) * 0.5f;
                    float fullBtnW = pW - 44.f;
                    ImVec2 halfBtn(btnW, 28.f);
                    ImVec2 fullBtn(fullBtnW, 28.f);

                    if (Widgets::MenuButton(dl, "cfg_save", ImVec2(lt.x + 20, lt.y + yy), halfBtn, "Save") && cfgName[0]) {
                        if (Config::Save(cfgName)) { char b[64]; snprintf(b, sizeof(b), "Saved \"%s\"", cfgName); SetMsg(b); }
                        else { char b[80]; snprintf(b, sizeof(b), "Failed to save \"%s\"", cfgName); SetMsg(b); }
                    }
                    if (Widgets::MenuButton(dl, "cfg_load", ImVec2(lt.x + 26 + btnW, lt.y + yy), halfBtn, "Load") && cfgName[0]) {
                        if (Config::Load(cfgName)) { char b[64]; snprintf(b, sizeof(b), "Loaded \"%s\"", cfgName); SetMsg(b); SyncGlobalsToSt(); }
                        else { char b[80]; snprintf(b, sizeof(b), "Failed to load \"%s\"", cfgName); SetMsg(b); }
                    }
                    yy += 36.f;

                    if (Widgets::MenuButton(dl, "cfg_del", ImVec2(lt.x + 20, lt.y + yy), fullBtn, "Delete", IM_COL32(180, 60, 60, 215)) && cfgName[0]) {
                        if (Config::Delete(cfgName)) { char b[64]; snprintf(b, sizeof(b), "Deleted \"%s\"", cfgName); SetMsg(b); }
                        else { char b[80]; snprintf(b, sizeof(b), "Failed to delete \"%s\"", cfgName); SetMsg(b); }
                    }
                    yy += 36.f;

                    if (msg[0] && msgTimer > 0.f) {
                        msgTimer -= dt;
                        int ma = static_cast<int>(255 * (msgTimer > 2.5f ? 1.f : msgTimer / 2.5f));
                        dl->AddText(ImVec2(lt.x + 20, lt.y + yy), IM_COL32(200, 200, 200, ma), msg);
                        yy += 24.f;
                    } else {
                        msg[0] = '\0';
                    }

                    EndPanel(lt, ImVec2(pW, pH), leftScroll[5], yy);
                }
            }

            {
                bool& exp = rightPanelOpen[5];
                Widgets::Panel(dl, "cfg_list", rt, ImVec2(pW, pH), "Configs", &exp);
                if (exp) {
                    BeginPanel(rt, ImVec2(pW, pH), rightScroll[5]);
                    float yy = 44.f + rightScroll[5];

                    if (Widgets::MenuButton(dl, "cfg_reset", ImVec2(rt.x + 20, rt.y + yy), ImVec2(pW - 44.f, 28.f), "Reset Defaults")) {
                        Config::ResetDefaults();
                        SyncGlobalsToSt();
                        SetMsg("Defaults restored");
                    }
                    yy += 38.f;

                    static std::vector<Config::ConfigInfo> cfgList;
                    static float listRefreshTimer = 0.f;
                    listRefreshTimer -= dt;
                    if (listRefreshTimer <= 0.f) {
                        cfgList = Config::ListConfigs();
                        listRefreshTimer = 2.f;
                    }

                    auto FormatTime = [](int64_t ts, char* buf, size_t len) {
                        if (ts <= 0) { strncpy_s(buf, len, "unknown", _TRUNCATE); return; }
                        time_t t = static_cast<time_t>(ts);
                        struct tm lt;
                        if (localtime_s(&lt, &t) != 0) { strncpy_s(buf, len, "unknown", _TRUNCATE); return; }
                        strftime(buf, len, "%b %d %Y  %I:%M %p", &lt);
                    };

                    dl->AddText(ImVec2(rt.x + 20, rt.y + yy + 4), Theme::MutedText(200), "Saved Configs");
                    yy += 24.f;

                    for (size_t i = 0; i < cfgList.size(); ++i) {
                        float iy = rt.y + yy;
                        float itemH = 48.f;
                        float itemX = rt.x + 16;
                        float itemW = pW - 32.f;
                        bool hovered = ImGui::IsMouseHoveringRect(
                            ImVec2(itemX, iy), ImVec2(itemX + itemW, iy + itemH));
                        bool selected = cfgName[0] && strcmp(cfgList[i].name.c_str(), cfgName) == 0;

                        ImU32 boxBg = selected ? Theme::Accent(50) : (hovered ? Theme::SurfaceLight(180) : Theme::Surface(120));
                        ImU32 boxBorder = selected ? Theme::Accent(180) : (hovered ? Theme::Accent(80) : Theme::Outline(60));
                        dl->AddRectFilled(ImVec2(itemX, iy), ImVec2(itemX + itemW, iy + itemH), boxBg, 5);
                        dl->AddRect(ImVec2(itemX, iy), ImVec2(itemX + itemW, iy + itemH), boxBorder, 5);

                        ImU32 nameCol = selected ? Theme::Accent() : Theme::Text(hovered ? 255 : 200);
                        dl->AddText(ImVec2(itemX + 10, iy + 6), nameCol, cfgList[i].name.c_str());

                        char timeBuf[64];
                        FormatTime(cfgList[i].created_at, timeBuf, sizeof(timeBuf));
                        dl->AddText(ImVec2(itemX + 10, iy + 26), Theme::MutedText(150), timeBuf);

                        ImGui::SetCursorScreenPos(ImVec2(itemX, iy));
                        ImGui::InvisibleButton(("##cfg_" + cfgList[i].name).c_str(), ImVec2(itemW, itemH));
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                            strncpy_s(cfgName, sizeof(cfgName), cfgList[i].name.c_str(), _TRUNCATE);
                        }
                        yy += itemH + 4.f;
                    }

                    EndPanel(rt, ImVec2(pW, pH), rightScroll[5], yy);
                }
            }
        }

        style.Alpha = savedAlpha;
        Theme::SetMenuAlpha(1.0f);

        ImGui::End();

        // ── Write st back to Globals ──
        SyncStToGlobals();
    }
}
