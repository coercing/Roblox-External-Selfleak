#include <Windows.h>
#include <thread>
#include <cmath>
#include <string>
#include <algorithm>
#include <vector>
#include <mutex>

#include "Aimbot.h"
#include "../../Cache/Cache.h"
#include "../../../../Engine/Engine.h"
#include <Globals.hxx>
#include "../../../../Engine/Offsets/Offsets.h"
#include "../Misc/WallCheck.h"

namespace Aimbot {
    std::string CurrentLockedName = "";
    SDK::Vector3 AimPositionW = { 0, 0, 0 };
    SDK::Vector2 AimPositionS = { 0, 0 };
    bool TargetFound = false;

    std::string PersistenceName = "";
    bool IsPersisting = false;

    static int LockedRandomIdx = 0;
    static std::string LastRandomTarget = "";

    static bool IsPlayerKnocked(SDK::Player& Player)
    {
        if (Player.Character.Address == 0)
            return false;

        SDK::Instance BodyEffects = Player.Character.Find_First_Child("BodyEffects");
        if (BodyEffects.Address == 0)
            return false;

        SDK::Instance Ko = BodyEffects.Find_First_Child("K.O");
        if (Ko.Address == 0)
            return false;

        bool KoValue = Driver->Read<bool>(Ko.Address + Offsets::Misc::Value);

        return KoValue;
    }

    static SDK::Matrix3 LerpMatrix3(const SDK::Matrix3& A, const SDK::Matrix3& B, float T) {
        SDK::Matrix3 Result;
        for (int I = 0; I < 9; ++I) Result.data[I] = A.data[I] + (B.data[I] - A.data[I]) * T;
        return Result;
    }

    static SDK::Vector3 Cross(const SDK::Vector3& A, const SDK::Vector3& B) {
        return { A.y * B.z - A.z * B.y, A.z * B.x - A.x * B.z, A.x * B.y - A.y * B.x };
    }

    static SDK::Matrix3 LookAtToMatrix(const SDK::Vector3& CamPos, const SDK::Vector3& TargetPos) {
        SDK::Vector3 Forward = (TargetPos - CamPos).normalize();
        SDK::Vector3 Right = Cross({ 0.f, 1.f, 0.f }, Forward).normalize();
        SDK::Vector3 Up = Cross(Forward, Right);

        SDK::Matrix3 M;
        M.data[0] = -Right.x; M.data[1] = Up.x; M.data[2] = -Forward.x;
        M.data[3] = Right.y;  M.data[4] = Up.y; M.data[5] = -Forward.y;
        M.data[6] = -Right.z; M.data[7] = Up.z; M.data[8] = -Forward.z;
        return M;
    }

    static SDK::Vector3 PickBone(const std::vector<SDK::Vector3>& Bones, POINT CursorPos) {
        int idx = Globals::Aimbot::HitPart;
        if (idx == 3) {
            SDK::VisualEngine Ve(Globals::VisualEngine.Address);
            float bestDist = 9e9f;
            SDK::Vector3 best = Bones[0];
            for (const auto& b : Bones) {
                if (std::isnan(b.x) || std::isnan(b.y) || std::isnan(b.z)) continue;
                auto s = Ve.World_To_Screen(b);
                if (s.x < 0 || s.y < 0) continue;
                float d = sqrtf((s.x - CursorPos.x) * (s.x - CursorPos.x) + (s.y - CursorPos.y) * (s.y - CursorPos.y));
                if (d < bestDist) { bestDist = d; best = b; }
            }
            return best;
        }
        if (idx == 4) {
            if (idx >= (int)Bones.size()) idx = 0;
            LockedRandomIdx = rand() % (int)Bones.size();
            return Bones[LockedRandomIdx];
        }
        if (idx >= (int)Bones.size()) idx = 0;
        return Bones[idx];
    }

