#include "Radar.h"
#include <cmath>
#include <algorithm>
#include <mutex>
#include <imgui/imgui.h>
#include <Globals.hxx>
#include <Engine/Engine.h>
#include <Core/Features/Cache/Cache.h>

namespace Radar {
    void Render() {
        if (!Globals::Radar::Enabled) return;
        if (Globals::Camera.Address == 0) return;

        ImDrawList* rdl = ImGui::GetBackgroundDrawList();
        float sz = Globals::Radar::Size;
        float range = Globals::Radar::Range;
        float half = sz * 0.5f;

        if (Globals::LocalPlayer.HumanoidRootPart.Address == 0) return;
        SDK::Part local_hrp(Globals::LocalPlayer.HumanoidRootPart.Address);
        SDK::Vector3 lp = local_hrp.Get_PartPosition();

        ImVec2 screen = ImGui::GetMainViewport()->Size;

        if (Globals::Radar::PosY < 0.f)
            Globals::Radar::PosY = screen.y - sz - 15.f;

        ImVec2 center(Globals::Radar::PosX + half, Globals::Radar::PosY + half);

        {
            ImVec2 tl(Globals::Radar::PosX, Globals::Radar::PosY);
            ImVec2 br(tl.x + sz, tl.y + sz);
            ImVec2 mp = ImGui::GetIO().MousePos;
            bool hovered = mp.x >= tl.x && mp.x <= br.x && mp.y >= tl.y && mp.y <= br.y;

            static bool dragging = false;
            static ImVec2 drag_offset = {0, 0};

            if (hovered && ImGui::GetIO().MouseClicked[0]) {
                dragging = true;
                drag_offset = ImVec2(mp.x - Globals::Radar::PosX, mp.y - Globals::Radar::PosY);
            }
            if (!ImGui::GetIO().MouseDown[0])
                dragging = false;
            if (dragging) {
                Globals::Radar::PosX = mp.x - drag_offset.x;
                Globals::Radar::PosY = mp.y - drag_offset.y;
                center = ImVec2(Globals::Radar::PosX + half, Globals::Radar::PosY + half);
            }
        }

        SDK::Camera cam(Globals::Camera.Address);
        SDK::Matrix3 cr = cam.Get_CameraRot();
        float cam_right_x = cr.data[0], cam_right_z = cr.data[6];
        float cam_look_x = -cr.data[2], cam_look_z = -cr.data[8];

        int bg_a = (int)(Globals::Radar::Opacity * 255);
        rdl->AddRectFilled(
            ImVec2(center.x - half, center.y - half),
            ImVec2(center.x + half, center.y + half),
            IM_COL32(10, 10, 15, bg_a));
        rdl->AddRect(
            ImVec2(center.x - half - 1, center.y - half - 1),
            ImVec2(center.x + half + 1, center.y + half + 1),
            IM_COL32(60, 60, 70, bg_a));

        rdl->AddLine(ImVec2(center.x, center.y - half), ImVec2(center.x, center.y + half), IM_COL32(50,50,60,bg_a/2));
        rdl->AddLine(ImVec2(center.x - half, center.y), ImVec2(center.x + half, center.y), IM_COL32(50,50,60,bg_a/2));

        rdl->PushClipRect(
            ImVec2(center.x - half, center.y - half),
            ImVec2(center.x + half, center.y + half), true);

        auto w2r = [&](float wx, float wz) -> ImVec2 {
            float dx = wx - lp.x;
            float dz = wz - lp.z;
            float rx = dx * cam_right_x + dz * cam_right_z;
            float ry = dx * cam_look_x + dz * cam_look_z;
            return ImVec2(center.x + rx / range * half,
                          center.y - ry / range * half);
        };

        int ef = Globals::Radar::EnemyFlags;
        {
            std::lock_guard<std::mutex> lock(Cache::Mutex);
            for (auto& player : Globals::Player_Cache) {
                if (player.Local_Player || !player.Character.Address || player.Health <= 0.f) continue;

                SDK::Vector3 ep{0,0,0};
                if (player.HumanoidRootPart.Address) {
                    SDK::Part hrp(player.HumanoidRootPart.Address);
                    ep = hrp.Get_PartPosition();
                } else if (player.Torso.Address) {
                    SDK::Part torso(player.Torso.Address);
                    ep = torso.Get_PartPosition();
                } else continue;

                float dx = ep.x - lp.x, dy = ep.y - lp.y, dz = ep.z - lp.z;
                ImVec2 dot = w2r(ep.x, ep.z);

                if (ef & 1) {
                    rdl->AddCircleFilled(dot, 3.5f, IM_COL32(255, 60, 60, 255));
                    rdl->AddCircle(dot, 3.5f, IM_COL32(0, 0, 0, 180));
                }

                float ly = dot.y + 6.f;
                if (ef & 2) {
                    ImVec2 ts = ImGui::CalcTextSize(player.Name.c_str());
                    float tx = dot.x - ts.x * 0.5f;
                    for (int ox=-1;ox<=1;ox++) for (int oy=-1;oy<=1;oy++) if(ox||oy)
                        rdl->AddText(ImVec2(tx+ox,ly+oy), IM_COL32(0,0,0,220), player.Name.c_str());
                    rdl->AddText(ImVec2(tx,ly), IM_COL32(255,255,255,255), player.Name.c_str());
                    ly += ImGui::GetFontSize() + 1.f;
                }
                if (ef & 4) {
                    float dist_m = sqrtf(dx*dx + dy*dy + dz*dz) * 0.28f;
                    char dist_buf[16]; sprintf_s(dist_buf, "%.0fm", dist_m);
                    ImVec2 ts = ImGui::CalcTextSize(dist_buf);
                    float tx = dot.x - ts.x * 0.5f;
                    for (int ox=-1;ox<=1;ox++) for (int oy=-1;oy<=1;oy++) if(ox||oy)
                        rdl->AddText(ImVec2(tx+ox,ly+oy), IM_COL32(0,0,0,220), dist_buf);
                    rdl->AddText(ImVec2(tx,ly), IM_COL32(180,180,180,255), dist_buf);
                }
            }
        }

        float ar = 9.f;
        ImVec2 tip (center.x,             center.y - ar);
        ImVec2 bll (center.x - ar*0.65f,  center.y + ar*0.6f);
        ImVec2 brr (center.x + ar*0.65f,  center.y + ar*0.6f);
        rdl->AddTriangleFilled(tip, bll, brr, IM_COL32(0, 200, 170, 255));
        rdl->AddTriangle(tip, bll, brr, IM_COL32(0, 0, 0, 200));

        rdl->PopClipRect();
    }
}
