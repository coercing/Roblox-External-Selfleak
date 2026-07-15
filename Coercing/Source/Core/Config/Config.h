#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "crypt32.lib")

#include <Globals.hxx>
#include <imgui/addons/imgui_addons.h>

namespace Config {

static const uint32_t MAGIC   = 0x464E4758; // "FNGX"
static const uint32_t VERSION = 2;

#pragma pack(push, 1)
struct Data {
    uint32_t magic;
    uint32_t version;
    int64_t  created_at;   // seconds since epoch

    // Settings
    bool     Team_Check;
    bool     Client_Check;
    bool     FriendCheck;
    bool     Streamproof;
    bool     Hide_Console;
    int      Performance_Mode;
    int      Menu_Toggle_Key;
    int      Exit_Key;
    float    AccentColor[4];
    int      FontIndex;

    // Aimbot
    bool     Aim_useFov;
    bool     Aim_Enabled;
    bool     Aim_DrawFov;
    bool     Aim_FovSpin;
    bool     Aim_FillFov;
    bool     Aim_AimbotSticky;
    bool     Aim_Shake;
    bool     Aim_KnockedCheck;
    bool     Aim_VisibilityCheck;
    bool     Aim_Prediction;
    float    Aim_ShakeX;
    float    Aim_ShakeY;
    float    Aim_ShakeZ;
    float    Aim_Prediction_X;
    float    Aim_Prediction_Y;
    float    Aim_FovSize;
    int      Aim_HitPart;
    int      Aim_type;
    float    Aim_FovColor[4];
    int      Aim_FovSpinSpeed;
    int      Aim_Key;
    int      Aim_Mode;
    float    Aim_CamSmooth_X;
    float    Aim_CamSmooth_Y;
    float    Aim_MouseSmooth_X;
    float    Aim_MouseSmooth_Y;
    float    Aim_MouseSens;

    // Silent
    bool     Sil_DrawFov;
    bool     Sil_Enabled;
    bool     Sil_StickyAim;
    bool     Sil_SpoofMouse;
    bool     Sil_UseFov;
    bool     Sil_KnockedCheck;
    bool     Sil_GunBasedFov;
    bool     Sil_VisibilityCheck;
    float    Sil_Fov;
    float    Sil_FovDoubleBarrel;
    float    Sil_FovTacticalShotgun;
    float    Sil_FovRevolver;
    int      Sil_Key;
    int      Sil_Mode;
    int      Sil_AimPart;
    float    Sil_FovColor[4];
    int      Sil_FovSpinSpeed;
    bool     Sil_FovSpin;
    bool     Sil_FillFov;

    // Visuals
    bool     Vis_Enabled;
    bool     Vis_Box;
    bool     Vis_Box_Fill;
    bool     Vis_Box_Fill_Gradient;
    bool     Vis_Box_Fill_Gradient_Rotate;
    bool     Vis_Healthbar;
    bool     Vis_Health;
    bool     Vis_Name;
    bool     Vis_Distance;
    bool     Vis_Rig_Type;
    bool     Vis_Tool;
    bool     Vis_Skeleton;
    bool     Vis_Chams;
    float    Vis_Col_Chams[4];
    float    Vis_Render_Distance;
    int      Vis_BoxFillSpeed;
    int      Vis_Healthbar_Type;
    int      Vis_Box_Type;
    int      Vis_Box_Fill_Type;
    int      Vis_Name_Type;
    int      Vis_Gap;
    int      Vis_Thickness;
    float    Vis_Col_Box[4];
    float    Vis_Col_BoxOutline[4];
    float    Vis_Col_BoxFill_Top[4];
    float    Vis_Col_BoxFill_Bottom[4];
    float    Vis_Col_Name[4];
    float    Vis_Col_Distance[4];
    float    Vis_Col_RigType[4];
    float    Vis_Col_Tool[4];
    float    Vis_Col_Health[4];
    float    Vis_Col_Healthbar[4];
    float    Vis_Col_Healthbar_Top[4];
    float    Vis_Col_Healthbar_Mid[4];
    float    Vis_Col_Healthbar_Bot[4];
    float    Vis_Col_Skeleton[4];
    float    Vis_Col_Friend[4];
    float    Vis_Col_Visible[4];
    float    Vis_Col_Invisible[4];
    float    Vis_Col_Snapline[4];
    float    Vis_Col_HeadDot[4];
    float    Vis_Col_FOVCircle[4];

    // World
    bool     Wld_Fog;
    bool     Wld_Exposure;
    bool     Wld_FOV;
    float    Wld_Fog_Distance;
    float    Wld_FOV_Distance;
    float    Wld_ExposureI;
    float    Wld_Col_Fog[4];

    // Misc
    bool     Misc_Fly;
    float    Misc_Fly_Speed;
    int      Misc_Fly_Key;
    int      Misc_Fly_Mode;
    int      Misc_Fly_Type;
    bool     Misc_Fly_Stealth;
    bool     Misc_Fly_BypassPulse;
    float    Misc_Fly_BypassStrength;
    float    Misc_Fly_BypassInterval;
    float    Misc_Fly_BypassDuration;
    bool     Misc_Explorer;