    static void BuildBones(SDK::Player& Plr, std::vector<SDK::Vector3>& out) {
        auto add = [&](const SDK::Instance& inst) {
            if (inst.Address) out.push_back(SDK::Part(inst.Address).Get_PartPosition());
        };
        add(Plr.Head);
        add(Plr.Torso);
        add(Plr.UpperTorso);
        add(Plr.LowerTorso);
        add(Plr.HumanoidRootPart);
        add(Plr.LeftUpperArm);  add(Plr.RightUpperArm);
        add(Plr.LeftLowerArm);  add(Plr.RightLowerArm);
        add(Plr.LeftHand);      add(Plr.RightHand);
        add(Plr.LeftUpperLeg);  add(Plr.RightUpperLeg);
        add(Plr.LeftLowerLeg);  add(Plr.RightLowerLeg);
        add(Plr.LeftFoot);      add(Plr.RightFoot);
    }

    static bool IsCandidate(SDK::Player& Plr) {
        if (Plr.Local_Player || !Plr.Character.Address || !Plr.Head.Address) return false;
        if (Plr.Health <= 0.f) return false;
        if (Globals::Aimbot::KnockedCheck && IsPlayerKnocked(Plr)) return false;
        return true;
    }

    SDK::Player GetClosestToMouse() {
        POINT CursorPos;
        if (!GetCursorPos(&CursorPos)) return {};
        ScreenToClient(FindWindowA(0, "Roblox"), &CursorPos);

        SDK::Player best;
        float bestDist = 9e9f;

        SDK::VisualEngine Ve(Globals::VisualEngine.Address);
        if (!Ve.Address) return {};

        SDK::Vector3 camOrigin{};
        if (Globals::Aimbot::VisibilityCheck) {
            SDK::Datamodel Dm(Globals::Datamodel.Address);
            SDK::Instance CameraInst = Dm.Find_First_Child_Of_Class("Workspace")
                .Find_First_Child("Camera");
            if (CameraInst.Address)
                camOrigin = SDK::Camera(CameraInst.Address).Get_CameraPos();
        }

        for (auto& Plr : Globals::Player_Cache) {
            if (!IsCandidate(Plr)) continue;

            std::vector<SDK::Vector3> Bones;
            BuildBones(Plr, Bones);
            if (Bones.empty()) continue;

            SDK::Vector3 bonePos = PickBone(Bones, CursorPos);
            if (std::isnan(bonePos.x) || std::isnan(bonePos.y) || std::isnan(bonePos.z)) continue;

            if (Globals::Aimbot::VisibilityCheck && camOrigin.x != 0.f) {
                if (!wallcheck::is_visible(camOrigin, bonePos, Globals::LocalPlayer.Character.Address, false))
                    continue;
            }

            auto ScreenPos = Ve.World_To_Screen(bonePos);
            if (ScreenPos.x < 0 || ScreenPos.y < 0) continue;

            float Dist2D = sqrtf((ScreenPos.x - CursorPos.x) * (ScreenPos.x - CursorPos.x) + (ScreenPos.y - CursorPos.y) * (ScreenPos.y - CursorPos.y));

            if (Globals::Aimbot::useFov && Dist2D > Globals::Aimbot::FovSize) continue;

            if (Dist2D < bestDist) {
                bestDist = Dist2D;
                best = Plr;
            }
        }

        return best;
    }

