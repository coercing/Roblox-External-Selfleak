#include "Movement.h"
#include <Windows.h>
#include <thread>
#include <chrono>
#include <cmath>
#include <vector>
#include "Engine/Engine.h"
#include "Globals.hxx"
#include "ImGui/addons/imgui_addons.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

static bool ValidAddr(uintptr_t a) { return a != 0; }

// ── Fly ──────────────────────────────────────────────────────────────────────

static void FlyThread() {
    bool wasEnabled = false;
    ImKeybindState kbState;
    bool wasActive = false;

    SDK::Vector3 cf_pos = { 0, 0, 0 };
    bool cf_pos_init = false;
    uintptr_t anchor_prim = 0;

    struct PartPrim { uintptr_t prim; SDK::Vector3 last_pos; };
    std::vector<PartPrim> all_prims;
    uintptr_t all_prims_character = 0;
    DWORD all_prims_tick = 0;

    DWORD last_pulse = 0;
    DWORD pulse_start = 0;
    bool pulsing = false;

    while (true) {
        auto& lp = Globals::LocalPlayer;
        bool enabled = Globals::Misc::Fly;

        bool active = false;
        if (enabled) {
            int vk = ImGuiKeyToVK(Globals::Misc::Fly_Key);
            active = kbState.Update(vk, Globals::Misc::Fly_Mode);
        } else {
            kbState.Reset();
        }

        bool shouldApply = enabled && active;

        if (!shouldApply) {
            if (wasActive && anchor_prim) {
                try {
                    uint8_t flags = Driver->Read<uint8_t>(anchor_prim + Offsets::Primitive::Flags);
                    flags &= ~(uint8_t)Offsets::PrimitiveFlags::Anchored;
                    for (int i = 0; i < 100; i++)
                        Driver->Write<uint8_t>(anchor_prim + Offsets::Primitive::Flags, flags);
                } catch (...) {}
                anchor_prim = 0;
            }
            if (!enabled) { wasEnabled = false; }
            wasActive = false;
            cf_pos_init = false;
            all_prims.clear();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        if (!ValidAddr(lp.HumanoidRootPart.Address)) {
            wasActive = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        uintptr_t character = lp.Character.Address;
        if (!character) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        SDK::Part hrp(lp.HumanoidRootPart.Address);
        SDK::Part prim = hrp.Get_Primitive();
        if (!prim.Address) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        wasActive = true;

        SDK::Matrix3 cam_rot = Globals::Camera.Get_CameraRot();

        SDK::Vector3 move_dir = { 0, 0, 0 };
        bool is_moving = false;
        if (GetAsyncKeyState('W') & 0x8000) { move_dir.z -= 1.f; is_moving = true; }
        if (GetAsyncKeyState('S') & 0x8000) { move_dir.z += 1.f; is_moving = true; }
        if (GetAsyncKeyState('A') & 0x8000) { move_dir.x -= 1.f; is_moving = true; }
        if (GetAsyncKeyState('D') & 0x8000) { move_dir.x += 1.f; is_moving = true; }
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) { move_dir.y += 1.f; is_moving = true; }
        if (GetAsyncKeyState(VK_LCONTROL) & 0x8000) { move_dir.y -= 1.f; is_moving = true; }

        float mag = sqrtf(move_dir.x * move_dir.x + move_dir.y * move_dir.y + move_dir.z * move_dir.z);
        if (mag > 0.f) { move_dir.x /= mag; move_dir.y /= mag; move_dir.z /= mag; }

        SDK::Vector3 world_dir = cam_rot * move_dir;

        float speed = Globals::Misc::Fly_Speed;
        SDK::Vector3 zero = { 0.f, 0.f, 0.f };

        int flyType = Globals::Misc::Fly_Type;

        try {
            if (flyType == 0) {
                SDK::Vector3 vel = is_moving
                    ? SDK::Vector3{ world_dir.x * speed, world_dir.y * speed, world_dir.z * speed }
                    : zero;
                for (int i = 0; i < 500; i++)
                    Driver->Write<SDK::Vector3>(prim.Address + Offsets::Primitive::AssemblyLinearVelocity, vel);
            }
            else if (flyType == 1) {
                if (!cf_pos_init) {
                    cf_pos = Driver->Read<SDK::Vector3>(prim.Address + Offsets::Primitive::Position);
                    cf_pos_init = true;
                }
                SDK::Matrix3 rot = Driver->Read<SDK::Matrix3>(prim.Address + Offsets::Primitive::Rotation);
                if (is_moving) {
                    float step = speed / 165.f;
                    cf_pos.x += world_dir.x * step;
                    cf_pos.y += world_dir.y * step;
                    cf_pos.z += world_dir.z * step;
                }
                for (int i = 0; i < 500; i++) {
                    Driver->Write<SDK::Vector3>(prim.Address + Offsets::Primitive::Position, cf_pos);
                    Driver->Write<SDK::Matrix3>(prim.Address + Offsets::Primitive::Rotation, rot);
                    Driver->Write<SDK::Vector3>(prim.Address + Offsets::Primitive::AssemblyLinearVelocity, zero);
                    Driver->Write<SDK::Vector3>(prim.Address + Offsets::Primitive::AssemblyAngularVelocity, zero);
                }
            }
            else if (flyType == 2) {
                uint8_t cur_flags = Driver->Read<uint8_t>(prim.Address + Offsets::Primitive::Flags);
                uint8_t anc_bit = (uint8_t)Offsets::PrimitiveFlags::Anchored;
                if (!(cur_flags & anc_bit)) {
                    for (int i = 0; i < 100; i++)
                        Driver->Write<uint8_t>(prim.Address + Offsets::Primitive::Flags, cur_flags | anc_bit);
                }
                anchor_prim = prim.Address;

                if (!cf_pos_init) {
                    cf_pos = Driver->Read<SDK::Vector3>(prim.Address + Offsets::Primitive::Position);
                    cf_pos_init = true;
                }
                SDK::Matrix3 rot = Driver->Read<SDK::Matrix3>(prim.Address + Offsets::Primitive::Rotation);
                if (is_moving) {
                    float step = speed / 165.f;
                    cf_pos.x += world_dir.x * step;
                    cf_pos.y += world_dir.y * step;
                    cf_pos.z += world_dir.z * step;
                }
                for (int i = 0; i < 100; i++) {
                    Driver->Write<SDK::Vector3>(prim.Address + Offsets::Primitive::Position, cf_pos);
                    Driver->Write<SDK::Matrix3>(prim.Address + Offsets::Primitive::Rotation, rot);
                    Driver->Write<SDK::Vector3>(prim.Address + Offsets::Primitive::AssemblyLinearVelocity, zero);
                }
            }
            else if (flyType == 3) {
                DWORD now_fb = GetTickCount();
                if (character != all_prims_character || all_prims.empty() || (now_fb - all_prims_tick) > 2000) {
                    all_prims.clear();
                    all_prims_character = character;
                    all_prims_tick = now_fb;
                    cf_pos_init = false;

                    SDK::Instance charInst(character);
                    auto children = charInst.Children();
                    for (auto& child : children) {
                        if (!child.Address) continue;
                        std::string cls = child.Class();
                        if (cls == "Part" || cls == "MeshPart" || cls == "UnionOperation" ||
                            cls == "SpecialMesh" || cls == "WedgePart" || cls == "CornerWedgePart") {
                            uintptr_t p = Driver->Read<uintptr_t>(child.Address + Offsets::BasePart::Primitive);
                            if (p) {
                                SDK::Vector3 pos = Driver->Read<SDK::Vector3>(p + Offsets::Primitive::Position);
                                all_prims.push_back({ p, pos });
                            }
                        }
                    }
                }

                if (!cf_pos_init) {
                    cf_pos = Driver->Read<SDK::Vector3>(prim.Address + Offsets::Primitive::Position);
                    cf_pos_init = true;
                }

                SDK::Vector3 delta = { 0.f, 0.f, 0.f };
                if (is_moving) {
                    float step = speed / 165.f;
                    delta.x = world_dir.x * step;
                    delta.y = world_dir.y * step;
                    delta.z = world_dir.z * step;
                    cf_pos.x += delta.x;
                    cf_pos.y += delta.y;
                    cf_pos.z += delta.z;
                }

                for (auto& pp : all_prims) {
                    pp.last_pos.x += delta.x;
                    pp.last_pos.y += delta.y;
                    pp.last_pos.z += delta.z;
                    for (int i = 0; i < 3; i++) {
                        Driver->Write<SDK::Vector3>(pp.prim + Offsets::Primitive::Position, pp.last_pos);
                        Driver->Write<SDK::Vector3>(pp.prim + Offsets::Primitive::AssemblyLinearVelocity, zero);
                        Driver->Write<SDK::Vector3>(pp.prim + Offsets::Primitive::AssemblyAngularVelocity, zero);
                    }
                }
                for (int i = 0; i < 3; i++) {
                    Driver->Write<SDK::Vector3>(prim.Address + Offsets::Primitive::Position, cf_pos);
                    Driver->Write<SDK::Vector3>(prim.Address + Offsets::Primitive::AssemblyLinearVelocity, zero);
                }
            }

            // ── Stealth mode ──
            if (Globals::Misc::Fly_Stealth) {
                static uintptr_t cached_humanoid = 0;
                static DWORD humanoid_tick = 0;
                DWORD now = GetTickCount();
                if (!cached_humanoid || (now - humanoid_tick) > 2000) {
                    SDK::Instance charInst(character);
                    auto h = charInst.Find_First_Child("Humanoid");
                    cached_humanoid = h.Address;
                    humanoid_tick = now;
                }

                if (flyType == 1) {
                    SDK::Vector3 move_vel = is_moving
                        ? SDK::Vector3{ world_dir.x * speed, world_dir.y * speed, world_dir.z * speed }
                        : zero;
                    for (int i = 0; i < 500; i++)
                        Driver->Write<SDK::Vector3>(prim.Address + Offsets::Primitive::AssemblyLinearVelocity, move_vel);
                }

                if (cached_humanoid) {
                    uintptr_t state_ptr = Driver->Read<uintptr_t>(cached_humanoid + Offsets::Humanoid::HumanoidState);
                    if (state_ptr)
                        Driver->Write<int>(state_ptr + Offsets::Humanoid::HumanoidStateID, 8);
                    Driver->Write<int>(cached_humanoid + Offsets::Humanoid::FloorMaterial, 256);
                    Driver->Write<bool>(cached_humanoid + Offsets::Humanoid::EvaluateStateMachine, false);
                    Driver->Write<SDK::Vector3>(cached_humanoid + Offsets::Humanoid::MoveDirection, is_moving ? world_dir : zero);
                    Driver->Write<bool>(cached_humanoid + Offsets::Humanoid::Jump, false);
                    Driver->Write<bool>(cached_humanoid + Offsets::Humanoid::PlatformStand, false);
                }
            }

            // ── Bypass pulse ──
            if (Globals::Misc::Fly_BypassPulse) {
                DWORD now_bp = GetTickCount();
                DWORD interval_ms = (DWORD)(Globals::Misc::Fly_BypassInterval * 1000.f);
                DWORD duration_ms = (DWORD)(Globals::Misc::Fly_BypassDuration * 1000.f);

                if (!pulsing && (now_bp - last_pulse) >= interval_ms) {
                    pulsing = true;
                    pulse_start = now_bp;
                }

                if (pulsing) {
                    float str = Globals::Misc::Fly_BypassStrength;
                    SDK::Vector3 pulse_vel = { 0.f, -str, 0.f };
                    for (int i = 0; i < 500; i++)
                        Driver->Write<SDK::Vector3>(prim.Address + Offsets::Primitive::AssemblyLinearVelocity, pulse_vel);

                    if ((now_bp - pulse_start) >= duration_ms) {
                        pulsing = false;
                        last_pulse = now_bp;
                    }
                }
            }
        } catch (...) {}

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// ── Speed ─────────────────────────────────────────────────────────────────────

static void SpeedThread() {
    bool    wasEnabled    = false;
    float   originalSpeed = 16.0f;
    uintptr_t lastHumanoid = 0;
    ImKeybindState kbState;
    bool wasActive = false;

    SDK::Vector3 cf_pos = {};
    bool cf_pos_init = false;

    while (true) {
        auto& lp = Globals::LocalPlayer;
        bool enabled = Globals::Movement::Speed;

        bool active = false;
        if (enabled) {
            int vk = ImGuiKeyToVK(Globals::Movement::Speed_Key);
            active = kbState.Update(vk, Globals::Movement::Speed_Mode);
        } else {
            kbState.Reset();
        }

        bool shouldApply = enabled && active;

        if (!shouldApply) {
            if (wasActive && ValidAddr(lp.Humanoid.Address)) {
                try {
                    Driver->Write<float>(lp.Humanoid.Address + Offsets::Humanoid::Walkspeed,      originalSpeed);
                    Driver->Write<float>(lp.Humanoid.Address + Offsets::Humanoid::WalkspeedCheck, originalSpeed);
                    if (ValidAddr(lp.HumanoidRootPart.Address)) {
                        SDK::Part hrp(lp.HumanoidRootPart.Address);
                        SDK::Vector3 vel = hrp.Get_Velocity();
                        hrp.Write_Velocity({ 0.f, vel.y, 0.f });
                    }
                } catch (...) {}
            }
            if (!enabled) { wasEnabled = false; lastHumanoid = 0; }
            wasActive = false;
            cf_pos_init = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        if (!ValidAddr(lp.HumanoidRootPart.Address)) {
            cf_pos_init = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        if (Globals::Movement::Speed_Type == 0 && !ValidAddr(lp.Humanoid.Address)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        if (Globals::Movement::Speed_Type == 0) {
            if (!wasEnabled || lp.Humanoid.Address != lastHumanoid) {
                try {
                    originalSpeed = Driver->Read<float>(lp.Humanoid.Address + Offsets::Humanoid::Walkspeed);
                    if (originalSpeed > 100.f || originalSpeed <= 0.f) originalSpeed = 16.0f;
                } catch (...) { originalSpeed = 16.0f; }
                lastHumanoid = lp.Humanoid.Address;
                wasEnabled = true;
            }
        }

        wasActive = true;

        try {
            int stype = Globals::Movement::Speed_Type;
            float spd = Globals::Movement::Speed_Value;

            if (stype == 0) {
                Driver->Write<float>(lp.Humanoid.Address + Offsets::Humanoid::Walkspeed,      spd);
                Driver->Write<float>(lp.Humanoid.Address + Offsets::Humanoid::WalkspeedCheck, spd);
            }
            else if (stype == 1) {
                SDK::Matrix3 cam_rot = Globals::Camera.Get_CameraRot();
                SDK::Vector3 move_dir = { 0, 0, 0 };
                bool is_moving = false;
                if (GetAsyncKeyState('W') & 0x8000) { move_dir.z -= 1.f; is_moving = true; }
                if (GetAsyncKeyState('S') & 0x8000) { move_dir.z += 1.f; is_moving = true; }
                if (GetAsyncKeyState('A') & 0x8000) { move_dir.x -= 1.f; is_moving = true; }
                if (GetAsyncKeyState('D') & 0x8000) { move_dir.x += 1.f; is_moving = true; }

                if (is_moving) {
                    float mag = sqrtf(move_dir.x * move_dir.x + move_dir.z * move_dir.z);
                    if (mag > 0.f) { move_dir.x /= mag; move_dir.z /= mag; }
                    SDK::Vector3 world_dir = cam_rot * move_dir;
                    world_dir.y = 0.f;
                    float wm = sqrtf(world_dir.x * world_dir.x + world_dir.z * world_dir.z);
                    if (wm > 0.f) { world_dir.x /= wm; world_dir.z /= wm; }

                    SDK::Part hrp(lp.HumanoidRootPart.Address);
                    SDK::Vector3 cur = hrp.Get_Velocity();
                    float safe_y = cur.y;
                    if (safe_y > 52.f) safe_y = 52.f;
                    SDK::Vector3 vel = { world_dir.x * spd, safe_y, world_dir.z * spd };
                    for (int i = 0; i < 300; i++)
                        hrp.Write_Velocity(vel);
                }
            }
            else if (stype == 2) {
                if (!cf_pos_init) {
                    SDK::Part hrp(lp.HumanoidRootPart.Address);
                    SDK::Part prim = hrp.Get_Primitive();
                    if (prim.Address) {
                        cf_pos = Driver->Read<SDK::Vector3>(prim.Address + Offsets::Primitive::Position);
                        cf_pos_init = true;
                    }
                }

                if (cf_pos_init) {
                    SDK::Part hrp(lp.HumanoidRootPart.Address);
                    SDK::Part prim = hrp.Get_Primitive();
                    if (prim.Address) {
                        SDK::Matrix3 cam_rot = Globals::Camera.Get_CameraRot();
                        SDK::Vector3 move_dir = { 0, 0, 0 };
                        bool is_moving = false;
                        if (GetAsyncKeyState('W') & 0x8000) { move_dir.z -= 1.f; is_moving = true; }
                        if (GetAsyncKeyState('S') & 0x8000) { move_dir.z += 1.f; is_moving = true; }
                        if (GetAsyncKeyState('A') & 0x8000) { move_dir.x -= 1.f; is_moving = true; }
                        if (GetAsyncKeyState('D') & 0x8000) { move_dir.x += 1.f; is_moving = true; }

                        if (is_moving) {
                            float mag = sqrtf(move_dir.x * move_dir.x + move_dir.z * move_dir.z);
                            if (mag > 0.f) { move_dir.x /= mag; move_dir.z /= mag; }
                            SDK::Vector3 world_dir = cam_rot * move_dir;
                            world_dir.y = 0.f;
                            float wm = sqrtf(world_dir.x * world_dir.x + world_dir.z * world_dir.z);
                            if (wm > 0.f) { world_dir.x /= wm; world_dir.z /= wm; }

                            float step = spd / 165.f;
                            cf_pos.x += world_dir.x * step;
                            cf_pos.z += world_dir.z * step;
                        }

                        SDK::Vector3 cur_pos = Driver->Read<SDK::Vector3>(prim.Address + Offsets::Primitive::Position);
                        cf_pos.y = cur_pos.y;

                        SDK::Matrix3 rot = Driver->Read<SDK::Matrix3>(prim.Address + Offsets::Primitive::Rotation);
                        for (int i = 0; i < 500; i++) {
                            Driver->Write<SDK::Vector3>(prim.Address + Offsets::Primitive::Position, cf_pos);
                            Driver->Write<SDK::Matrix3>(prim.Address + Offsets::Primitive::Rotation, rot);
                        }
                    }
                }
            }
        } catch (...) {}

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// ── Jump Power ────────────────────────────────────────────────────────────────

static void JumpPowerThread() {
    bool    wasEnabled       = false;
    float   originalJump     = 50.0f;
    bool    originalUseJump  = false;
    uintptr_t lastHumanoid   = 0;
    ImKeybindState kbState;
    bool wasActive = false;

    while (true) {
        auto& lp = Globals::LocalPlayer;
        bool enabled = Globals::Movement::JumpPower;

        bool active = false;
        if (enabled) {
            int vk = ImGuiKeyToVK(Globals::Movement::JumpPower_Key);
            active = kbState.Update(vk, Globals::Movement::JumpPower_Mode);
        } else {
            kbState.Reset();
        }

        bool shouldApply = enabled && active;

        if (!shouldApply) {
            if (wasActive && ValidAddr(lp.Humanoid.Address)) {
                try {
                    Driver->Write<float>(lp.Humanoid.Address + Offsets::Humanoid::JumpPower,  originalJump);
                    Driver->Write<bool> (lp.Humanoid.Address + Offsets::Humanoid::UseJumpPower, originalUseJump);
                } catch (...) {}
            }
            if (!enabled) { wasEnabled = false; lastHumanoid = 0; }
            wasActive = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        if (!ValidAddr(lp.Humanoid.Address)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        if (!wasEnabled || lp.Humanoid.Address != lastHumanoid) {
            try {
                originalJump    = Driver->Read<float>(lp.Humanoid.Address + Offsets::Humanoid::JumpPower);
                originalUseJump = Driver->Read<bool> (lp.Humanoid.Address + Offsets::Humanoid::UseJumpPower);
                if (originalJump <= 0.f || originalJump > 1000.f) originalJump = 50.0f;
            } catch (...) { originalJump = 50.0f; originalUseJump = false; }
            lastHumanoid = lp.Humanoid.Address;
            wasEnabled = true;
        }

        wasActive = true;

        try {
            Driver->Write<float>(lp.Humanoid.Address + Offsets::Humanoid::JumpPower,   Globals::Movement::JumpPower_Value);
            Driver->Write<bool> (lp.Humanoid.Address + Offsets::Humanoid::UseJumpPower, true);
        } catch (...) {}

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

// ── Macro ─────────────────────────────────────────────────────────────────────

static void SendKey(WORD vk, bool down) {
    INPUT ip{};
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = vk;
    if (!down) ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));
}

static void SendMouseWheel(short delta) {
    INPUT ip{};
    ip.type = INPUT_MOUSE;
    ip.mi.dwFlags = MOUSEEVENTF_WHEEL;
    ip.mi.mouseData = static_cast<DWORD>(delta);
    SendInput(1, &ip, sizeof(INPUT));
}

static void MacroThread() {
    ImKeybindState kbState;
    bool wasActive = false;

    while (true) {
        bool enabled = Globals::Movement::Macro;

        bool active = false;
        if (enabled) {
            int vk = ImGuiKeyToVK(Globals::Movement::Macro_Key);
            active = kbState.Update(vk, Globals::Movement::Macro_Mode);
        } else {
            kbState.Reset();
        }

        bool shouldApply = enabled && active;

        if (!shouldApply) {
            wasActive = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        wasActive = true;

        try {
            int type = Globals::Movement::Macro_Type;
            if (type == 0) { // IOIOIO
                SendKey('I', true);  SendKey('I', false);
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                SendKey('O', true);  SendKey('O', false);
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            } else { // Mouse Wheel
                SendMouseWheel(120);   // wheel up
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                SendMouseWheel(-120);  // wheel down
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        } catch (...) {}

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

// ── Spinbot ────────────────────────────────────────────────────────────────────

static SDK::Matrix3 MakeRotY(float a) {
    float c = cosf(a), s = sinf(a);
    return { c, 0.f, -s, 0.f, 1.f, 0.f, s, 0.f, c };
}

static void SpinbotThread() {
    bool wasEnabled = false;
    static float angle = 0.f;

    while (true) {
        auto& lp = Globals::LocalPlayer;
        bool enabled = Globals::Movement::Spinbot;

        if (!enabled) {
            if (wasEnabled && ValidAddr(lp.Humanoid.Address) && ValidAddr(lp.HumanoidRootPart.Address)) {
                try {
                    SDK::Part hrp(lp.HumanoidRootPart.Address);
                    SDK::Part prim = hrp.Get_Primitive();
                    if (prim.Address) {
                        SDK::Matrix3 identity = { 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f };
                        prim.Set_Rotation(identity);
                    }
                    Driver->Write<uint8_t>(lp.Humanoid.Address + Offsets::Humanoid::AutoRotate, 1);
                } catch (...) {}
            }
            wasEnabled = false;
            angle = 0.f;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        if (!ValidAddr(lp.Humanoid.Address) || !ValidAddr(lp.HumanoidRootPart.Address)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        wasEnabled = true;

        int type = Globals::Movement::Spinbot_Type;
        float speed = Globals::Movement::Spinbot_Speed;
        angle += speed;
        if (angle > 6.283185f) angle -= 6.283185f;

        try {
            SDK::Part hrp(lp.HumanoidRootPart.Address);
            SDK::Part prim = hrp.Get_Primitive();
            if (prim.Address) {
                Driver->Write<uint8_t>(lp.Humanoid.Address + Offsets::Humanoid::AutoRotate, 0);

                SDK::Matrix3 rot;
                if (type == 0) {
                    rot = MakeRotY(angle);
                } else if (type == 1) {
                    float j = (fmodf(angle, 0.5f) < 0.25f) ? 0.f : 3.14159f;
                    rot = MakeRotY(j);
                } else if (type == 2) {
                    rot = MakeRotY(3.14159f);
                } else {
                    static float holdUntil = 0.f;
                    static float randomAngle = 0.f;
                    float now = GetTickCount() * 0.001f;
                    if (now >= holdUntil) {
                        randomAngle = static_cast<float>(rand()) / RAND_MAX * 6.283185f;
                        holdUntil = now + 0.3f + (static_cast<float>(rand()) / RAND_MAX * 0.5f);
                    }
                    rot = MakeRotY(randomAngle);
                }
                prim.Set_Rotation(rot);
            }
        } catch (...) {}

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Movement::RunService() {
    std::thread(FlyThread).detach();
    std::thread(SpeedThread).detach();
    std::thread(JumpPowerThread).detach();
    std::thread(MacroThread).detach();
    std::thread(SpinbotThread).detach();
}