    // Movement
    bool     Mov_Speed;
    float    Mov_Speed_Value;
    int      Mov_Speed_Type;
    int      Mov_Speed_Key;
    int      Mov_Speed_Mode;
    bool     Mov_JumpPower;
    float    Mov_JumpPower_Value;
    int      Mov_JumpPower_Key;
    int      Mov_JumpPower_Mode;
    bool     Mov_Macro;
    int      Mov_Macro_Type;
    int      Mov_Macro_Key;
    int      Mov_Macro_Mode;
    bool     Mov_Spinbot;
    int      Mov_Spinbot_Type;
    float    Mov_Spinbot_Speed;

    // Radar
    bool     Rad_Enabled;
    float    Rad_Size;
    float    Rad_Range;
    float    Rad_Opacity;
    float    Rad_PosX;
    float    Rad_PosY;
    int      Rad_EnemyFlags;

    // ESP Upgrades
    bool     Vis_LookDir;
    float    Vis_LookDirColor[4];

    // Visuals upgrades (snapline, head dot)
    bool     Vis_Snapline;
    int      Vis_SnaplinePosition;
    bool     Vis_HeadDot;
    float    Vis_HeadDotSize;

    // Watermark
    bool     Wm_Enabled;
    bool     Wm_ShowPlayers;
    bool     Wm_ShowGame;

    // Hit effects + shot tracers
    int      Vis_HitEffectsType;
    float    Vis_HitEffectsDuration;
    float    Vis_HitEffectsColor[4];
    bool     Vis_HitNotifications;
    bool     Vis_ShotTracers;
    int      Vis_ShotTracerType;
    float    Vis_ShotTracerColor[4];