    void AcquireTarget() {
        TargetFound = false;
        if (!Globals::VisualEngine.Address) return;

        if (Globals::Aimbot::AimbotSticky && IsPersisting && !PersistenceName.empty()) {
        std::vector<SDK::Player> aimSnapshot;
        {
            std::lock_guard<std::mutex> lock(Cache::Mutex);
            aimSnapshot = Globals::Player_Cache;
        }
        for (auto& Plr : aimSnapshot) {
                if (Plr.Name != PersistenceName) continue;
                if (!IsCandidate(Plr)) {
                    IsPersisting = false; PersistenceName = "";
                    Globals::Aimbot::AimTarget = SDK::Instance(0);
                    return;
                }
                std::vector<SDK::Vector3> Bones;
                BuildBones(Plr, Bones);
                if (Bones.empty()) {
                    IsPersisting = false; PersistenceName = "";
                    Globals::Aimbot::AimTarget = SDK::Instance(0);
                    return;
                }
                POINT CursorPos;
                if (!GetCursorPos(&CursorPos)) { IsPersisting = false; PersistenceName = ""; Globals::Aimbot::AimTarget = SDK::Instance(0); return; }
                ScreenToClient(FindWindowA(0, "Roblox"), &CursorPos);

                SDK::VisualEngine Ve(Globals::VisualEngine.Address);
                SDK::Vector3 bonePos = PickBone(Bones, CursorPos);
                if (std::isnan(bonePos.x) || std::isnan(bonePos.y) || std::isnan(bonePos.z)) {
                    IsPersisting = false; PersistenceName = "";
                    Globals::Aimbot::AimTarget = SDK::Instance(0);
                    return;
                }
                auto ScreenPos = Ve.World_To_Screen(bonePos);
                float Dist2D = sqrtf((ScreenPos.x - CursorPos.x) * (ScreenPos.x - CursorPos.x) + (ScreenPos.y - CursorPos.y) * (ScreenPos.y - CursorPos.y));

                if (Globals::Aimbot::Aimbot_type == 1 && Dist2D > Globals::Aimbot::FovSize) {
                    IsPersisting = false; PersistenceName = "";
                    Globals::Aimbot::AimTarget = SDK::Instance(0);
                    return;
                }
                AimPositionW = bonePos;
                AimPositionS = { ScreenPos.x, ScreenPos.y };
                CurrentLockedName = Plr.Name;
                Globals::Aimbot::AimTarget = SDK::Instance(Plr.Character.Address);
                TargetFound = true;
                return;
            }
            IsPersisting = false; PersistenceName = "";
            Globals::Aimbot::AimTarget = SDK::Instance(0);
            return;
        }

        auto target = GetClosestToMouse();
        if (target.Character.Address) {
            std::vector<SDK::Vector3> Bones;
            BuildBones(target, Bones);
            if (!Bones.empty()) {
                POINT CursorPos;
                GetCursorPos(&CursorPos);
                ScreenToClient(FindWindowA(0, "Roblox"), &CursorPos);
                SDK::VisualEngine Ve(Globals::VisualEngine.Address);
                SDK::Vector3 bonePos = PickBone(Bones, CursorPos);
                if (!std::isnan(bonePos.x) && !std::isnan(bonePos.y) && !std::isnan(bonePos.z)) {
                    auto ScreenPos = Ve.World_To_Screen(bonePos);
                    AimPositionW = bonePos;
                    AimPositionS = { ScreenPos.x, ScreenPos.y };
                    CurrentLockedName = target.Name;
                    Globals::Aimbot::AimTarget = SDK::Instance(target.Character.Address);
                    TargetFound = true;
                }
            }
        }

        if (TargetFound && Globals::Aimbot::AimbotSticky && !IsPersisting) {
            PersistenceName = CurrentLockedName;
            IsPersisting = true;
        }
    }

