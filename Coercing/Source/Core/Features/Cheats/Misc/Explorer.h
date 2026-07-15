#pragma once
#define NOMINMAX
#include <cmath>
#include <d3d11.h>
#include <wincodec.h>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <functional>

#pragma comment(lib, "Windowscodecs.lib")

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <Globals.hxx>
#include <Engine/Engine.h>
#include <Core/Graphics/Graphics.h>
#include <Core/Menu/Theme.h>
#include <Core/Features/Cheats/Misc/explorer_icons.h>

namespace ExtExplorer
{
    inline std::string ReadName(uintptr_t addr)
    {
        if (!addr) return "?";
        uintptr_t str_ptr = Driver->Read<uintptr_t>(addr + Offsets::Instance::Name);
        if (!str_ptr) return "?";
        return Driver->Read_String(str_ptr);
    }

    inline std::string ReadClass(uintptr_t addr)
    {
        if (!addr) return "?";
        uintptr_t desc = Driver->Read<uintptr_t>(addr + Offsets::Instance::ClassDescriptor);
        if (!desc) return "?";
        uintptr_t str_ptr = Driver->Read<uintptr_t>(desc + Offsets::Instance::ClassName);
        if (!str_ptr) return "?";
        return Driver->Read_String(str_ptr);
    }

    inline std::vector<uintptr_t> GetChildren(uintptr_t parent)
    {
        std::vector<uintptr_t> vec;
        if (!parent) return vec;
        uintptr_t child_ptr = Driver->Read<uintptr_t>(parent + Offsets::Instance::ChildrenStart);
        if (!child_ptr) return vec;
        uintptr_t start_addr = Driver->Read<uintptr_t>(child_ptr);
        uintptr_t end_addr = Driver->Read<uintptr_t>(child_ptr + Offsets::Instance::ChildrenEnd);
        if (!start_addr || !end_addr || end_addr <= start_addr) return vec;
        size_t diff = end_addr - start_addr;
        if (diff > 160000) return vec;
        vec.reserve(diff / 16);
        for (uintptr_t addr = start_addr; addr < end_addr; addr += 16) {
            uintptr_t child = Driver->Read<uintptr_t>(addr);
            if (child) vec.push_back(child);
        }
        return vec;
    }

    struct State
    {
        std::unordered_map<uintptr_t, std::vector<uintptr_t>> ChildCache;
        std::unordered_map<std::string, ID3D11ShaderResourceView*> IconCache;
        uintptr_t SelectedAddress = 0;
        bool scroll_to_selection = false;
        std::chrono::steady_clock::time_point LastRefresh;
    };
    inline State g_State;

