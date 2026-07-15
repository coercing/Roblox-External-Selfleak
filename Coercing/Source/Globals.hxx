#pragma once
#include <memory>
#include <vector>
#include <atomic>
#include "Engine/Engine.h"
#include "Core/Features/Cache/Cache.h"
#include "ImGui/imgui.h"
#include "ImGui/addons/imgui_addons.h"
namespace Globals {
	inline SDK::Datamodel Datamodel;
	inline std::uint64_t GameID;
	inline SDK::VisualEngine VisualEngine;
	inline SDK::Player LocalPlayer;
	inline SDK::Players Players;
	inline SDK::Datamodel Workspace;
	inline SDK::Lighting Lighting;
	inline SDK::Renderview Renderview;
	inline SDK::Camera Camera;
	inline std::vector<SDK::Player> Player_Cache;
	namespace Settings {
		inline bool Team_Check;
		inline bool Client_Check = true;
		inline bool FriendCheck;
		inline bool Streamproof = true;
		inline bool Hide_Console = false;
		inline int Performance_Mode = 2;
		inline ImGuiKey Menu_Toggle_Key = ImGuiKey_Insert;
		inline ImGuiKey Exit_Key = ImGuiKey_Delete;
		inline float AccentColor[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
		inline int FontIndex = 0;
		inline bool FontRebuildPending = false;
		
	}
	namespace World {
		inline bool Fog;
		inline bool Exposure;
		inline bool FOV;
		inline float Fog_Distance = 300.0f;
		inline float FOV_Distance = 90.0f;
		inline float ExposureI = 0.0f;
		namespace Colors {
			inline float Fog[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
		}
	}
	namespace Aimbot {
		inline bool useFov;
		inline bool Enabled;
		inline bool DrawFov;
		inline bool FovSpin;
		inline bool ClosestPlayerFound;
		inline bool FillFov;
		inline bool AimbotSticky;
		inline bool Shake;
		inline bool KnockedCheck;
		inline bool VisibilityCheck;
		inline bool Prediction;
		inline float ShakeX{ 0.0f };
		inline float ShakeY{ 0.0f };
		inline float ShakeZ{ 0.0f };
		inline float Prediction_X{ 1.0f };
		inline float Prediction_Y{ 1.0f };
		inline SDK::Instance AimTarget;
		inline float FovSize = 50;
		inline int HitPart = 0; // 0=Head 1=HRP 2=UpperTorso 3=LowerTorso 4=LeftHand 5=RightHand 6=LeftUpperArm 7=RightUpperArm 8=LeftUpperLeg 9=RightUpperLeg 10=LeftFoot 11=RightFoot
		inline int Aimbot_type = 0; // 0 = Mouse , 1 = Camera
		inline float FovColor[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
		inline int FovSpinSpeed = 1;
		inline ImGuiKey Aimbot_Key = ImGuiKey_Q;
		inline ImKeyBindMode Aimbot_Mode = ImKeyBindMode_Hold;
		namespace Camera {
			inline float Smoothing_X{ 1.0f };
			inline float Smoothing_Y{ 1.0f };
		}
		namespace Mouse {
			inline float Smoothing_X{ 1.0f };
			inline float Smoothing_Y{ 1.0f };
			inline float Mouse_Sensitivty{ 1.0f };
		}
	}
	namespace Silent
	{
		inline bool DrawFov{ false };
		inline bool Enabled{ false };
		inline bool Raycast{ false };
		inline bool StickyAim{ false };
		inline bool SpoofMouse{ true };
		inline bool UseFov{ false };
		inline bool KnockedCheck{ false };
		inline bool GunBasedFov{ false };
		inline bool VisibilityCheck{ false };
		inline float Fov{ 67.67f };
		inline float FovDoubleBarrel{ 67.67f };
		inline float FovTacticalShotgun{ 67.67f };
		inline float FovRevolver{ 67.67f };
		inline ImGuiKey Silent_Key = ImGuiKey_Q;
		inline ImKeyBindMode Silent_Mode = ImKeyBindMode_Toggle;
		// 0=Head 1=HRP 2=UpperTorso 3=LowerTorso 4=LeftHand 5=RightHand
		// 6=LeftUpperArm 7=RightUpperArm 8=LeftUpperLeg 9=RightUpperLeg 10=LeftFoot 11=RightFoot
		inline int AimPart{ 0 };
		inline float FovColor[4]{ 0.372549f, 0.784314f, 0.992157f, 1.0f };
		inline int FovSpinSpeed = 1;
		inline bool FovSpin;
		inline bool FillFov;
	}
	namespace Visuals {
		inline bool Enabled;
		inline bool Box;
		inline bool Box_Fill;
		inline bool Box_Fill_Gradient;
		inline bool Box_Fill_Gradient_Rotate;
		inline bool Healthbar;
		inline bool Health;
		inline bool Name;
		inline bool Distance;
		inline bool Rig_Type;
		inline bool Tool;
		inline bool Skeleton;
		inline float Render_Distance = 1000.0f;
		inline int BoxFillSpeed = 2;
		inline int Healthbar_Type = 1;
		inline int Box_Type = 0;
		inline int Box_Fill_Type = 0;
		inline int Name_Type = 1;
		inline int Gap = 2;
		inline int Thickness = 2;
		inline bool LookDirection = false;
		inline float LookDirColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inline bool Snapline = false;
		inline int SnaplinePosition = 0;
		inline bool HeadDot = false;
		inline float HeadDotSize = 3.f;
		// Hit effects
		inline int HitEffectsType = 0;        // 0=off, 1=hitmarker, 2=ghost skeleton, 3=ghost chams
		inline float HitEffectsDuration = 1.5f;
		inline float HitEffectsColor[4] = { 1.f, 1.f, 1.f, 0.8f };
		inline bool Chams = false;
		inline float ChamsColor[4] = { 1.f, 0.f, 0.f, 0.4f };
		inline bool HitNotifications = false;
		// Shot tracers
		inline bool ShotTracers = false;
		inline int ShotTracerType = 0;         // 0=line, 1=beam, 2=dots
		inline float ShotTracerColor[4] = { 1.f, 1.f, 1.f, 1.f };
		namespace Colors {
			inline float Primary[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
			inline float Box[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
			inline float BoxOutline[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			inline float BoxFill_Top[4] = { 0.372549f, 0.784314f, 0.992157f, 0.32f };
			inline float BoxFill_Bottom[4] = { 0.05f, 0.18f, 0.45f, 0.48f };
			inline float Name[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
			inline float Distance[4] = { 0.372549f, 0.784314f, 0.992157f, 0.95f };
			inline float Rig_Type[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
			inline float Tool[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
			inline float Health[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
			inline float Healthbar[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
			inline float Healthbar_Top[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
			inline float Healthbar_Middle[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
			inline float Healthbar_Bottom[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
			inline float Skeleton[4] = { 0.372549f, 0.784314f, 0.992157f, 0.95f };
			inline float Visible[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
			inline float Invisible[4] = { 0.18f, 0.48f, 0.88f, 1.0f };
			inline float Friend[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
			inline float Snapline[4] = { 0.372549f, 0.784314f, 0.992157f, 0.78f };
			inline float HeadDot[4] = { 0.372549f, 0.784314f, 0.992157f, 1.0f };
			inline float FOVCircle[4] = { 0.372549f, 0.784314f, 0.992157f, 0.28f };
		}
	}
	namespace Misc {
		inline bool Fly = false;
		inline float Fly_Speed = 50.0f;
		inline ImGuiKey Fly_Key = ImGuiKey_Z;
		inline ImKeyBindMode Fly_Mode = ImKeyBindMode_Toggle;
		inline int Fly_Type = 0;            // 0=Velocity, 1=CFrame, 2=Anchored, 3=Multi-part
		inline bool Fly_Stealth = false;
		inline bool Fly_BypassPulse = false;
		inline float Fly_BypassStrength = 5.f;
		inline float Fly_BypassInterval = 5.f;
		inline float Fly_BypassDuration = 0.15f;
		inline bool Explorer = false;
	}

	namespace Movement {
		inline bool Speed = false;
		inline float Speed_Value = 50.0f;
		inline int Speed_Type = 0;
		inline ImGuiKey Speed_Key = ImGuiKey_None;
		inline ImKeyBindMode Speed_Mode = ImKeyBindMode_Toggle;
		inline bool JumpPower = false;
		inline float JumpPower_Value = 50.0f;
		inline ImGuiKey JumpPower_Key = ImGuiKey_None;
		inline ImKeyBindMode JumpPower_Mode = ImKeyBindMode_Toggle;
		inline bool Macro = false;
		inline int Macro_Type = 0; // 0 = IOIOIO, 1 = Mouse Wheel
		inline ImGuiKey Macro_Key = ImGuiKey_None;
		inline ImKeyBindMode Macro_Mode = ImKeyBindMode_Toggle;
		inline bool Spinbot = false;
		inline int Spinbot_Type = 0; // 0=Y-Spin 1=Jitter 2=Backwards 3=Random
		inline float Spinbot_Speed = 0.067f;
	}
	namespace Radar {
		inline bool Enabled = false;
		inline float Size = 200.f;
		inline float Range = 150.f;
		inline float Opacity = 0.7f;
		inline float PosX = 15.f;
		inline float PosY = -1.f;
		inline int EnemyFlags = 3; // bit 1=dot 2=name 4=distance
	}
	namespace Watermark {
		inline bool Enabled = true;
		inline bool ShowPlayers = true;
		inline bool ShowGame = true;
	}
}