    void UpdateAimbot() {
        if (!TargetFound) return;

        if (Globals::Aimbot::Aimbot_type == 1) {
            SDK::Datamodel Dm(Globals::Datamodel.Address);
            SDK::Instance WorkspaceInst = Dm.Find_First_Child_Of_Class("Workspace");
            SDK::Instance CameraInst = WorkspaceInst.Find_First_Child("Camera");
            if (!CameraInst.Address) return;

            SDK::Camera Cam(CameraInst.Address);
            auto CamPos = Cam.Get_CameraPos();
            auto TargetPos = AimPositionW;

            TargetPos.x += Globals::Aimbot::ShakeX;
            TargetPos.y += Globals::Aimbot::ShakeY;
            TargetPos.z += Globals::Aimbot::ShakeZ;

            if (Globals::Aimbot::Shake) {
                TargetPos.x += ((float)rand() / RAND_MAX * 2 - 1) * Globals::Aimbot::ShakeX;
                TargetPos.y += ((float)rand() / RAND_MAX * 2 - 1) * Globals::Aimbot::ShakeY;
                TargetPos.z += ((float)rand() / RAND_MAX * 2 - 1) * Globals::Aimbot::ShakeZ;
            }

            float SmoothFactor = (std::max)(Globals::Aimbot::Camera::Smoothing_X, 0.f) / 100.f;
            SmoothFactor = std::pow(SmoothFactor, 1.2f);

            SDK::Matrix3 TargetMatrix = LookAtToMatrix(CamPos, TargetPos);
            SDK::Matrix3 CurrentMatrix = Cam.Get_CameraRot();
            SDK::Matrix3 FinalMatrix = LerpMatrix3(CurrentMatrix, TargetMatrix, 1.f - SmoothFactor);

            Cam.Set_CameraRot(FinalMatrix);
        }
        else {
            POINT CursorPos;
            if (!GetCursorPos(&CursorPos)) return;
            ScreenToClient(FindWindowA(0, "Roblox"), &CursorPos);

            float Sensitivity = Globals::Aimbot::Mouse::Mouse_Sensitivty;
            if (Globals::Aimbot::Mouse::Smoothing_X > 0) {
                float SmoothVal = Globals::Aimbot::Mouse::Smoothing_X;
                if (SmoothVal < 1.f) SmoothVal = 1.f;
                if (SmoothVal > 100.f) SmoothVal = 100.f;
                Sensitivity /= SmoothVal;
            }

            float MoveX = (AimPositionS.x - CursorPos.x) * Sensitivity;
            float MoveY = (AimPositionS.y - CursorPos.y) * Sensitivity;

            if (Globals::Aimbot::Shake) {
                MoveX += ((float)rand() / RAND_MAX * 2 - 1) * Globals::Aimbot::ShakeX;
                MoveY += ((float)rand() / RAND_MAX * 2 - 1) * Globals::Aimbot::ShakeY;
            }

            if (MoveX < -100.f) MoveX = -100.f;
            if (MoveX > 100.f) MoveX = 100.f;
            if (MoveY < -100.f) MoveY = -100.f;
            if (MoveY > 100.f) MoveY = 100.f;

            if (abs(MoveX) >= 1.f || abs(MoveY) >= 1.f) {
                INPUT Input = {};
                Input.type = INPUT_MOUSE;
                Input.mi.dx = (LONG)MoveX;
                Input.mi.dy = (LONG)MoveY;
                Input.mi.dwFlags = MOUSEEVENTF_MOVE;
                SendInput(1, &Input, sizeof(INPUT));
            }
        }
    }

    void RunService() {
        std::thread([]() {

            bool Toggled = false;
            bool LastPressed = false;

            while (true) {

                if (Globals::Aimbot::Enabled) {

                    int Vk = ImGuiKeyToVK(Globals::Aimbot::Aimbot_Key);
                    if (!Vk) { Sleep(1); continue; }

                    bool Pressed = (GetAsyncKeyState(Vk) & 0x8000) != 0;

                    if (Globals::Aimbot::Aimbot_Mode == ImKeyBindMode_Toggle) {

                        if (Pressed && !LastPressed)
                            Toggled = !Toggled;

                        if (Toggled) {
                            AcquireTarget();
                            UpdateAimbot();
                        }
                        else {
                            CurrentLockedName = "";
                            IsPersisting = false;
                            PersistenceName = "";
                            LastRandomTarget = "";
                            Globals::Aimbot::AimTarget = SDK::Instance(0);
                        }
                    }
                    else {

                        if (Pressed) {
                            AcquireTarget();
                            UpdateAimbot();
                        }
                        else {
                            CurrentLockedName = "";
                            IsPersisting = false;
                            PersistenceName = "";
                            LastRandomTarget = "";
                            Globals::Aimbot::AimTarget = SDK::Instance(0);
                        }
                    }

                    LastPressed = Pressed;
                }

                Sleep(1);
            }

            }).detach();
    }

}