    inline void RefreshCache()
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_State.LastRefresh).count();
        if (elapsed > 2000)
        {
            g_State.ChildCache.clear();
            g_State.LastRefresh = now;
        }
    }

    inline HRESULT CreateTextureFromMemory(
        ID3D11Device* Device,
        const uint8_t* Data, size_t DataSize,
        ID3D11ShaderResourceView** OutSrv)
    {
        if (!Device || !Data || !DataSize || !OutSrv) return E_INVALIDARG;

        IWICImagingFactory* factory = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&factory));
        if (FAILED(hr)) return hr;

        IWICStream* stream = nullptr;
        factory->CreateStream(&stream);
        stream->InitializeFromMemory(const_cast<uint8_t*>(Data), (DWORD)DataSize);

        IWICBitmapDecoder* decoder = nullptr;
        factory->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnLoad, &decoder);

        IWICBitmapFrameDecode* frame = nullptr;
        if (decoder) decoder->GetFrame(0, &frame);

        IWICFormatConverter* converter = nullptr;
        factory->CreateFormatConverter(&converter);
        if (converter && frame)
            converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA,
                WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);

        UINT w = 0, h = 0;
        if (converter) converter->GetSize(&w, &h);

        if (w > 0 && h > 0)
        {
            std::vector<uint8_t> pixels(w * h * 4);
            converter->CopyPixels(nullptr, w * 4, (UINT)pixels.size(), pixels.data());

            D3D11_TEXTURE2D_DESC td{};
            td.Width = w; td.Height = h; td.MipLevels = 1; td.ArraySize = 1;
            td.Format = DXGI_FORMAT_R8G8B8A8_UNORM; td.SampleDesc.Count = 1;
            td.Usage = D3D11_USAGE_DEFAULT; td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

            D3D11_SUBRESOURCE_DATA sd{}; sd.pSysMem = pixels.data(); sd.SysMemPitch = w * 4;
            ID3D11Texture2D* tex = nullptr;
            Device->CreateTexture2D(&td, &sd, &tex);

            if (tex)
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
                srvd.Format = td.Format; srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvd.Texture2D.MipLevels = 1;
                Device->CreateShaderResourceView(tex, &srvd, OutSrv);
                tex->Release();
            }
        }

        if (converter) converter->Release();
        if (frame) frame->Release();
        if (decoder) decoder->Release();
        if (stream) stream->Release();
        if (factory) factory->Release();

        return *OutSrv ? S_OK : E_FAIL;
    }

    inline ID3D11ShaderResourceView* GetIcon(const std::string& ClassName, ID3D11Device* Device)
    {
        auto it = g_State.IconCache.find(ClassName);
        if (it != g_State.IconCache.end())
            return it->second;

        static const std::map<std::string, const std::vector<uint8_t>*> IconMap = {
            { "Workspace",          &ExplorerIcons::workspace_image_data         },
            { "Part",               &ExplorerIcons::part_image_data              },
            { "MeshPart",           &ExplorerIcons::part_image_data              },
            { "UnionOperation",     &ExplorerIcons::part_image_data              },
            { "Model",              &ExplorerIcons::model_image_data             },
            { "Folder",             &ExplorerIcons::folder_image_data            },
            { "Camera",             &ExplorerIcons::camera_image_data            },
            { "Humanoid",           &ExplorerIcons::humanoid_image_data          },
            { "LocalScript",        &ExplorerIcons::local_script_image_data      },
            { "Script",             &ExplorerIcons::script_image_data            },
            { "ModuleScript",       &ExplorerIcons::module_script_image_data     },
            { "Players",            &ExplorerIcons::players_image_data           },
            { "Player",             &ExplorerIcons::player_image_data            },
            { "Sound",              &ExplorerIcons::sound_image_data             },
            { "Accessory",          &ExplorerIcons::accessory_image_data         },
            { "Hat",                &ExplorerIcons::hat_image_data               },
            { "ReplicatedStorage",  &ExplorerIcons::replicated_storage_image_data},
            { "ReplicatedFirst",    &ExplorerIcons::replicated_first_image_data  },
            { "RunService",         &ExplorerIcons::run_service_image_data       },
            { "SpawnLocation",      &ExplorerIcons::spawn_location_image_data    },
            { "StarterGui",         &ExplorerIcons::starter_gui_image_data       },
            { "StarterPack",        &ExplorerIcons::starter_pack_image_data      },
            { "StarterPlayer",      &ExplorerIcons::starter_player_image_data    },
            { "Stats",              &ExplorerIcons::stats_image_data             },
            { "Chat",               &ExplorerIcons::chat_image_data              },
            { "CoreGui",            &ExplorerIcons::core_gui_image_data          },
            { "GuiService",         &ExplorerIcons::gui_service_image_data       },
        };

        auto mapIt = IconMap.find(ClassName);
        if (mapIt != IconMap.end() && !mapIt->second->empty())
        {
            ID3D11ShaderResourceView* srv = nullptr;
            if (SUCCEEDED(CreateTextureFromMemory(Device,
                    mapIt->second->data(), mapIt->second->size(), &srv)))
            {
                g_State.IconCache[ClassName] = srv;
                return srv;
            }
        }

        g_State.IconCache[ClassName] = nullptr;
        return nullptr;
    }

    inline std::string GetFullPath(uintptr_t addr)
    {
        std::vector<std::string> parts;
        uintptr_t current = addr;
        uintptr_t dm = Globals::Datamodel.Address;

        while (current && current != dm)
        {
            try { parts.push_back(ReadName(current)); }
            catch (...) { break; }
            uintptr_t parent = Driver->Read<uintptr_t>(current + Offsets::Instance::Parent);
            if (!parent || parent == current) break;
            current = parent;
        }

        std::string result;
        for (int i = (int)parts.size() - 1; i >= 0; --i)
        {
            if (!result.empty()) result += ".";
            result += parts[i];
        }
        return result;
    }

    inline std::unordered_map<uintptr_t, float> g_NodeAnim;
    inline std::unordered_map<uintptr_t, bool>  g_NodeOpen;

    inline void ExpandToSelection(uintptr_t addr)
    {
        if (!addr) return;
        uintptr_t dm = Globals::Datamodel.Address;
        std::vector<uintptr_t> ancestors;
        uintptr_t current = Driver->Read<uintptr_t>(addr + Offsets::Instance::Parent);
        int safety = 64;
        while (current && current != dm && --safety > 0)
        {
            ancestors.push_back(current);
            uintptr_t parent = Driver->Read<uintptr_t>(current + Offsets::Instance::Parent);
            if (!parent || parent == current) break;
            current = parent;
        }
        for (uintptr_t anc : ancestors) {
            g_NodeOpen[anc] = true;
            g_NodeAnim[anc] = 1.f;
        }
        g_State.scroll_to_selection = true;
    }

    inline void DrawTree(uintptr_t addr, ID3D11Device* Device)
    {
        if (!addr) return;

        std::vector<uintptr_t> children;
        auto cacheIt = g_State.ChildCache.find(addr);
        if (cacheIt == g_State.ChildCache.end())
        {
            children = GetChildren(addr);
            g_State.ChildCache[addr] = children;
        }
        else
        {
            children = cacheIt->second;
        }

        std::string name = ReadName(addr);
        std::string cls = ReadClass(addr);

        ID3D11ShaderResourceView* icon = GetIcon(cls, Device);
        bool hasChildren = !children.empty();
        bool& logicOpen = g_NodeOpen[addr];
        float& animT = g_NodeAnim[addr];

        float dt = ImGui::GetIO().DeltaTime;
        float spd = 12.f;
        float target = logicOpen ? 1.f : 0.f;
        animT = animT + (target - animT) * ImMin(dt * spd, 1.f);

        float RowH = ImGui::GetFontSize() + ImGui::GetStyle().ItemSpacing.y;

        ImVec2 RowPos = ImGui::GetCursorScreenPos();
        float  AvailW = ImGui::GetContentRegionAvail().x;
        ImDrawList* DL = ImGui::GetWindowDrawList();

        bool hovered = ImGui::IsMouseHoveringRect(RowPos, ImVec2(RowPos.x + AvailW, RowPos.y + RowH));
        bool clicked = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

        ImU32 selBg = Menu::Theme::Accent(60);
        ImU32 hoverBg = Menu::Theme::Accent(30);

        if (g_State.SelectedAddress == addr)
        {
            DL->AddRectFilled(RowPos, ImVec2(RowPos.x + AvailW, RowPos.y + RowH), selBg);
            if (g_State.scroll_to_selection) {
                ImGui::SetScrollHereY(0.5f);
                g_State.scroll_to_selection = false;
            }
        }
        else if (hovered)
            DL->AddRectFilled(RowPos, ImVec2(RowPos.x + AvailW, RowPos.y + RowH), hoverBg);

        float CurX = RowPos.x + 2.f;
        float MidY = RowPos.y + RowH * 0.5f;

        if (hasChildren)
        {
            float angle = animT * (3.14159f * 0.5f);
            float arrowR = 4.f;
            auto rot = [&](float ax, float ay) -> ImVec2 {
                float rx = ax * cosf(angle) - ay * sinf(angle);
                float ry = ax * sinf(angle) + ay * cosf(angle);
                return ImVec2(CurX + arrowR + rx, MidY + ry);
            };
            ImU32 arrowCol = IM_COL32(180, 180, 180, (int)(255 * (0.5f + animT * 0.5f)));
            DL->AddTriangleFilled(
                rot(-arrowR * 0.6f, -arrowR * 0.8f),
                rot( arrowR * 0.9f,  0.f),
                rot(-arrowR * 0.6f,  arrowR * 0.8f),
                arrowCol);
        }
        CurX += 14.f;

        if (icon)
        {
            float iconAlpha = hasChildren ? 1.f : 0.55f;
            DL->AddImage((ImTextureID)(void*)icon,
                ImVec2(CurX, MidY - 8.f), ImVec2(CurX + 16.f, MidY + 8.f),
                ImVec2(0,0), ImVec2(1,1),
                IM_COL32(255, 255, 255, (int)(255 * iconAlpha)));
            CurX += 18.f;
        }

        ImU32 textCol = hasChildren ? Menu::Theme::Text() : Menu::Theme::MutedText(200);
        DL->AddText(ImVec2(CurX, MidY - ImGui::GetFontSize() * 0.5f), textCol, name.c_str());

        ImGui::Dummy(ImVec2(AvailW, RowH));

        if (clicked)
        {
            g_State.SelectedAddress = addr;
            if (hasChildren) logicOpen = !logicOpen;
        }

        ImGui::SetCursorScreenPos(RowPos);
        ImGui::InvisibleButton(("##ctx_" + std::to_string(addr)).c_str(), ImVec2(AvailW, RowH));
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
            ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 1.f);
            ImGui::PushStyleColor(ImGuiCol_PopupBg, Menu::Theme::SurfaceVec());
            ImGui::PushStyleColor(ImGuiCol_Border, Menu::Theme::SurfaceVec(0.4f));
            ImGui::PushStyleColor(ImGuiCol_Header, Menu::Theme::AccentVec(0.22f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Menu::Theme::AccentVec(0.32f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, Menu::Theme::AccentVec(0.42f));

            if (ImGui::BeginPopupContextItem())
            {
                if (Tahoma_BoldXP) ImGui::PushFont(Tahoma_BoldXP);
                ImGui::Text("Class: %s", cls.c_str());
                char addr_buf[64]; sprintf_s(addr_buf, "Address: %llX", (unsigned long long)addr);
                ImGui::Text("%s", addr_buf);
                if (Tahoma_BoldXP) ImGui::PopFont();

                std::string props_text;
                try {
                    char line[256];
                    ImGui::Separator();
                    ImGui::PushStyleColor(ImGuiCol_Text, Menu::Theme::AccentVec());
                    ImGui::Text("-- Properties --");
                    ImGui::PopStyleColor();

                    auto children_list = GetChildren(addr);
                    sprintf_s(line, "Children: %d", (int)children_list.size());
                    ImGui::Text("%s", line); props_text += line; props_text += "\n";

                    auto P = [&](const char* fmt, ...) {
                        va_list ap; va_start(ap, fmt);
                        char buf[512]; vsnprintf(buf, sizeof(buf), fmt, ap);
                        va_end(ap);
                        ImGui::Text("%s", buf);
                        props_text += buf; props_text += "\n";
                    };

                    if (cls == "Model")
                    {
                        uintptr_t pp = Driver->Read<uintptr_t>(addr + Offsets::Model::PrimaryPart);
                        P("PrimaryPart: %s", pp ? ReadName(pp).c_str() : "nil");
                        P("Scale: %.2f", Driver->Read<float>(addr + Offsets::Model::Scale));
                    }
                    else if (cls == "Humanoid")
                    {
                        auto xor_read = [&](uintptr_t off) -> float {
                            uint64_t p1 = Driver->Read<uint64_t>(addr + off);
                            uint64_t p2 = Driver->Read<uint64_t>(Driver->Read<uint64_t>(addr + off));
                            union { uint64_t hex; float f; } conv;
                            conv.hex = p1 ^ p2; return conv.f;
                        };
                        P("Health: %.1f / %.1f", xor_read(0x170), xor_read(0x178));
                        P("WalkSpeed: %.1f", Driver->Read<float>(addr + Offsets::Humanoid::Walkspeed));
                        P("JumpPower: %.1f  JumpHeight: %.1f", Driver->Read<float>(addr + Offsets::Humanoid::JumpPower), Driver->Read<float>(addr + Offsets::Humanoid::JumpHeight));
                        P("HipHeight: %.2f", Driver->Read<float>(addr + Offsets::Humanoid::HipHeight));
                        int rigtype = Driver->Read<int>(addr + Offsets::Humanoid::RigType);
                        P("RigType: %s (%d)", rigtype == 0 ? "R15" : "R6", rigtype);
                        uintptr_t dnPtr = Driver->Read<uintptr_t>(addr + Offsets::Humanoid::DisplayName);
                        if (dnPtr) P("DisplayName: %s", Driver->Read_String(dnPtr).c_str());
                    }
                    else if (cls == "Part" || cls == "MeshPart" || cls == "UnionOperation" || cls == "SpawnLocation" || cls == "Seat" || cls == "TrussPart" || cls == "WedgePart" || cls == "CornerWedgePart")
                    {
                        uintptr_t prim = Driver->Read<uintptr_t>(addr + Offsets::BasePart::Primitive);
                        if (prim) {
                            SDK::Vector3 pos = Driver->Read<SDK::Vector3>(prim + Offsets::Primitive::Position);
                            P("Position: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);
                            SDK::Vector3 sz = Driver->Read<SDK::Vector3>(prim + Offsets::Primitive::Size);
                            P("Size: %.1f, %.1f, %.1f", sz.x, sz.y, sz.z);
                            SDK::Vector3 vel = Driver->Read<SDK::Vector3>(prim + Offsets::Primitive::AssemblyLinearVelocity);
                            P("Velocity: %.1f, %.1f, %.1f", vel.x, vel.y, vel.z);
                            uint8_t flags = Driver->Read<uint8_t>(prim + Offsets::Primitive::Flags);
                            P("Anchored: %s  CanCollide: %s",
                                (flags & 2) ? "true" : "false",
                                (flags & 8) ? "true" : "false");
                        }
                        P("Transparency: %.2f", Driver->Read<float>(addr + Offsets::BasePart::Transparency));
                        P("Locked: %s", Driver->Read<uint8_t>(addr + Offsets::BasePart::Locked) ? "true" : "false");
                        if (cls == "Seat") {
                            uintptr_t occ = Driver->Read<uintptr_t>(addr + Offsets::Seat::Occupant);
                            P("Occupant: %s", occ ? ReadName(occ).c_str() : "nil");
                        }
                    }
                    else if (cls == "Camera")
                    {
                        SDK::Vector3 cpos = Driver->Read<SDK::Vector3>(addr + Offsets::Camera::Position);
                        P("Position: %.1f, %.1f, %.1f", cpos.x, cpos.y, cpos.z);
                        P("FieldOfView: %.1f", Driver->Read<float>(addr + Offsets::Camera::FieldOfView));
                        int ctype = Driver->Read<int>(addr + Offsets::Camera::CameraType);
                        const char* ctypes[] = {"Custom","Fixed","Attach","Track","Watch","Scriptable","Orbital"};
                        P("CameraType: %s (%d)", (ctype >= 0 && ctype < 7) ? ctypes[ctype] : "?", ctype);
                    }
                    else if (cls == "Player")
                    {
                        P("UserId: %lld", Driver->Read<int64_t>(addr + Offsets::Player::UserId));
                        P("AccountAge: %d days", Driver->Read<int>(addr + Offsets::Player::AccountAge));
                        uintptr_t team = Driver->Read<uintptr_t>(addr + Offsets::Player::Team);
                        P("Team: %s", team ? ReadName(team).c_str() : "nil");
                        uintptr_t chr = Driver->Read<uintptr_t>(addr + Offsets::Player::ModelInstance);
                        P("Character: %s", chr ? ReadName(chr).c_str() : "nil");
                    }
                    else if (cls == "Sound")
                    {
                        P("Volume: %.2f", Driver->Read<float>(addr + Offsets::Sound::Volume));
                        P("PlaybackSpeed: %.2f", Driver->Read<float>(addr + Offsets::Sound::PlaybackSpeed));
                        P("Looped: %s  Playing: %s",
                            Driver->Read<uint8_t>(addr + Offsets::Sound::Looped) ? "true" : "false",
                            Driver->Read<uint8_t>(addr + 0x139) ? "true" : "false");
                    }
                    else if (cls == "Lighting")
                    {
                        P("ClockTime: %.2f", Driver->Read<float>(addr + Offsets::Lighting::ClockTime));
                        P("Brightness: %.2f", Driver->Read<float>(addr + Offsets::Lighting::Brightness));
                        P("Fog: %.0f - %.0f", Driver->Read<float>(addr + Offsets::Lighting::FogStart), Driver->Read<float>(addr + Offsets::Lighting::FogEnd));
                        P("GlobalShadows: %s", Driver->Read<uint8_t>(addr + Offsets::Lighting::GlobalShadows) ? "true" : "false");
                    }
                    else if (cls == "Tool")
                    {
                        P("Enabled: %s", Driver->Read<uint8_t>(addr + Offsets::Tool::Enabled) ? "true" : "false");
                        P("CanBeDropped: %s", Driver->Read<uint8_t>(addr + Offsets::Tool::CanBeDropped) ? "true" : "false");
                        P("RequiresHandle: %s", Driver->Read<uint8_t>(addr + Offsets::Tool::RequiresHandle) ? "true" : "false");
                    }
                    else if (cls == "Script" || cls == "LocalScript" || cls == "ModuleScript")
                    {
                        uintptr_t guidPtr = Driver->Read<uintptr_t>(addr + Offsets::Script::GUID);
                        if (guidPtr) P("GUID: %s", Driver->Read_String(guidPtr).c_str());
                    }
                    else if (cls == "IntValue" || cls == "NumberValue") { P("Value: %.4g", Driver->Read<double>(addr + Offsets::Misc::Value)); }
                    else if (cls == "StringValue") { uintptr_t sp = Driver->Read<uintptr_t>(addr + Offsets::Misc::Value); if (sp) P("Value: %s", Driver->Read_String(sp).c_str()); }
                    else if (cls == "BoolValue") { P("Value: %s", Driver->Read<uint8_t>(addr + Offsets::Misc::Value) ? "true" : "false"); }
                    else if (cls == "ObjectValue") { uintptr_t ov = Driver->Read<uintptr_t>(addr + Offsets::Misc::Value); P("Value: %s", ov ? ReadName(ov).c_str() : "nil"); }
                    else if (cls == "VehicleSeat")
                    {
                        P("MaxSpeed: %.1f  Torque: %.1f", Driver->Read<float>(addr + Offsets::VehicleSeat::MaxSpeed), Driver->Read<float>(addr + Offsets::VehicleSeat::Torque));
                        P("TurnSpeed: %.1f", Driver->Read<float>(addr + Offsets::VehicleSeat::TurnSpeed));
                    }
                } catch (...) {}

                ImGui::Separator();
                if (ImGui::MenuItem("Copy All"))
                {
                    char hdr[256];
                    sprintf_s(hdr, "Name: %s\nClass: %s\nAddress: %llX\nPath: %s\n",
                        name.c_str(), cls.c_str(), (unsigned long long)addr, GetFullPath(addr).c_str());
                    std::string all = std::string(hdr) + props_text;
                    ImGui::SetClipboardText(all.c_str());
                }
                if (ImGui::MenuItem("Copy Full Path"))
                    ImGui::SetClipboardText(GetFullPath(addr).c_str());
                if (ImGui::MenuItem("Copy Address"))
                {
                    char buf[32]; sprintf_s(buf, "%llX", (unsigned long long)addr);
                    ImGui::SetClipboardText(buf);
                }
                if (ImGui::MenuItem("Copy Name"))  ImGui::SetClipboardText(name.c_str());
                if (ImGui::MenuItem("Copy Class")) ImGui::SetClipboardText(cls.c_str());
                ImGui::Separator();
                if (ImGui::MenuItem("Copy All Data (recursive)"))
                {
                    std::string dump;
                    std::function<void(uintptr_t, int)> dumpNode = [&](uintptr_t a, int depth) {
                        if (!a) return;
                        std::string indent(depth * 2, ' ');
                        std::string n = ReadName(a);
                        std::string c = ReadClass(a);

                        char hdr[256];
                        sprintf_s(hdr, "%s[%s] %s (0x%llX)\n", indent.c_str(), c.c_str(), n.c_str(), (unsigned long long)a);
                        dump += hdr;

                        if (c == "Part" || c == "MeshPart" || c == "UnionOperation" || c == "SpawnLocation" ||
                            c == "Seat" || c == "TrussPart" || c == "WedgePart" || c == "CornerWedgePart") {
                            try {
                                uintptr_t prim = Driver->Read<uintptr_t>(a + Offsets::BasePart::Primitive);
                                if (prim) {
                                    SDK::Vector3 pos = Driver->Read<SDK::Vector3>(prim + Offsets::Primitive::Position);
                                    SDK::Vector3 sz = Driver->Read<SDK::Vector3>(prim + Offsets::Primitive::Size);
                                    SDK::Matrix3 rot = Driver->Read<SDK::Matrix3>(prim + Offsets::Primitive::Rotation);
                                    char buf[512];
                                    sprintf_s(buf, "%s  Position: %.2f, %.2f, %.2f\n%s  Size: %.2f, %.2f, %.2f\n%s  Rotation: [%.3f,%.3f,%.3f | %.3f,%.3f,%.3f | %.3f,%.3f,%.3f]\n",
                                        indent.c_str(), pos.x, pos.y, pos.z,
                                        indent.c_str(), sz.x, sz.y, sz.z,
                                        indent.c_str(), rot.data[0],rot.data[1],rot.data[2], rot.data[3],rot.data[4],rot.data[5], rot.data[6],rot.data[7],rot.data[8]);
                                    dump += buf;
                                }
                            } catch (...) {}
                        }
                        else if (c == "Humanoid") {
                            try {
                                auto xr = [&](uintptr_t off) -> float {
                                    uint64_t p1 = Driver->Read<uint64_t>(a + off);
                                    uint64_t p2 = Driver->Read<uint64_t>(Driver->Read<uint64_t>(a + off));
                                    union { uint64_t h; float f; } cv; cv.h = p1 ^ p2; return cv.f;
                                };
                                char buf[256];
                                sprintf_s(buf, "%s  Health: %.1f/%.1f  WalkSpeed: %.1f  JumpPower: %.1f\n",
                                    indent.c_str(), xr(0x170), xr(0x178),
                                    Driver->Read<float>(a + Offsets::Humanoid::Walkspeed),
                                    Driver->Read<float>(a + Offsets::Humanoid::JumpPower));
                                dump += buf;
                            } catch (...) {}
                        }
                        else if (c == "IntValue" || c == "NumberValue") {
                            try { char buf[128]; sprintf_s(buf, "%s  Value: %.4g\n", indent.c_str(), Driver->Read<double>(a + Offsets::Misc::Value)); dump += buf; } catch (...) {}
                        }
                        else if (c == "BoolValue") {
                            try { char buf[128]; sprintf_s(buf, "%s  Value: %s\n", indent.c_str(), Driver->Read<uint8_t>(a + Offsets::Misc::Value) ? "true" : "false"); dump += buf; } catch (...) {}
                        }
                        else if (c == "StringValue") {
                            try { uintptr_t sp = Driver->Read<uintptr_t>(a + Offsets::Misc::Value); if (sp) { char buf[256]; sprintf_s(buf, "%s  Value: %s\n", indent.c_str(), Driver->Read_String(sp).c_str()); dump += buf; } } catch (...) {}
                        }

                        auto ch = GetChildren(a);
                        for (uintptr_t child : ch)
                            dumpNode(child, depth + 1);
                    };

                    dumpNode(addr, 0);
                    ImGui::SetClipboardText(dump.c_str());
                }
                ImGui::EndPopup();
            }
            ImGui::PopStyleColor(5);
            ImGui::PopStyleVar(3);
        }

        if (animT > 0.001f && hasChildren)
        {
            ImGui::Indent(14.f);

            static std::unordered_map<uintptr_t, float> s_childH;
            float& lastH = s_childH[addr];
            ImVec2 ClipStart = ImGui::GetCursorScreenPos();

            float clipH = (lastH > 0.f) ? lastH * animT : 9999.f;
            ImGui::PushClipRect(ClipStart,
                ImVec2(ClipStart.x + 9999.f, ClipStart.y + clipH), true);

            float beforeY = ClipStart.y;
            for (const auto& child : children)
                DrawTree(child, Device);
            float afterY = ImGui::GetCursorScreenPos().y;
            float measuredH = afterY - beforeY;

            ImGui::PopClipRect();

            if (measuredH > 0.f)
                lastH = measuredH;

            if (animT < 0.999f && lastH > 0.f)
            {
                ImGui::SetCursorScreenPos(ImVec2(ClipStart.x, ClipStart.y + lastH * animT));
                ImGui::Dummy(ImVec2(0, 0));
            }

            ImGui::Unindent(14.f);
        }
    }

    inline void DrawTreeFiltered(uintptr_t addr, ID3D11Device* Device, const char* Filter)
    {
        if (!addr) return;
        if (Filter && Filter[0] != '\0')
        {
            std::vector<uintptr_t> children;
            auto cacheIt = g_State.ChildCache.find(addr);
            if (cacheIt == g_State.ChildCache.end())
            {
                children = GetChildren(addr);
                g_State.ChildCache[addr] = children;
            }
            else children = cacheIt->second;

            std::string name = ReadName(addr);
            std::string cls = ReadClass(addr);

            bool nameMatch = name.find(Filter) != std::string::npos;
            bool classMatch = cls.find(Filter) != std::string::npos;

            if (nameMatch || classMatch)
            {
                ID3D11ShaderResourceView* icon = GetIcon(cls, Device);
                if (icon)
                {
                    ImGui::Image((ImTextureID)(void*)icon, ImVec2(16.f, 16.f));
                    ImGui::SameLine();
                }

                std::string label = name + " [" + cls + "]##" + std::to_string(addr);
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth;
                if (g_State.SelectedAddress == addr) flags |= ImGuiTreeNodeFlags_Selected;

                ImGui::TreeNodeEx(label.c_str(), flags);
                if (ImGui::IsItemClicked()) g_State.SelectedAddress = addr;
                ImGui::TreePop();
            }

            for (const auto& child : children)
                DrawTreeFiltered(child, Device, Filter);
        }
        else
        {
            DrawTree(addr, Device);
        }
    }

    inline void RenderContent(ID3D11Device* Device)
    {
        if (!Device) return;
        RefreshCache();

        static char SearchBuf[128] = {};
        float SearchW = ImGui::GetContentRegionAvail().x;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.f, 3.f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, Menu::Theme::ControlVec());
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, Menu::Theme::SurfaceVec());
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, Menu::Theme::SurfaceVec());
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(Menu::Theme::Text()));
        ImGui::SetNextItemWidth(SearchW);
        ImGui::InputTextWithHint("##exp_search", "search...", SearchBuf, sizeof(SearchBuf));
        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(1);

        float treeH = ImGui::GetContentRegionAvail().y;
        if (ImGui::BeginChild("##exp_tree", ImVec2(0, treeH), false, 0))
        {
            uintptr_t dm = Globals::Datamodel.Address;
            if (dm)
                DrawTreeFiltered(dm, Device, SearchBuf);
            else
                ImGui::TextDisabled("DataModel not found.");
        }
        ImGui::EndChild();
    }

    inline void RenderWindow(ID3D11Device* Device)
    {
        if (!Device || !Globals::Misc::Explorer) return;

        // ── Fade animation (same rate as main menu in Menu.cpp) ──
        static float expAlpha = 0.f;
        float dt = ImGui::GetIO().DeltaTime;
        float rate = g_MenuOpen ? 6.0f : 7.5f;
        expAlpha += ((g_MenuOpen ? 1.0f : 0.0f) - expAlpha) * (1.0f - std::exp(-rate * dt));
        if (expAlpha > 0.999f) expAlpha = 1.0f;
        if (expAlpha < 0.001f && !g_MenuOpen) return;

        ImGuiStyle& style = ImGui::GetStyle();
        float savedAlpha = style.Alpha;
        style.Alpha = savedAlpha * expAlpha;
        Menu::Theme::SetMenuAlpha(expAlpha);

        // ── Sizing & positioning ──
        static float w = 420.f, h = 540.f;
        ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(120, 120), ImGuiCond_FirstUseEver);

        // ── Window flags (same as main menu) ──
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(Menu::Theme::Text()));

        ImGui::Begin("##Explorer", &Globals::Misc::Explorer, flags);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 wp = ImGui::GetWindowPos();
        w = ImGui::GetWindowSize().x;
        h = ImGui::GetWindowSize().y;

        // ── Checkered grid (exactly like main menu) ──
        for (float x = wp.x + 12.f; x <= wp.x + w - 12.f; x += 48.f)
            dl->AddLine(ImVec2(x, wp.y), ImVec2(x, wp.y + h), Menu::Theme::Outline(72));
        for (float y = wp.y + 12.f; y <= wp.y + h - 12.f; y += 48.f)
            dl->AddLine(ImVec2(wp.x, y), ImVec2(wp.x + w, y), Menu::Theme::Outline(72));

        // ── Layout measurements (matching main menu) ──
        const float sepY = 32.f;
        const float sepH = 1.f;
        const float footH = 26.f;
        const float cTop = sepY + sepH + 12.f;
        const float pW = w - 32.f;
        const float pH = h - footH - cTop - 8.f;

        // ── Separator line (like main menu's breadcrumb background) ──
        dl->AddRectFilled(ImVec2(wp.x, wp.y + sepY), ImVec2(wp.x + w, wp.y + sepY + sepH), Menu::Theme::SurfaceLight(238));

        // ── Footer ──
        dl->AddRectFilled(ImVec2(wp.x, wp.y + h - footH), ImVec2(wp.x + w, wp.y + h), Menu::Theme::SurfaceLight());
        ImVec2 fts = ImGui::CalcTextSize("Dex Explorer");
        dl->AddText(ImVec2(wp.x + 16, wp.y + h - footH + (footH - fts.y) * 0.5f), Menu::Theme::MutedText(), "Dex Explorer");

        // ── Title ──
        dl->AddText(ImVec2(wp.x + 16, wp.y + 12), Menu::Theme::Text(), "Explorer");

        // ── Close button ──
        const char* closeIcon = (const char*)u8"\u2715";
        ImVec2 ciSize = ImGui::CalcTextSize(closeIcon);
        float closeX = wp.x + w - 20.f - ciSize.x;
        float closeY = wp.y + 10.f + (18.f - ciSize.y) * 0.5f;
        bool closeHov = ImGui::IsMouseHoveringRect(
            ImVec2(closeX - 4.f, closeY - 4.f),
            ImVec2(closeX + ciSize.x + 4.f, closeY + ciSize.y + 4.f));
        ImU32 closeCol = closeHov ? Menu::Theme::Accent() : Menu::Theme::MutedText(180);
        dl->AddText(ImVec2(closeX, closeY), closeCol, closeIcon);
        if (closeHov && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            Globals::Misc::Explorer = false;

        // ── Content panel (replicating main menu's panel style) ──
        ImVec2 panelPos(wp.x + 16, wp.y + cTop);
        ImVec2 panelSize(pW, pH);

        // Title bar for the content panel
        dl->AddRectFilled(panelPos, ImVec2(panelPos.x + panelSize.x, panelPos.y + 30.f), Menu::Theme::Surface(255), 4.f);
        dl->AddRect(panelPos, ImVec2(panelPos.x + panelSize.x, panelPos.y + panelSize.y), Menu::Theme::Outline(180), 4.f);
        dl->AddLine(ImVec2(panelPos.x, panelPos.y + 30.f), ImVec2(panelPos.x + panelSize.x, panelPos.y + 30.f), Menu::Theme::Outline(145), 1.f);
        dl->AddText(ImVec2(panelPos.x + 12.f, panelPos.y + 8.f), Menu::Theme::Text(), "Instance Tree");

        ImGui::SetCursorScreenPos(ImVec2(panelPos.x + 10.f, panelPos.y + 36.f));
        ImGui::BeginChild("##exp_tree_area", ImVec2(panelSize.x - 20.f, panelSize.y - 44.f), false,
            ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoBackground);

        RenderContent(Device);

        ImGui::EndChild();

        ImGui::End();
        ImGui::PopStyleColor(1);

        style.Alpha = savedAlpha;
        Menu::Theme::SetMenuAlpha(1.0f);
    }
}