    uint32_t checksum;
};
#pragma pack(pop)

inline uint32_t CalcChecksum(const Data& d)
{
    const uint8_t* p   = reinterpret_cast<const uint8_t*>(&d);
    const size_t   len = offsetof(Data, checksum);
    uint32_t       cs  = 0;
    for (size_t i = 0; i < len; ++i) cs ^= ((uint32_t)p[i] << ((i & 3) * 8));
    return cs;
}

inline std::string ToBase64(const void* data, size_t len)
{
    DWORD outLen = 0;
    CryptBinaryToStringA(reinterpret_cast<const BYTE*>(data), (DWORD)len,
        CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &outLen);
    std::string out(outLen, '\0');
    CryptBinaryToStringA(reinterpret_cast<const BYTE*>(data), (DWORD)len,
        CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, out.data(), &outLen);
    while (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
}

inline bool FromBase64(const std::string& b64, std::vector<uint8_t>& out)
{
    DWORD outLen = 0;
    if (!CryptStringToBinaryA(b64.c_str(), (DWORD)b64.size(),
        CRYPT_STRING_BASE64_ANY, nullptr, &outLen, nullptr, nullptr)) return false;
    out.resize(outLen);
    return CryptStringToBinaryA(b64.c_str(), (DWORD)b64.size(),
        CRYPT_STRING_BASE64_ANY, out.data(), &outLen, nullptr, nullptr) != 0;
}

inline std::string GetDir()
{
    static std::string dir;
    if (dir.empty()) {
        char exePath[MAX_PATH];
        GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        std::string p = exePath;
        auto pos = p.find_last_of("\\/");
        if (pos != std::string::npos) p = p.substr(0, pos);
        dir = p + "\\Configs";
    }
    return dir;
}

inline Data Capture()
{
    Data d{};
    d.magic   = MAGIC;
    d.version = VERSION;

    using namespace Globals;

    d.Team_Check          = Settings::Team_Check;
    d.Client_Check        = Settings::Client_Check;
    d.FriendCheck         = Settings::FriendCheck;
    d.Streamproof         = Settings::Streamproof;
    d.Hide_Console        = Settings::Hide_Console;
    d.Performance_Mode    = Settings::Performance_Mode;
    d.Menu_Toggle_Key     = ImGuiKeyToVK(Settings::Menu_Toggle_Key);
    d.Exit_Key            = ImGuiKeyToVK(Settings::Exit_Key);
    memcpy(d.AccentColor, Settings::AccentColor, sizeof(d.AccentColor));
    d.FontIndex           = Settings::FontIndex;

    d.Aim_useFov          = Aimbot::useFov;
    d.Aim_Enabled         = Aimbot::Enabled;
    d.Aim_DrawFov         = Aimbot::DrawFov;
    d.Aim_FovSpin         = Aimbot::FovSpin;
    d.Aim_FillFov         = Aimbot::FillFov;
    d.Aim_AimbotSticky    = Aimbot::AimbotSticky;
    d.Aim_Shake           = Aimbot::Shake;
    d.Aim_KnockedCheck    = Aimbot::KnockedCheck;
    d.Aim_VisibilityCheck = Aimbot::VisibilityCheck;
    d.Aim_Prediction      = Aimbot::Prediction;
    d.Aim_ShakeX          = Aimbot::ShakeX;
    d.Aim_ShakeY          = Aimbot::ShakeY;
    d.Aim_ShakeZ          = Aimbot::ShakeZ;
    d.Aim_Prediction_X    = Aimbot::Prediction_X;
    d.Aim_Prediction_Y    = Aimbot::Prediction_Y;
    d.Aim_FovSize         = Aimbot::FovSize;
    d.Aim_HitPart         = Aimbot::HitPart;
    d.Aim_type            = Aimbot::Aimbot_type;
    memcpy(d.Aim_FovColor, Aimbot::FovColor, sizeof(d.Aim_FovColor));
    d.Aim_FovSpinSpeed    = Aimbot::FovSpinSpeed;
    d.Aim_Key             = ImGuiKeyToVK(Aimbot::Aimbot_Key);
    d.Aim_Mode            = (int)Aimbot::Aimbot_Mode;
    d.Aim_CamSmooth_X     = Aimbot::Camera::Smoothing_X;
    d.Aim_CamSmooth_Y     = Aimbot::Camera::Smoothing_Y;
    d.Aim_MouseSmooth_X   = Aimbot::Mouse::Smoothing_X;
    d.Aim_MouseSmooth_Y   = Aimbot::Mouse::Smoothing_Y;
    d.Aim_MouseSens       = Aimbot::Mouse::Mouse_Sensitivty;

    d.Sil_DrawFov         = Silent::DrawFov;
    d.Sil_Enabled         = Silent::Enabled;
    d.Sil_StickyAim       = Silent::StickyAim;
    d.Sil_SpoofMouse      = Silent::SpoofMouse;
    d.Sil_UseFov          = Silent::UseFov;
    d.Sil_KnockedCheck    = Silent::KnockedCheck;
    d.Sil_GunBasedFov     = Silent::GunBasedFov;
    d.Sil_VisibilityCheck = Silent::VisibilityCheck;
    d.Sil_Fov             = Silent::Fov;
    d.Sil_FovDoubleBarrel      = Silent::FovDoubleBarrel;
    d.Sil_FovTacticalShotgun   = Silent::FovTacticalShotgun;
    d.Sil_FovRevolver     = Silent::FovRevolver;
    d.Sil_Key             = ImGuiKeyToVK(Silent::Silent_Key);
    d.Sil_Mode            = (int)Silent::Silent_Mode;
    d.Sil_AimPart         = Silent::AimPart;
    memcpy(d.Sil_FovColor, Silent::FovColor, sizeof(d.Sil_FovColor));
    d.Sil_FovSpinSpeed    = Silent::FovSpinSpeed;
    d.Sil_FovSpin         = Silent::FovSpin;
    d.Sil_FillFov         = Silent::FillFov;

    d.Vis_Enabled               = Visuals::Enabled;
    d.Vis_Box                   = Visuals::Box;
    d.Vis_Box_Fill              = Visuals::Box_Fill;
    d.Vis_Box_Fill_Gradient     = Visuals::Box_Fill_Gradient;
    d.Vis_Box_Fill_Gradient_Rotate = Visuals::Box_Fill_Gradient_Rotate;
    d.Vis_Healthbar             = Visuals::Healthbar;
    d.Vis_Health                = Visuals::Health;
    d.Vis_Name                  = Visuals::Name;
    d.Vis_Distance              = Visuals::Distance;
    d.Vis_Rig_Type              = Visuals::Rig_Type;
    d.Vis_Tool                  = Visuals::Tool;
    d.Vis_Skeleton              = Visuals::Skeleton;
    d.Vis_Chams                 = Visuals::Chams;
    memcpy(d.Vis_Col_Chams,             Visuals::ChamsColor,               sizeof(d.Vis_Col_Chams));
    d.Vis_Render_Distance       = Visuals::Render_Distance;
    d.Vis_BoxFillSpeed          = Visuals::BoxFillSpeed;
    d.Vis_Healthbar_Type        = Visuals::Healthbar_Type;
    d.Vis_Box_Type              = Visuals::Box_Type;
    d.Vis_Box_Fill_Type         = Visuals::Box_Fill_Type;
    d.Vis_Name_Type             = Visuals::Name_Type;
    d.Vis_Gap                   = Visuals::Gap;
    d.Vis_Thickness             = Visuals::Thickness;
    memcpy(d.Vis_Col_Box,            Visuals::Colors::Box,               sizeof(d.Vis_Col_Box));
    memcpy(d.Vis_Col_BoxOutline,     Visuals::Colors::BoxOutline,        sizeof(d.Vis_Col_BoxOutline));
    memcpy(d.Vis_Col_BoxFill_Top,    Visuals::Colors::BoxFill_Top,       sizeof(d.Vis_Col_BoxFill_Top));
    memcpy(d.Vis_Col_BoxFill_Bottom, Visuals::Colors::BoxFill_Bottom,    sizeof(d.Vis_Col_BoxFill_Bottom));
    memcpy(d.Vis_Col_Name,           Visuals::Colors::Name,              sizeof(d.Vis_Col_Name));
    memcpy(d.Vis_Col_Distance,       Visuals::Colors::Distance,          sizeof(d.Vis_Col_Distance));
    memcpy(d.Vis_Col_RigType,        Visuals::Colors::Rig_Type,          sizeof(d.Vis_Col_RigType));
    memcpy(d.Vis_Col_Tool,           Visuals::Colors::Tool,              sizeof(d.Vis_Col_Tool));
    memcpy(d.Vis_Col_Health,         Visuals::Colors::Health,            sizeof(d.Vis_Col_Health));
    memcpy(d.Vis_Col_Healthbar,      Visuals::Colors::Healthbar,         sizeof(d.Vis_Col_Healthbar));
    memcpy(d.Vis_Col_Healthbar_Top,  Visuals::Colors::Healthbar_Top,     sizeof(d.Vis_Col_Healthbar_Top));
    memcpy(d.Vis_Col_Healthbar_Mid,  Visuals::Colors::Healthbar_Middle,  sizeof(d.Vis_Col_Healthbar_Mid));
    memcpy(d.Vis_Col_Healthbar_Bot,  Visuals::Colors::Healthbar_Bottom,  sizeof(d.Vis_Col_Healthbar_Bot));
    memcpy(d.Vis_Col_Skeleton,       Visuals::Colors::Skeleton,          sizeof(d.Vis_Col_Skeleton));
    memcpy(d.Vis_Col_Friend,         Visuals::Colors::Friend,            sizeof(d.Vis_Col_Friend));
    memcpy(d.Vis_Col_Visible,        Visuals::Colors::Visible,           sizeof(d.Vis_Col_Visible));
    memcpy(d.Vis_Col_Invisible,      Visuals::Colors::Invisible,         sizeof(d.Vis_Col_Invisible));
    memcpy(d.Vis_Col_Snapline,       Visuals::Colors::Snapline,          sizeof(d.Vis_Col_Snapline));
    memcpy(d.Vis_Col_HeadDot,        Visuals::Colors::HeadDot,           sizeof(d.Vis_Col_HeadDot));
    memcpy(d.Vis_Col_FOVCircle,      Visuals::Colors::FOVCircle,         sizeof(d.Vis_Col_FOVCircle));

    d.Wld_Fog                 = World::Fog;
    d.Wld_Exposure            = World::Exposure;
    d.Wld_FOV                 = World::FOV;
    d.Wld_Fog_Distance        = World::Fog_Distance;
    d.Wld_FOV_Distance        = World::FOV_Distance;
    d.Wld_ExposureI           = World::ExposureI;
    memcpy(d.Wld_Col_Fog,      World::Colors::Fog,      sizeof(d.Wld_Col_Fog));

    d.Misc_Fly                = Misc::Fly;
    d.Misc_Fly_Speed          = Misc::Fly_Speed;
    d.Misc_Fly_Key            = ImGuiKeyToVK(Misc::Fly_Key);
    d.Misc_Fly_Mode           = (int)Misc::Fly_Mode;
    d.Misc_Fly_Type           = Misc::Fly_Type;
    d.Misc_Fly_Stealth        = Misc::Fly_Stealth;
    d.Misc_Fly_BypassPulse    = Misc::Fly_BypassPulse;
    d.Misc_Fly_BypassStrength = Misc::Fly_BypassStrength;
    d.Misc_Fly_BypassInterval = Misc::Fly_BypassInterval;
    d.Misc_Fly_BypassDuration = Misc::Fly_BypassDuration;
    d.Misc_Explorer           = Misc::Explorer;

    d.Mov_Speed           = Movement::Speed;
    d.Mov_Speed_Value     = Movement::Speed_Value;
    d.Mov_Speed_Type      = Movement::Speed_Type;
    d.Mov_Speed_Key       = ImGuiKeyToVK(Movement::Speed_Key);
    d.Mov_Speed_Mode      = (int)Movement::Speed_Mode;
    d.Mov_JumpPower       = Movement::JumpPower;
    d.Mov_JumpPower_Value = Movement::JumpPower_Value;
    d.Mov_JumpPower_Key   = ImGuiKeyToVK(Movement::JumpPower_Key);
    d.Mov_JumpPower_Mode  = (int)Movement::JumpPower_Mode;
    d.Mov_Macro           = Movement::Macro;
    d.Mov_Macro_Type      = Movement::Macro_Type;
    d.Mov_Macro_Key       = ImGuiKeyToVK(Movement::Macro_Key);
    d.Mov_Macro_Mode      = (int)Movement::Macro_Mode;
    d.Mov_Spinbot         = Movement::Spinbot;
    d.Mov_Spinbot_Type    = Movement::Spinbot_Type;
    d.Mov_Spinbot_Speed   = Movement::Spinbot_Speed;

    d.Rad_Enabled         = Radar::Enabled;
    d.Rad_Size            = Radar::Size;
    d.Rad_Range           = Radar::Range;
    d.Rad_Opacity         = Radar::Opacity;
    d.Rad_PosX            = Radar::PosX;
    d.Rad_PosY            = Radar::PosY;
    d.Rad_EnemyFlags      = Radar::EnemyFlags;

    d.Vis_LookDir         = Visuals::LookDirection;
    memcpy(d.Vis_LookDirColor, Visuals::LookDirColor, sizeof(d.Vis_LookDirColor));

    d.Vis_Snapline        = Visuals::Snapline;
    d.Vis_SnaplinePosition = Visuals::SnaplinePosition;
    d.Vis_HeadDot         = Visuals::HeadDot;
    d.Vis_HeadDotSize     = Visuals::HeadDotSize;

    d.Wm_Enabled                = Watermark::Enabled;
    d.Wm_ShowPlayers            = Watermark::ShowPlayers;
    d.Wm_ShowGame               = Watermark::ShowGame;

    d.Vis_HitEffectsType        = Visuals::HitEffectsType;
    d.Vis_HitEffectsDuration    = Visuals::HitEffectsDuration;
    std::copy_n(Visuals::HitEffectsColor, 4, d.Vis_HitEffectsColor);
    d.Vis_HitNotifications      = Visuals::HitNotifications;
    d.Vis_ShotTracers           = Visuals::ShotTracers;
    d.Vis_ShotTracerType        = Visuals::ShotTracerType;
    std::copy_n(Visuals::ShotTracerColor, 4, d.Vis_ShotTracerColor);

    d.checksum = CalcChecksum(d);
    return d;
}

inline bool Apply(const Data& d)
{
    if (d.magic != MAGIC || d.version != VERSION) return false;
    if (d.checksum != CalcChecksum(d))            return false;

    using namespace Globals;

    Settings::Team_Check         = d.Team_Check;
    Settings::Client_Check       = d.Client_Check;
    Settings::FriendCheck        = d.FriendCheck;
    Settings::Streamproof        = d.Streamproof;
    Settings::Hide_Console       = d.Hide_Console;
    Settings::Performance_Mode   = d.Performance_Mode;
    Settings::Menu_Toggle_Key    = VKToImGuiKey(d.Menu_Toggle_Key);
    Settings::Exit_Key           = VKToImGuiKey(d.Exit_Key);
    memcpy(Settings::AccentColor, d.AccentColor, sizeof(Settings::AccentColor));
    Settings::FontIndex          = d.FontIndex;

    Aimbot::useFov          = d.Aim_useFov;
    Aimbot::Enabled         = d.Aim_Enabled;
    Aimbot::DrawFov         = d.Aim_DrawFov;
    Aimbot::FovSpin         = d.Aim_FovSpin;
    Aimbot::FillFov         = d.Aim_FillFov;
    Aimbot::AimbotSticky    = d.Aim_AimbotSticky;
    Aimbot::Shake           = d.Aim_Shake;
    Aimbot::KnockedCheck    = d.Aim_KnockedCheck;
    Aimbot::VisibilityCheck = d.Aim_VisibilityCheck;
    Aimbot::Prediction      = d.Aim_Prediction;
    Aimbot::ShakeX          = d.Aim_ShakeX;
    Aimbot::ShakeY          = d.Aim_ShakeY;
    Aimbot::ShakeZ          = d.Aim_ShakeZ;
    Aimbot::Prediction_X    = d.Aim_Prediction_X;
    Aimbot::Prediction_Y    = d.Aim_Prediction_Y;
    Aimbot::FovSize         = d.Aim_FovSize;
    Aimbot::HitPart         = d.Aim_HitPart;
    Aimbot::Aimbot_type     = d.Aim_type;
    memcpy(Aimbot::FovColor, d.Aim_FovColor, sizeof(Aimbot::FovColor));
    Aimbot::FovSpinSpeed    = d.Aim_FovSpinSpeed;
    Aimbot::Aimbot_Key      = VKToImGuiKey(d.Aim_Key);
    Aimbot::Aimbot_Mode     = (ImKeyBindMode)d.Aim_Mode;
    Aimbot::Camera::Smoothing_X     = d.Aim_CamSmooth_X;
    Aimbot::Camera::Smoothing_Y     = d.Aim_CamSmooth_Y;
    Aimbot::Mouse::Smoothing_X      = d.Aim_MouseSmooth_X;
    Aimbot::Mouse::Smoothing_Y      = d.Aim_MouseSmooth_Y;
    Aimbot::Mouse::Mouse_Sensitivty = d.Aim_MouseSens;

    Silent::DrawFov         = d.Sil_DrawFov;
    Silent::Enabled         = d.Sil_Enabled;
    Silent::StickyAim       = d.Sil_StickyAim;
    Silent::SpoofMouse      = d.Sil_SpoofMouse;
    Silent::UseFov          = d.Sil_UseFov;
    Silent::KnockedCheck    = d.Sil_KnockedCheck;
    Silent::GunBasedFov     = d.Sil_GunBasedFov;
    Silent::VisibilityCheck = d.Sil_VisibilityCheck;
    Silent::Fov             = d.Sil_Fov;
    Silent::FovDoubleBarrel    = d.Sil_FovDoubleBarrel;
    Silent::FovTacticalShotgun = d.Sil_FovTacticalShotgun;
    Silent::FovRevolver     = d.Sil_FovRevolver;
    Silent::Silent_Key      = VKToImGuiKey(d.Sil_Key);
    Silent::Silent_Mode     = (ImKeyBindMode)d.Sil_Mode;
    Silent::AimPart         = d.Sil_AimPart;
    memcpy(Silent::FovColor, d.Sil_FovColor, sizeof(Silent::FovColor));
    Silent::FovSpinSpeed    = d.Sil_FovSpinSpeed;
    Silent::FovSpin         = d.Sil_FovSpin;
    Silent::FillFov         = d.Sil_FillFov;

    Visuals::Enabled               = d.Vis_Enabled;
    Visuals::Box                   = d.Vis_Box;
    Visuals::Box_Fill              = d.Vis_Box_Fill;
    Visuals::Box_Fill_Gradient     = d.Vis_Box_Fill_Gradient;
    Visuals::Box_Fill_Gradient_Rotate = d.Vis_Box_Fill_Gradient_Rotate;
    Visuals::Healthbar             = d.Vis_Healthbar;
    Visuals::Health                = d.Vis_Health;
    Visuals::Name                  = d.Vis_Name;
    Visuals::Distance              = d.Vis_Distance;
    Visuals::Rig_Type              = d.Vis_Rig_Type;
    Visuals::Tool                  = d.Vis_Tool;
    Visuals::Skeleton              = d.Vis_Skeleton;
    Visuals::Chams                 = d.Vis_Chams;
    memcpy(Visuals::ChamsColor,            d.Vis_Col_Chams,              sizeof(Visuals::ChamsColor));
    Visuals::Render_Distance       = d.Vis_Render_Distance;
    Visuals::BoxFillSpeed          = d.Vis_BoxFillSpeed;
    Visuals::Healthbar_Type        = d.Vis_Healthbar_Type;
    Visuals::Box_Type              = d.Vis_Box_Type;
    Visuals::Box_Fill_Type         = d.Vis_Box_Fill_Type;
    Visuals::Name_Type             = d.Vis_Name_Type;
    Visuals::Gap                   = d.Vis_Gap;
    Visuals::Thickness             = d.Vis_Thickness;
    memcpy(Visuals::Colors::Box,              d.Vis_Col_Box,            sizeof(Visuals::Colors::Box));
    memcpy(Visuals::Colors::BoxOutline,       d.Vis_Col_BoxOutline,     sizeof(Visuals::Colors::BoxOutline));
    memcpy(Visuals::Colors::BoxFill_Top,      d.Vis_Col_BoxFill_Top,    sizeof(Visuals::Colors::BoxFill_Top));
    memcpy(Visuals::Colors::BoxFill_Bottom,   d.Vis_Col_BoxFill_Bottom, sizeof(Visuals::Colors::BoxFill_Bottom));
    memcpy(Visuals::Colors::Name,             d.Vis_Col_Name,           sizeof(Visuals::Colors::Name));
    memcpy(Visuals::Colors::Distance,         d.Vis_Col_Distance,       sizeof(Visuals::Colors::Distance));
    memcpy(Visuals::Colors::Rig_Type,         d.Vis_Col_RigType,        sizeof(Visuals::Colors::Rig_Type));
    memcpy(Visuals::Colors::Tool,             d.Vis_Col_Tool,           sizeof(Visuals::Colors::Tool));
    memcpy(Visuals::Colors::Health,           d.Vis_Col_Health,         sizeof(Visuals::Colors::Health));
    memcpy(Visuals::Colors::Healthbar,        d.Vis_Col_Healthbar,      sizeof(Visuals::Colors::Healthbar));
    memcpy(Visuals::Colors::Healthbar_Top,    d.Vis_Col_Healthbar_Top,  sizeof(Visuals::Colors::Healthbar_Top));
    memcpy(Visuals::Colors::Healthbar_Middle, d.Vis_Col_Healthbar_Mid,  sizeof(Visuals::Colors::Healthbar_Middle));
    memcpy(Visuals::Colors::Healthbar_Bottom, d.Vis_Col_Healthbar_Bot,  sizeof(Visuals::Colors::Healthbar_Bottom));
    memcpy(Visuals::Colors::Skeleton,         d.Vis_Col_Skeleton,       sizeof(Visuals::Colors::Skeleton));
    memcpy(Visuals::Colors::Friend,           d.Vis_Col_Friend,         sizeof(Visuals::Colors::Friend));
    memcpy(Visuals::Colors::Visible,          d.Vis_Col_Visible,        sizeof(Visuals::Colors::Visible));
    memcpy(Visuals::Colors::Invisible,        d.Vis_Col_Invisible,      sizeof(Visuals::Colors::Invisible));
    memcpy(Visuals::Colors::Snapline,         d.Vis_Col_Snapline,       sizeof(Visuals::Colors::Snapline));
    memcpy(Visuals::Colors::HeadDot,          d.Vis_Col_HeadDot,        sizeof(Visuals::Colors::HeadDot));
    memcpy(Visuals::Colors::FOVCircle,        d.Vis_Col_FOVCircle,      sizeof(Visuals::Colors::FOVCircle));

    World::Fog                 = d.Wld_Fog;
    World::Exposure            = d.Wld_Exposure;
    World::FOV                 = d.Wld_FOV;
    World::Fog_Distance        = d.Wld_Fog_Distance;
    World::FOV_Distance        = d.Wld_FOV_Distance;
    World::ExposureI           = d.Wld_ExposureI;
    memcpy(World::Colors::Fog,      d.Wld_Col_Fog,      sizeof(World::Colors::Fog));

    Misc::Fly                = d.Misc_Fly;
    Misc::Fly_Speed          = d.Misc_Fly_Speed;
    Misc::Fly_Key            = VKToImGuiKey(d.Misc_Fly_Key);
    Misc::Fly_Mode           = (ImKeyBindMode)d.Misc_Fly_Mode;
    Misc::Fly_Type           = d.Misc_Fly_Type;
    Misc::Fly_Stealth        = d.Misc_Fly_Stealth;
    Misc::Fly_BypassPulse    = d.Misc_Fly_BypassPulse;
    Misc::Fly_BypassStrength = d.Misc_Fly_BypassStrength;
    Misc::Fly_BypassInterval = d.Misc_Fly_BypassInterval;
    Misc::Fly_BypassDuration = d.Misc_Fly_BypassDuration;
    Misc::Explorer           = d.Misc_Explorer;

    Movement::Speed           = d.Mov_Speed;
    Movement::Speed_Value     = d.Mov_Speed_Value;
    Movement::Speed_Type      = d.Mov_Speed_Type;
    Movement::Speed_Key       = VKToImGuiKey(d.Mov_Speed_Key);
    Movement::Speed_Mode      = (ImKeyBindMode)d.Mov_Speed_Mode;
    Movement::JumpPower       = d.Mov_JumpPower;
    Movement::JumpPower_Value = d.Mov_JumpPower_Value;
    Movement::JumpPower_Key   = VKToImGuiKey(d.Mov_JumpPower_Key);
    Movement::JumpPower_Mode  = (ImKeyBindMode)d.Mov_JumpPower_Mode;
    Movement::Macro           = d.Mov_Macro;
    Movement::Macro_Type      = d.Mov_Macro_Type;
    Movement::Macro_Key       = VKToImGuiKey(d.Mov_Macro_Key);
    Movement::Macro_Mode      = (ImKeyBindMode)d.Mov_Macro_Mode;
    Movement::Spinbot         = d.Mov_Spinbot;
    Movement::Spinbot_Type    = d.Mov_Spinbot_Type;
    Movement::Spinbot_Speed   = d.Mov_Spinbot_Speed;

    Radar::Enabled            = d.Rad_Enabled;
    Radar::Size               = d.Rad_Size;
    Radar::Range              = d.Rad_Range;
    Radar::Opacity            = d.Rad_Opacity;
    Radar::PosX               = d.Rad_PosX;
    Radar::PosY               = d.Rad_PosY;
    Radar::EnemyFlags         = d.Rad_EnemyFlags;

    Visuals::LookDirection    = d.Vis_LookDir;
    memcpy(Visuals::LookDirColor, d.Vis_LookDirColor, sizeof(Visuals::LookDirColor));

    Visuals::Snapline         = d.Vis_Snapline;
    Visuals::SnaplinePosition = d.Vis_SnaplinePosition;
    Visuals::HeadDot          = d.Vis_HeadDot;
    Visuals::HeadDotSize      = d.Vis_HeadDotSize;

    Watermark::Enabled              = d.Wm_Enabled;
    Watermark::ShowPlayers          = d.Wm_ShowPlayers;
    Watermark::ShowGame             = d.Wm_ShowGame;

    Visuals::HitEffectsType         = d.Vis_HitEffectsType;
    Visuals::HitEffectsDuration     = d.Vis_HitEffectsDuration;
    std::copy_n(d.Vis_HitEffectsColor, 4, Visuals::HitEffectsColor);
    Visuals::HitNotifications       = d.Vis_HitNotifications;
    Visuals::ShotTracers            = d.Vis_ShotTracers;
    Visuals::ShotTracerType         = d.Vis_ShotTracerType;
    std::copy_n(d.Vis_ShotTracerColor, 4, Visuals::ShotTracerColor);

    return true;
}

inline void ResetDefaults()
{
    using namespace Globals;
    Settings::Team_Check         = false;
    Settings::Client_Check       = false;
    Settings::FriendCheck        = false;
    Settings::Streamproof        = true;
    Settings::Hide_Console       = false;
    Settings::Performance_Mode   = 2;
    Settings::Menu_Toggle_Key    = ImGuiKey_Insert;
    Settings::Exit_Key           = ImGuiKey_Delete;
    float defAccent[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
    memcpy(Settings::AccentColor, defAccent, sizeof(defAccent));
    Settings::FontIndex          = 0;

    Aimbot::useFov = false; Aimbot::Enabled = false; Aimbot::DrawFov = false;
    Aimbot::FovSpin = false; Aimbot::FillFov = false; Aimbot::AimbotSticky = false;
    Aimbot::Shake = false; Aimbot::KnockedCheck = false; Aimbot::VisibilityCheck = false;
    Aimbot::Prediction = false; Aimbot::Prediction_X = 1.0f; Aimbot::Prediction_Y = 1.0f;
    Aimbot::ShakeX = 0.f; Aimbot::ShakeY = 0.f; Aimbot::ShakeZ = 0.f;
    Aimbot::FovSize = 50.f; Aimbot::HitPart = 0; Aimbot::Aimbot_type = 0;
    float defAimCol[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
    memcpy(Aimbot::FovColor, defAimCol, sizeof(defAimCol));
    Aimbot::FovSpinSpeed = 1; Aimbot::Aimbot_Key = ImGuiKey_Q; Aimbot::Aimbot_Mode = ImKeyBindMode_Hold;
    Aimbot::Camera::Smoothing_X = 1.f; Aimbot::Camera::Smoothing_Y = 1.f;
    Aimbot::Mouse::Smoothing_X = 1.f; Aimbot::Mouse::Smoothing_Y = 1.f; Aimbot::Mouse::Mouse_Sensitivty = 1.f;

    Silent::DrawFov = false; Silent::Enabled = false; Silent::StickyAim = false;
    Silent::SpoofMouse = true; Silent::UseFov = false; Silent::KnockedCheck = false;
    Silent::GunBasedFov = false; Silent::VisibilityCheck = false;
    Silent::Fov = 67.67f; Silent::FovDoubleBarrel = 67.67f;
    Silent::FovTacticalShotgun = 67.67f; Silent::FovRevolver = 67.67f;
    Silent::Silent_Key = ImGuiKey_Q; Silent::Silent_Mode = ImKeyBindMode_Toggle; Silent::AimPart = 0;
    float defSilCol[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
    memcpy(Silent::FovColor, defSilCol, sizeof(defSilCol));
    Silent::FovSpinSpeed = 1; Silent::FovSpin = false; Silent::FillFov = false;

    Visuals::Enabled = false; Visuals::Box = false; Visuals::Box_Fill = false;
    Visuals::Box_Fill_Gradient = false; Visuals::Box_Fill_Gradient_Rotate = false;
    Visuals::Healthbar = false; Visuals::Health = false; Visuals::Name = false;
    Visuals::Distance = false; Visuals::Rig_Type = false; Visuals::Tool = false;
    Visuals::Skeleton = false;
    Visuals::Chams = false;
    float defChamsCol[4] = {1.f,0.f,0.f,0.4f}; memcpy(Visuals::ChamsColor, defChamsCol, sizeof(defChamsCol));
    Visuals::Render_Distance = 1000.f;
    Visuals::BoxFillSpeed = 2; Visuals::Healthbar_Type = 1; Visuals::Box_Type = 0;
    Visuals::Box_Fill_Type = 0; Visuals::Name_Type = 1; Visuals::Gap = 2; Visuals::Thickness = 2;

    Misc::Fly = false; Misc::Fly_Speed = 50.f; Misc::Fly_Key = ImGuiKey_Z; Misc::Fly_Mode = ImKeyBindMode_Toggle;
    Misc::Fly_Type = 0; Misc::Fly_Stealth = false; Misc::Fly_BypassPulse = false;
    Misc::Fly_BypassStrength = 5.f; Misc::Fly_BypassInterval = 5.f; Misc::Fly_BypassDuration = 0.15f;
    Misc::Explorer = false;
    Movement::Speed = false; Movement::Speed_Value = 50.f; Movement::Speed_Type = 0; Movement::Speed_Key = ImGuiKey_None; Movement::Speed_Mode = ImKeyBindMode_Toggle;
    Movement::JumpPower = false; Movement::JumpPower_Value = 50.f; Movement::JumpPower_Key = ImGuiKey_None; Movement::JumpPower_Mode = ImKeyBindMode_Toggle;
    Movement::Macro = false; Movement::Macro_Type = 0; Movement::Macro_Key = ImGuiKey_None; Movement::Macro_Mode = ImKeyBindMode_Toggle;
    Movement::Spinbot = false; Movement::Spinbot_Type = 0; Movement::Spinbot_Speed = 0.067f;

    Radar::Enabled = false; Radar::Size = 200.f; Radar::Range = 150.f; Radar::Opacity = 0.7f;
    Radar::PosX = 15.f; Radar::PosY = -1.f; Radar::EnemyFlags = 3;

    Visuals::LookDirection = false; float defLDCol[4] = {1.f,1.f,1.f,1.f}; memcpy(Visuals::LookDirColor, defLDCol, sizeof(defLDCol));

    Visuals::Snapline = false; Visuals::SnaplinePosition = 0;
    Visuals::HeadDot = false; Visuals::HeadDotSize = 3.f;

    Watermark::Enabled = false;
    Watermark::ShowPlayers = true;
    Watermark::ShowGame = true;

    Visuals::HitEffectsType = 0; Visuals::HitEffectsDuration = 1.5f;
    float defHEC[4] = {1.f,1.f,1.f,0.8f}; memcpy(Visuals::HitEffectsColor, defHEC, sizeof(defHEC));
    Visuals::HitNotifications = false;
    Visuals::ShotTracers = false; Visuals::ShotTracerType = 0;
    float defSTC[4] = {1.f,1.f,1.f,1.f}; memcpy(Visuals::ShotTracerColor, defSTC, sizeof(defSTC));
}

inline void EnsureDir()
{
    CreateDirectoryA(GetDir().c_str(), nullptr);
}

inline std::string FilePath(const std::string& name)
{
    return GetDir() + "\\" + name + ".dat";
}

inline bool Save(const std::string& name)
{
    EnsureDir();
    Data d = Capture();
    d.created_at = static_cast<int64_t>(std::time(nullptr));
    d.checksum = CalcChecksum(d);
    std::string b64 = ToBase64(&d, sizeof(d));
    std::ofstream f(FilePath(name), std::ios::out | std::ios::trunc);
    if (!f) return false;
    f << b64;
    return true;
}

inline bool Load(const std::string& name)
{
    std::ifstream f(FilePath(name));
    if (!f) return false;
    std::string b64((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    std::vector<uint8_t> raw;
    if (!FromBase64(b64, raw) || raw.size() < sizeof(Data)) return false;
    Data d;
    memcpy(&d, raw.data(), sizeof(Data));
    return Apply(d);
}

inline bool Delete(const std::string& name)
{
    return DeleteFileA(FilePath(name).c_str()) != 0;
}

struct ConfigInfo {
    std::string name;
    int64_t     created_at;  // 0 if unknown
};

inline std::vector<ConfigInfo> ListConfigs()
{
    std::vector<ConfigInfo> out;
    EnsureDir();
    WIN32_FIND_DATAA fd{};
    std::string pat = GetDir() + "\\*.dat";
    HANDLE h = FindFirstFileA(pat.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return out;
    do {
        std::string fn = fd.cFileName;
        if (fn.size() <= 4) continue;
        std::string n = fn.substr(0, fn.size() - 4);
        if (n.empty()) continue;
        int64_t ts = 0;
        std::string fp = GetDir() + "\\" + fn;
        std::ifstream f(fp, std::ios::in);
        if (f) {
            std::string b64((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            std::vector<uint8_t> raw;
            if (FromBase64(b64, raw) && raw.size() >= offsetof(Data, created_at) + sizeof(ts)) {
                memcpy(&ts, raw.data() + offsetof(Data, created_at), sizeof(ts));
            }
        }
        if (ts < 0) ts = 0;
        out.push_back({ n, ts });
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return out;
}

} // namespace Config
