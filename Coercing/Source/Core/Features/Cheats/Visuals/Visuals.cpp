#define IMGUI_DEFINE_MATH_OPERATORS
#include <iostream>
#include <format>
#include "visuals.h"
#include <Engine/Engine.h>
#include <cfloat>
#include <imgui/imgui.h>
#include <globals.hxx>
#include <Windows.h>
#include <algorithm>
#include <mutex>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#include <ImGui/imgui_internal.h>
#include "../../../Graphics/Graphics.h"
#include <unordered_map>
#include <cstring>
#include <cstdlib>
#include <thread>
#include "FriendCheck.h"
#include "Radar/Radar.h"
#include <thread>
#include <Core/Menu/Theme.h>

#define FlotationDevice(c) ImGui::ColorConvertFloat4ToU32(ImVec4(c[0], c[1], c[2], c[3]))

static void Outline(const ImVec2& Pos, const char* Text, const float Col[4]) {

    if (!Text || !*Text)
        return;

    ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    ImDrawList* Draw = ImGui::GetBackgroundDrawList();
    Draw->Flags &= ~ImDrawListFlags_AntiAliasedLines;

    ImGui::PushFont(Tahoma_BoldXP);

    const ImVec2 Position(std::roundf(Pos.x), std::roundf(Pos.y));

    const ImU32 Col1 = ImGui::ColorConvertFloat4ToU32(ImVec4(Col[0], Col[1], Col[2], Col[3]));
    const ImU32 Col2 = IM_COL32(0, 0, 0, 255);

    static constexpr ImVec2 Offsets[8] = {
        {-1.f,  0.f}, {1.f,  0.f},
        { 0.f, -1.f}, {0.f,  1.f},
        {-1.f, -1.f}, {1.f, -1.f},
        {-1.f,  1.f}, {1.f,  1.f}
    };

    const float Font_Size = ImGui::GetFontSize();

    for (const ImVec2& o : Offsets) {

        Draw->AddText(nullptr, Font_Size, Position + o, Col2, Text);
    }

    Draw->AddText(nullptr, Font_Size, Position, Col1, Text);

    ImGui::PopFont();
}

static const SDK::Vector3 Corners[8] = {

    {-1,-1,-1}, {1,-1,-1}, {-1,1,-1}, {1,1,-1},
    {-1,-1, 1}, {1,-1, 1}, {-1,1, 1}, {1,1, 1}
};

// ── Mesh chams (download .mesh files from assetdelivery, parse + render) ──
struct CachedMesh {
    std::vector<float> pos;
    std::vector<uint32_t> idx;
};

static std::unordered_map<std::string, CachedMesh> s_meshCache;
static std::mutex s_meshMutex;

static bool FetchAsset(const std::wstring& host, const std::wstring& path, std::vector<uint8_t>& out) {
    out.clear();
    HINTERNET hs = WinHttpOpen(L" Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hs) return false;
    HINTERNET hc = WinHttpConnect(hs, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hc) { WinHttpCloseHandle(hs); return false; }
    HINTERNET hr = WinHttpOpenRequest(hc, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hr) { WinHttpCloseHandle(hc); WinHttpCloseHandle(hs); return false; }
    DWORD dec = WINHTTP_DECOMPRESSION_FLAG_ALL;
    WinHttpSetOption(hr, WINHTTP_OPTION_DECOMPRESSION, &dec, sizeof(dec));
    DWORD rp = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
    WinHttpSetOption(hr, WINHTTP_OPTION_REDIRECT_POLICY, &rp, sizeof(rp));
    bool ok = false;
    if (WinHttpSendRequest(hr, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
        WinHttpReceiveResponse(hr, nullptr)) {
        out.reserve(65536);
        uint8_t buf[8192]; DWORD got;
        while (WinHttpReadData(hr, buf, sizeof(buf), &got) && got > 0)
            out.insert(out.end(), buf, buf + got);
        ok = !out.empty();
    }
    WinHttpCloseHandle(hr); WinHttpCloseHandle(hc); WinHttpCloseHandle(hs);
    return ok;
}

static bool ParseMeshData(const std::vector<uint8_t>& data, std::vector<float>& outPos, std::vector<uint32_t>& outIdx) {
    outPos.clear(); outIdx.clear();
    if (data.size() < 13) return false;
    auto starts = [&](const char* s) { size_t n = strlen(s); return data.size() >= n && memcmp(data.data(), s, n) == 0; };
    // version 1.00 / 1.01 text format
    if (starts("version 1.")) {
        const char* p = (const char*)data.data();
        const char* end = p + data.size();
        const char* nl = (const char*)memchr(p, '\n', data.size());
        if (!nl) return false; p = nl + 1;
        while (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++;
        uint32_t fc = 0;
        while (p < end && *p >= '0' && *p <= '9') { fc = fc * 10 + (*p - '0'); p++; }
        if (fc == 0 || fc > 500000) return false;
        float scale = (data.size() >= 12 && data[9] == '0') ? 2.f : 1.f;
        outPos.reserve(fc * 9); outIdx.reserve(fc * 3);
        auto skip = [&]() { while (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++; };
        auto chr = [&](char c) { skip(); if (p < end && *p == c) { p++; return true; } return false; };
        auto flt = [&](float& f) { skip(); char* ep = nullptr; f = strtof(p, &ep); if (!ep || ep == p) return false; p = ep; return true; };
        auto tri = [&](float v[3]) { return chr('[') && flt(v[0]) && chr(',') && flt(v[1]) && chr(',') && flt(v[2]) && chr(']'); };
        for (uint32_t f = 0; f < fc; f++) {
            for (int v = 0; v < 3; v++) {
                float pos[3], norm[3], uv[2];
                if (!tri(pos) || !tri(norm) || !chr(',') || !flt(uv[0]) || !chr(',') || !flt(uv[1])) return false;
                outPos.push_back(pos[0] * scale); outPos.push_back(pos[1] * scale); outPos.push_back(pos[2] * scale);
                outIdx.push_back((uint32_t)(outPos.size() / 3) - 1);
            }
        }
        return true;
    }
    // binary formats v2.00 - v5.00
    const uint8_t* bp = data.data();
    const uint8_t* be = data.data() + data.size();
    while (bp < be && *bp != '\n') bp++;
    if (bp >= be) return false;
    bp++;
    // read header
    uint16_t hdrSz, vertSz, faceSz, lodKind, numLods;
    uint32_t numVerts, numFaces;
    auto r16 = [&]() -> uint16_t { if (bp + 2 > be) return 0; uint16_t v; memcpy(&v, bp, 2); bp += 2; return v; };
    auto r32 = [&]() -> uint32_t { if (bp + 4 > be) return 0; uint32_t v; memcpy(&v, bp, 4); bp += 4; return v; };
    auto r8 = [&]() -> uint8_t { if (bp >= be) return 0; return *bp++; };
    hdrSz = r16(); vertSz = r8(); faceSz = r8();
    if (starts("version 2.")) {
        numVerts = r32(); numFaces = r32();
    } else {
        lodKind = r16(); numLods = r16(); numVerts = r32(); numFaces = r32();
    }
    if (vertSz < 12 || faceSz != 12 || numVerts == 0 || numVerts > 500000 || numFaces == 0 || numFaces > 500000) return false;
    size_t consumed = (starts("version 2.")) ? 12 : 16;
    if (hdrSz > consumed) bp += (hdrSz - consumed);
    outPos.resize(numVerts * 3);
    for (uint32_t i = 0; i < numVerts; i++) {
        if (bp + vertSz > be) return false;
        float p[3]; memcpy(p, bp, 12);
        outPos[i * 3] = p[0]; outPos[i * 3 + 1] = p[1]; outPos[i * 3 + 2] = p[2];
        bp += vertSz;
    }
    outIdx.resize(numFaces * 3);
    for (uint32_t i = 0; i < numFaces; i++) {
        if (bp + 12 > be) return false;
        uint32_t tri[3]; memcpy(tri, bp, 12);
        if (tri[0] >= numVerts || tri[1] >= numVerts || tri[2] >= numVerts) return false;
        outIdx[i * 3] = tri[0]; outIdx[i * 3 + 1] = tri[1]; outIdx[i * 3 + 2] = tri[2];
        bp += 12;
    }
    return true;
}

static std::string NormalizeAssetId(const std::string& raw) {
    std::string digits;
    for (char c : raw) if (c >= '0' && c <= '9') digits.push_back(c);
    // Known built-in mesh asset IDs
    static const std::unordered_map<std::string, std::string> known = {
        {"fonts/head", "12562046409"},
        {"fonts/upper torso", "12562024203"}, {"fonts/upperTorso", "12562024203"},
        {"fonts/lower torso", "12562016977"}, {"fonts/lowerTorso", "12562016977"},
        {"fonts/torso", "12562024203"},
        {"fonts/left arm", "12562037521"}, {"fonts/leftArm", "12562037521"},
        {"fonts/right arm", "12562031894"}, {"fonts/rightArm", "12562031894"},
        {"fonts/left leg", "12562042966"}, {"fonts/leftLeg", "12562042966"},
        {"fonts/right leg", "12562044662"}, {"fonts/rightLeg", "12562044662"},
        {"fonts/left foot", "12561990782"}, {"fonts/leftFoot", "12561990782"},
        {"fonts/right foot", "12562012668"}, {"fonts/rightFoot", "12562012668"},
        {"fonts/left hand", "12561998431"}, {"fonts/leftHand", "12561998431"},
        {"fonts/right hand", "12562007254"}, {"fonts/rightHand", "12562007254"},
    };
    std::string lower = raw;
    for (auto& c : lower) c = (char)tolower(c);
    for (auto& [key, id] : known)
        if (lower.find(key) != std::string::npos) return id;
    if (digits.size() >= 5 && digits.size() <= 24) return digits;
    return {};
}

static bool GetOrLoadMesh(const std::string& assetId, CachedMesh*& out) {
    std::lock_guard<std::mutex> lock(s_meshMutex);
    auto it = s_meshCache.find(assetId);
    if (it != s_meshCache.end()) { out = &it->second; return true; }
    std::vector<uint8_t> raw;
    std::wstring path = L"/v1/asset/?id=";
    for (char c : assetId) path.push_back((wchar_t)c);
    if (FetchAsset(L"assetdelivery.roblox.com", path, raw) || FetchAsset(L"assetdelivery.roproxy.com", path, raw)) {
        CachedMesh cm;
        if (ParseMeshData(raw, cm.pos, cm.idx)) {
            auto r = s_meshCache.emplace(assetId, std::move(cm));
            out = &r.first->second;
            return true;
        }
    }
    return false;
}

static std::string GetMeshIdFromPart(const SDK::Part& part) {
    uintptr_t addr = part.Address;
    if (!addr) return {};
    std::string cls = SDK::Instance(addr).Class();
    if (cls == "MeshPart") {
        std::string sid = Driver->Read_String(addr + Offsets::MeshPart::MeshId);
        if (!sid.empty()) return sid;
    }
    SDK::Instance child(addr);
    auto children = child.Children();
    for (auto& c : children) {
        if (c.Class() == "SpecialMesh") {
            std::string sid = Driver->Read_String(c.Address + Offsets::SpecialMesh::MeshId);
            if (!sid.empty()) return sid;
        }
    }
    return {};
}

static void RenderPartMesh(ImDrawList* Draw, const SDK::Vector3& pos, const SDK::Matrix3& rot, const std::string& meshId, ImU32 fillCol) {
    std::string aid = NormalizeAssetId(meshId);
    if (aid.empty()) return;
    CachedMesh* cm = nullptr;
    if (!GetOrLoadMesh(aid, cm)) return;
    if (cm->idx.empty() || cm->pos.empty()) return;
    size_t vertCount = cm->pos.size() / 3;
    size_t idxCount = cm->idx.size();
    if (vertCount > 65535) return;
    Draw->PrimReserve((int)idxCount, (int)vertCount);
    ImDrawVert* vtx = Draw->_VtxWritePtr;
    ImDrawIdx* idx = Draw->_IdxWritePtr;
    for (size_t i = 0; i < vertCount; i++) {
        SDK::Vector3 local = { cm->pos[i * 3], cm->pos[i * 3 + 1], cm->pos[i * 3 + 2] };
        SDK::Vector3 world = pos + rot * local;
        auto ss = Globals::VisualEngine.World_To_Screen(world);
        vtx[i].pos.x = ss.x;
        vtx[i].pos.y = ss.y;
        vtx[i].uv.x = 0.f; vtx[i].uv.y = 0.f;
        vtx[i].col = fillCol;
    }
    for (size_t i = 0; i < idxCount; i++)
        idx[i] = (ImDrawIdx)cm->idx[i];
    Draw->_VtxWritePtr += vertCount;
    Draw->_IdxWritePtr += idxCount;
}

static void FallbackRenderPart(ImDrawList* Draw, const SDK::Vector3& pos, const SDK::Vector3& sz, const SDK::Matrix3& rot, ImU32 fillCol, ImU32 outCol) {
    SDK::Vector3 half = { sz.x * 0.5f, sz.y * 0.5f, sz.z * 0.5f };
    SDK::Vector3 local[8] = {
        {-half.x, -half.y, -half.z}, {-half.x, -half.y,  half.z},
        {-half.x,  half.y, -half.z}, {-half.x,  half.y,  half.z},
        { half.x, -half.y, -half.z}, { half.x, -half.y,  half.z},
        { half.x,  half.y, -half.z}, { half.x,  half.y,  half.z},
    };
    float minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
    int valid = 0;
    for (int i = 0; i < 8; i++) {
        SDK::Vector3 world = pos + rot * local[i];
        auto ss = Globals::VisualEngine.World_To_Screen(world);
        if (ss.x > 0 && ss.y > 0) {
            if (ss.x < minX) minX = ss.x;
            if (ss.y < minY) minY = ss.y;
            if (ss.x > maxX) maxX = ss.x;
            if (ss.y > maxY) maxY = ss.y;
            valid++;
        }
    }
    if (valid < 3) return;
    Draw->AddRectFilled(ImVec2(minX, minY), ImVec2(maxX, maxY), fillCol);
    Draw->AddRect(ImVec2(minX, minY), ImVec2(maxX, maxY), outCol);
}

struct HitmarkerEvent { float time; SDK::Vector3 world_pos; };
struct GhostSnapshot {
    float time;
    std::unordered_map<std::string, SDK::Vector3> positions;
    std::unordered_map<std::string, SDK::Vector3> sizes;
    std::unordered_map<std::string, SDK::Matrix3> rotations;
    std::unordered_map<std::string, std::string> meshIds;
};
struct HitNotification { float time; std::string target_name; float damage; std::string hit_part; };
struct ShotTracer { float time; SDK::Vector3 origin; SDK::Vector3 direction; float dist; bool finished; };

namespace Visuals {
    std::vector<const SDK::Instance*> Get_Bones(const SDK::Player& Player) {

        std::vector<const SDK::Instance*> Parts;

        const bool R15 = Player.UpperTorso.Address && Player.LowerTorso.Address;
        const bool R6 = Player.Torso.Address;

        if (R15) {

            if (Player.Head.Address) Parts.push_back(&Player.Head);

            if (Player.UpperTorso.Address) Parts.push_back(&Player.UpperTorso);
            if (Player.LowerTorso.Address) Parts.push_back(&Player.LowerTorso);

            if (Player.LeftUpperArm.Address) Parts.push_back(&Player.LeftUpperArm);
            if (Player.LeftLowerArm.Address) Parts.push_back(&Player.LeftLowerArm);
            if (Player.LeftHand.Address)     Parts.push_back(&Player.LeftHand);

            if (Player.RightUpperArm.Address) Parts.push_back(&Player.RightUpperArm);
            if (Player.RightLowerArm.Address) Parts.push_back(&Player.RightLowerArm);
            if (Player.RightHand.Address)     Parts.push_back(&Player.RightHand);

            if (Player.LeftUpperLeg.Address) Parts.push_back(&Player.LeftUpperLeg);
            if (Player.LeftLowerLeg.Address) Parts.push_back(&Player.LeftLowerLeg);
            if (Player.LeftFoot.Address)     Parts.push_back(&Player.LeftFoot);

            if (Player.RightUpperLeg.Address) Parts.push_back(&Player.RightUpperLeg);
            if (Player.RightLowerLeg.Address) Parts.push_back(&Player.RightLowerLeg);
            if (Player.RightFoot.Address)     Parts.push_back(&Player.RightFoot);
        }
        else if (R6) {

            if (Player.Head.Address)  Parts.push_back(&Player.Head);
            if (Player.Torso.Address) Parts.push_back(&Player.Torso);

            if (Player.LeftArm.Address)  Parts.push_back(&Player.LeftArm);
            if (Player.RightArm.Address) Parts.push_back(&Player.RightArm);

            if (Player.LeftLeg.Address)  Parts.push_back(&Player.LeftLeg);
            if (Player.RightLeg.Address) Parts.push_back(&Player.RightLeg);
        }
        else {

            for (const auto& Bone : Player.Bones) {

                if (Bone.Address)
                    Parts.push_back(&Bone);
            }

            if (Parts.empty()) {

                if (Player.HumanoidRootPart.Address) Parts.push_back(&Player.HumanoidRootPart);
                if (Player.Head.Address)             Parts.push_back(&Player.Head);
                if (Player.Torso.Address)            Parts.push_back(&Player.Torso);
                if (Player.UpperTorso.Address)       Parts.push_back(&Player.UpperTorso);
                if (Player.LowerTorso.Address)       Parts.push_back(&Player.LowerTorso);
            }
        }
        return Parts;
    }

    void RunService()
    {
        if (Globals::VisualEngine.Address == 0 || Globals::Datamodel.Address == 0)
            return;

        ImDrawList* Draw = ImGui::GetBackgroundDrawList();
        ImDrawListFlags drawFlags = Draw->Flags;
        Draw->Flags &= ~ImDrawListFlags_AntiAliasedLines;

        std::vector<SDK::Player> Snapshot;
        {
            std::lock_guard<std::mutex> lock(Cache::Mutex);
            Snapshot = Globals::Player_Cache;
        }
        const auto& Local = Globals::LocalPlayer;

        for (auto& Player : Snapshot)
        {
            if (!Globals::Visuals::Enabled || !Player.Character.Address)
                continue;

            if (Globals::Visuals::Render_Distance > 0.f && Player.Distance > Globals::Visuals::Render_Distance)
                continue;

            bool isFriend = Globals::Settings::FriendCheck && GetFriendStatus(static_cast<int64_t>(Player.UserID)) == FriendStatus::Friends;

            SDK::Part Head(Player.Head.Address);
            if (!Head.Address) continue;

            SDK::Part HeadPrim = Head.Get_Primitive();
            if (!HeadPrim.Address) continue;
            auto Head_W2S = Globals::VisualEngine.World_To_Screen(HeadPrim.Get_Position());
            if (Head_W2S.x <= 0.f || Head_W2S.y <= 0.f)
                continue;

            float Left = FLT_MAX, Top = FLT_MAX, Right = -FLT_MAX, Bottom = -FLT_MAX;
            bool Valid = false;

            auto Bones = Visuals::Get_Bones(Player);
            if (Bones.empty()) continue;

            for (auto* Inst : Bones) {
                if (!Inst || !Inst->Address) continue;

                const auto Part = SDK::Part(Inst->Address);
                const auto Primitive = Part.Get_Primitive();
                if (!Primitive.Address) continue;

                SDK::Vector3 Size = Primitive.Get_Size();
                const auto Position = Primitive.Get_Position();
                const auto Rotation = Primitive.Get_Rotation();

                if (Globals::GameID == 292439477)
                {
                    std::string Name = Part.Name();

                    if (Name.find("Other_") != std::string::npos) {
                        Size = { 1.f, 2.f, 1.f };
                    }
                    else if (Name == "Head") {
                        Size = { 1.f, 1.f, 1.f };
                    }
                    else if (Name == "Torso") {
                        Size = { 2.f, 2.f, 1.f };
                    }
                }

                if (Size.x == 0.f && Size.y == 0.f && Size.z == 0.f)
                    continue;

                for (const auto& LocalCorners : Corners) {
                    SDK::Vector3 Offset{
                        LocalCorners.x * Size.x * 0.5f,
                        LocalCorners.y * Size.y * 0.5f,
                        LocalCorners.z * Size.z * 0.5f
                    };

                    SDK::Vector3 World = Position + Rotation * Offset;
                    auto W2S = Globals::VisualEngine.World_To_Screen(World);

                    if (W2S.x < 0.f || W2S.y < 0.f) continue;

                    Valid = true;
                    Left = min(Left, W2S.x);
                    Top = min(Top, W2S.y);
                    Right = max(Right, W2S.x);
                    Bottom = max(Bottom, W2S.y);
                }
            }

            if (!Valid || Left >= Right || Top >= Bottom) continue;

            ImVec2 Pos(Left - 1.f, Top - 1.f);
            ImVec2 Size((Right - Left) + 2.f, (Bottom - Top) + 2.f);

            auto col_fade = [&](ImU32 c) -> ImU32 { return c; };

            if (Globals::Visuals::Box)
            {
                if (Globals::Visuals::Box_Type == 0) {

                    Pos.x = std::round(Pos.x);
                    Pos.y = std::round(Pos.y);
                    Size.x = std::round(Size.x);
                    Size.y = std::round(Size.y);

                    ImVec2 Min = Pos;
                    ImVec2 Max = ImVec2(Pos.x + Size.x, Pos.y + Size.y);

                    if (Globals::Visuals::Box_Fill) {

                        if (Globals::Visuals::Box_Fill_Gradient) {

                            float time = (Globals::Visuals::Box_Fill_Gradient_Rotate) ? (float)ImGui::GetTime() * Globals::Visuals::BoxFillSpeed : 0.0f;
                            ImU32 col1 = FlotationDevice(Globals::Visuals::Colors::BoxFill_Top);
                            ImU32 col2 = FlotationDevice(Globals::Visuals::Colors::BoxFill_Bottom);

                            float s = sinf(time);
                            float c = cosf(time);

                            ImU32 c_tl, c_tr, c_br, c_bl;

                            if (Globals::Visuals::Box_Fill_Type == 0) {
                                c_tl = c_bl = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (s + 1.0f) * 0.5f));
                                c_tr = c_br = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (c + 1.0f) * 0.5f));
                            }
                            else if (Globals::Visuals::Box_Fill_Type == 1) {
                                c_tl = c_tr = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (s + 1.0f) * 0.5f));
                                c_bl = c_br = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (c + 1.0f) * 0.5f));
                            }
                            else {
                                c_tl = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (s + 1.0f) * 0.5f));
                                c_tr = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (c + 1.0f) * 0.5f));
                                c_br = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (-s + 1.0f) * 0.5f));
                                c_bl = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (-c + 1.0f) * 0.5f));
                            }

                            Draw->AddRectFilledMultiColor(ImVec2(Min.x + 1.f, Min.y + 1.f), ImVec2(Max.x - 1.f, Max.y - 1.f), c_tl, c_tr, c_br, c_bl);

                        }
                        else {
                            Draw->AddRectFilled(ImVec2(Min.x + 1.f, Min.y + 1.f), ImVec2(Max.x - 1.f, Max.y - 1.f), FlotationDevice(Globals::Visuals::Colors::BoxFill_Top));
                        }
                    }

                    Draw->AddRect(Min, Max, IM_COL32(0, 0, 0, 255), 0.f);
                    Draw->AddRect(ImVec2(Min.x - 1.f, Min.y - 1.f), ImVec2(Max.x + 1.f, Max.y + 1.f), FlotationDevice(Globals::Visuals::Colors::Box), 0.f);
                    Draw->AddRect(ImVec2(Min.x - 2.f, Min.y - 2.f), ImVec2(Max.x + 2.f, Max.y + 2.f), IM_COL32(0, 0, 0, 255), 0.f);
                }

                else if (Globals::Visuals::Box_Type == 1) {

                    float X1 = Pos.x - 1.f;
                    float Y1 = Pos.y - 1.f;
                    float X2 = Pos.x + Size.x + 1.f;
                    float Y2 = Pos.y + Size.y + 1.f;

                    float Box_Width = X2 - X1;
                    float Box_Height = Y2 - Y1;

                    float Length = min(Box_Width, Box_Height) * 0.25f;
                    Length = min(Length, 50.0f);
                    Length = min(Length, (min(Box_Width, Box_Height) * 0.5f) - 1.0f);

                    float X1_Length = X1 + Length;
                    float Y1_Length = Y1 + Length;
                    float X2_Length = X2 - Length;
                    float Y2_Length = Y2 - Length;

                    if (Globals::Visuals::Box_Fill) {

                        if (Globals::Visuals::Box_Fill_Gradient) {

                            float time = (Globals::Visuals::Box_Fill_Gradient_Rotate) ? (float)ImGui::GetTime() * Globals::Visuals::BoxFillSpeed : 0.0f;
                            ImU32 col1 = FlotationDevice(Globals::Visuals::Colors::BoxFill_Top);
                            ImU32 col2 = FlotationDevice(Globals::Visuals::Colors::BoxFill_Bottom);

                            float s = sinf(time);
                            float c = cosf(time);

                            ImU32 c_tl, c_tr, c_br, c_bl;

                            if (Globals::Visuals::Box_Fill_Type == 0) {
                                c_tl = c_bl = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (s + 1.0f) * 0.5f));
                                c_tr = c_br = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (c + 1.0f) * 0.5f));
                            }
                            else if (Globals::Visuals::Box_Fill_Type == 1) {
                                c_tl = c_tr = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (s + 1.0f) * 0.5f));
                                c_bl = c_br = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (c + 1.0f) * 0.5f));
                            }
                            else {
                                c_tl = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (s + 1.0f) * 0.5f));
                                c_tr = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (c + 1.0f) * 0.5f));
                                c_br = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (-s + 1.0f) * 0.5f));
                                c_bl = ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(col1), ImGui::ColorConvertU32ToFloat4(col2), (-c + 1.0f) * 0.5f));
                            }

                            Draw->AddRectFilledMultiColor(ImVec2(X1 + 2.f, Y1 + 2.f), ImVec2(X2 - 2.f, Y2 - 2.f), c_tl, c_tr, c_br, c_bl);

                        }
                        else {
                            Draw->AddRectFilled(ImVec2(X1 + 2.f, Y1 + 2.f), ImVec2(X2 - 2.f, Y2 - 2.f), FlotationDevice(Globals::Visuals::Colors::BoxFill_Top));
                        }
                    }

                    Draw->AddRectFilled(ImVec2(X1 - 1.f, Y1 - 1.f), ImVec2(X1_Length + 1.f, Y1 + 1.f), IM_COL32(0, 0, 0, 255));
                    Draw->AddRectFilled(ImVec2(X1 - 1.f, Y1 - 1.f), ImVec2(X1 + 1.f, Y1_Length + 1.f), IM_COL32(0, 0, 0, 255));
                    Draw->AddRectFilled(ImVec2(X2_Length - 1.f, Y1 - 1.f), ImVec2(X2 + 1.f, Y1 + 1.f), IM_COL32(0, 0, 0, 255));
                    Draw->AddRectFilled(ImVec2(X2 - 1.f, Y1 - 1.f), ImVec2(X2 + 1.f, Y1_Length + 1.f), IM_COL32(0, 0, 0, 255));
                    Draw->AddRectFilled(ImVec2(X1 - 1.f, Y2 - 1.f), ImVec2(X1_Length + 1.f, Y2 + 1.f), IM_COL32(0, 0, 0, 255));
                    Draw->AddRectFilled(ImVec2(X1 - 1.f, Y2_Length - 1.f), ImVec2(X1 + 1.f, Y2 + 1.f), IM_COL32(0, 0, 0, 255));
                    Draw->AddRectFilled(ImVec2(X2_Length - 1.f, Y2 - 1.f), ImVec2(X2 + 1.f, Y2 + 1.f), IM_COL32(0, 0, 0, 255));
                    Draw->AddRectFilled(ImVec2(X2 - 1.f, Y2_Length - 1.f), ImVec2(X2 + 1.f, Y2 + 1.f), IM_COL32(0, 0, 0, 255));

                    Draw->AddRectFilled(ImVec2(X1 + 1.f, Y1 + 1.f), ImVec2(X1_Length + 1.f, Y1 + 2.f), IM_COL32(0, 0, 0, 255));
                    Draw->AddRectFilled(ImVec2(X1 + 1.f, Y1 + 1.f), ImVec2(X1 + 2.f, Y1_Length + 1.f), IM_COL32(0, 0, 0, 255));
                    Draw->AddRectFilled(ImVec2(X2_Length - 1.f, Y1 + 1.f), ImVec2(X2 - 1.f, Y1 + 2.f), IM_COL32(0, 0, 0, 255));
                    Draw->AddRectFilled(ImVec2(X2 - 2.f, Y1 + 1.f), ImVec2(X2 - 1.f, Y1_Length + 1.f), IM_COL32(0, 0, 0, 255));
                    Draw->AddRectFilled(ImVec2(X1 + 1.f, Y2 - 2.f), ImVec2(X1_Length + 1.f, Y2 - 1.f), IM_COL32(0, 0, 0, 255));
                    Draw->AddRectFilled(ImVec2(X1 + 1.f, Y2_Length - 1.f), ImVec2(X1 + 2.f, Y2 - 1.f), IM_COL32(0, 0, 0, 255));
                    Draw->AddRectFilled(ImVec2(X2_Length - 1.f, Y2 - 2.f), ImVec2(X2 - 1.f, Y2 - 1.f), IM_COL32(0, 0, 0, 255));
                    Draw->AddRectFilled(ImVec2(X2 - 2.f, Y2_Length - 1.f), ImVec2(X2 - 1.f, Y2 - 1.f), IM_COL32(0, 0, 0, 255));

                    Draw->AddRectFilled(ImVec2(X1, Y1), ImVec2(X1_Length, Y1 + 1.f), FlotationDevice(Globals::Visuals::Colors::Box));
                    Draw->AddRectFilled(ImVec2(X1, Y1), ImVec2(X1 + 1.f, Y1_Length), FlotationDevice(Globals::Visuals::Colors::Box));
                    Draw->AddRectFilled(ImVec2(X2_Length, Y1), ImVec2(X2, Y1 + 1.f), FlotationDevice(Globals::Visuals::Colors::Box));
                    Draw->AddRectFilled(ImVec2(X2 - 1.f, Y1), ImVec2(X2, Y1_Length), FlotationDevice(Globals::Visuals::Colors::Box));
                    Draw->AddRectFilled(ImVec2(X1, Y2 - 1.f), ImVec2(X1_Length, Y2), FlotationDevice(Globals::Visuals::Colors::Box));
                    Draw->AddRectFilled(ImVec2(X1, Y2_Length), ImVec2(X1 + 1.f, Y2), FlotationDevice(Globals::Visuals::Colors::Box));
                    Draw->AddRectFilled(ImVec2(X2_Length, Y2 - 1.f), ImVec2(X2, Y2), FlotationDevice(Globals::Visuals::Colors::Box));
                    Draw->AddRectFilled(ImVec2(X2 - 1.f, Y2_Length), ImVec2(X2, Y2), FlotationDevice(Globals::Visuals::Colors::Box));
                }
            }

            if (Globals::Visuals::Healthbar) {

                if (Globals::Visuals::Healthbar_Type == 0) {

                    float Ratio = (Player.MaxHealth > 0.f) ? Player.Health / Player.MaxHealth : 0.f;
                    Ratio = ImClamp(Ratio, 0.f, 1.f);

                    float Gap = static_cast<float>(Globals::Visuals::Gap);
                    float Thickness = static_cast<float>(Globals::Visuals::Thickness);

                    float X_Min = Pos.x - Gap - Thickness - 3.f;
                    float Y_Min = Pos.y - 1.f;
                    float Y_Max = Pos.y + Size.y + 1.f;

                    ImVec2 Bg_Min(X_Min - 1.f, Y_Min - 1.f);
                    ImVec2 Bg_Max(X_Min + Thickness + 1.f, Y_Max + 1.f);
                    Draw->AddRectFilled(Bg_Min, Bg_Max, IM_COL32(0, 0, 0, 255));

                    ImVec2 Empty_Min(X_Min, Y_Min);
                    ImVec2 Empty_max(X_Min + Thickness, Y_Max);
                    Draw->AddRectFilled(Empty_Min, Empty_max, IM_COL32(130, 130, 130, 150));

                    float Height = (Y_Max - Y_Min) * Ratio;
                    ImVec2 Fg_Min(X_Min, Y_Max - Height);
                    ImVec2 Fg_max(X_Min + Thickness, Y_Max);
                    Draw->AddRectFilled(Fg_Min, Fg_max, FlotationDevice(Globals::Visuals::Colors::Healthbar));
                }

                else if (Globals::Visuals::Healthbar_Type == 1) {

                    float Ratio = (Player.MaxHealth > 0.f) ? Player.Health / Player.MaxHealth : 0.f;
                    Ratio = ImClamp(Ratio, 0.f, 1.f);

                    float Gap = static_cast<float>(Globals::Visuals::Gap);
                    float Thickness = static_cast<float>(Globals::Visuals::Thickness);

                    float X_Min = Pos.x - Gap - Thickness - 3.f;
                    float Y_Min = Pos.y - 1.f;
                    float Y_Max = Pos.y + Size.y + 1.f;

                    Draw->AddRectFilled(ImVec2(X_Min - 1, Y_Min - 1), ImVec2(X_Min + Thickness + 1, Y_Max + 1), IM_COL32(0, 0, 0, 255));

                    Draw->AddRectFilled(ImVec2(X_Min, Y_Min), ImVec2(X_Min + Thickness, Y_Max), IM_COL32(130, 130, 130, 150));

                    ImU32 TopColor = IM_COL32((int)(Globals::Visuals::Colors::Healthbar_Top[0] * 255), (int)(Globals::Visuals::Colors::Healthbar_Top[1] * 255), (int)(Globals::Visuals::Colors::Healthbar_Top[2] * 255), (int)(Globals::Visuals::Colors::Healthbar_Top[3] * 255));
                    ImU32 MiddleColor = IM_COL32((int)(Globals::Visuals::Colors::Healthbar_Middle[0] * 255), (int)(Globals::Visuals::Colors::Healthbar_Middle[1] * 255), (int)(Globals::Visuals::Colors::Healthbar_Middle[2] * 255), (int)(Globals::Visuals::Colors::Healthbar_Middle[3] * 255));
                    ImU32 BottomColor = IM_COL32((int)(Globals::Visuals::Colors::Healthbar_Bottom[0] * 255), (int)(Globals::Visuals::Colors::Healthbar_Bottom[1] * 255), (int)(Globals::Visuals::Colors::Healthbar_Bottom[2] * 255), (int)(Globals::Visuals::Colors::Healthbar_Bottom[3] * 255));

                    float FullHeight = Y_Max - Y_Min;
                    float HealthHeight = FullHeight * Ratio;

                    float FillMinY = Y_Max - HealthHeight;
                    float MidY = Y_Min + FullHeight * 0.5f;

                    if (FillMinY < MidY) {

                        Draw->AddRectFilledMultiColor(ImVec2(X_Min, ImMax(FillMinY, Y_Min)), ImVec2(X_Min + Thickness, MidY), TopColor, TopColor, MiddleColor, MiddleColor);
                    }

                    Draw->AddRectFilledMultiColor(ImVec2(X_Min, ImMax(FillMinY, MidY)), ImVec2(X_Min + Thickness, Y_Max), MiddleColor, MiddleColor, BottomColor, BottomColor);
                }
            }

            if (Globals::Visuals::Health)
            {
                std::string HealthStr = "[" + std::to_string(static_cast<int>(Player.Health)) + "]";

                float X_Text = Pos.x - 6.0f;
                if (Globals::Visuals::Healthbar) {
                    X_Text -= Globals::Visuals::Thickness + Globals::Visuals::Gap;
                }


                float Y_text = Pos.y - 3.0f;

                ImGui::PushFont(Tahoma_BoldXP);
                ImVec2 Text_Size = ImGui::CalcTextSize(HealthStr.c_str());
                ImGui::PopFont();

                ImVec2 Text_Pos(X_Text - Text_Size.x, Y_text);

                Outline(Text_Pos, HealthStr.c_str(), Globals::Visuals::Colors::Health);
            }

            if (Globals::Visuals::Name) {

                if (Globals::Visuals::Name_Type == 0) {

                    ImGui::PushFont(Tahoma_BoldXP);
                    ImVec2 Text_Size = ImGui::CalcTextSize(Player.Name.c_str());
                    ImGui::PopFont();

                    ImVec2 Text_Position(Pos.x + (Size.x * 0.5f) - (Text_Size.x * 0.5f), Pos.y - Text_Size.y - 3.f);

                    Outline(Text_Position, Player.Name.c_str(), Globals::Visuals::Colors::Name);
                }

                else if (Globals::Visuals::Name_Type == 1) {

                    ImGui::PushFont(Tahoma_BoldXP);
                    ImVec2 Text_Size = ImGui::CalcTextSize(Player.Display_Name.c_str());
                    ImGui::PopFont();

                    ImVec2 Text_Position(Pos.x + (Size.x * 0.5f) - (Text_Size.x * 0.5f), Pos.y - Text_Size.y - 3.f);

                    Outline(Text_Position, Player.Display_Name.c_str(), Globals::Visuals::Colors::Name);
                }

                else if (Globals::Visuals::Name_Type == 2) {

                    std::string Text = Player.Name + " [" + Player.Display_Name + "]";

                    ImGui::PushFont(Tahoma_BoldXP);
                    ImVec2 Text_Size = ImGui::CalcTextSize(Text.c_str());
                    float NameW = ImGui::CalcTextSize((Player.Name + " ").c_str()).x;
                    float BracketW = ImGui::CalcTextSize("[").x;
                    float DisplayW = ImGui::CalcTextSize(Player.Display_Name.c_str()).x;
                    ImGui::PopFont();

                    ImVec2 Position(Pos.x + (Size.x * 0.5f) - (Text_Size.x * 0.5f), Pos.y - Text_Size.y - 3.f);

                    Outline(Position, Player.Name.c_str(), Globals::Visuals::Colors::Name);
                    Outline(ImVec2(Position.x + NameW, Position.y - 2.f), "[", Globals::Visuals::Colors::Name);

                    static float white[4] = { 1.f, 1.f, 1.f, 1.f };
                    Outline(ImVec2(Position.x + NameW + BracketW, Position.y - 1.f), Player.Display_Name.c_str(), white);

                    Outline(ImVec2(Position.x + NameW + BracketW + DisplayW, Position.y - 2.f), "]", Globals::Visuals::Colors::Name);
                }
            }

            if (isFriend) {
                float tagOffset = 0.f;
                if (Globals::Visuals::Distance) tagOffset += 18.f;
                ImGui::PushFont(Tahoma_BoldXP);
                const char* tag = "[F]";
                ImVec2 tagSize = ImGui::CalcTextSize(tag);
                ImGui::PopFont();
                ImVec2 tagPos(Pos.x + (Size.x * 0.5f) - (tagSize.x * 0.5f), Pos.y + Size.y + 3.f + tagOffset);
                Outline(tagPos, tag, Globals::Visuals::Colors::Friend);
            }

            if (Globals::Visuals::Distance) {

                ImGui::PushFont(Tahoma_BoldXP);

                char Buffer[16];
                snprintf(Buffer, sizeof(Buffer), "[%dm]", static_cast<int>(Player.Distance));

                ImVec2 Text_Size = ImGui::CalcTextSize(Buffer);
                ImGui::PopFont();

                ImVec2 Text_Position(Pos.x + (Size.x * 0.5f) - (Text_Size.x * 0.5f), Pos.y + Size.y + 3.0f);

                Outline(Text_Position, Buffer, Globals::Visuals::Colors::Distance);
            }

            if (Globals::Visuals::Rig_Type)
            {
                ImGui::PushFont(Tahoma_BoldXP);

                const char* Rig_Type = nullptr;

                if (Player.Rig_Type == 1)
                    Rig_Type = "[R15]";
                else if (Player.Rig_Type == 0)
                    Rig_Type = "[R6]";
                else
                {
                    ImGui::PopFont();
                    continue;
                }

                ImVec2 Text_Size = ImGui::CalcTextSize(Rig_Type);

                ImVec2 Text_Position(std::round(Pos.x + Size.x + 5.0f), std::round(Pos.y - Text_Size.y + 10.0f));

                Outline(Text_Position, Rig_Type, Globals::Visuals::Colors::Rig_Type);

                ImGui::PopFont();
            }

            if (Globals::Visuals::Tool)
            {
                std::string Cl_Name;

                const std::string& Tool_Name = Player.Tool_Name;

                if (Tool_Name.empty()) {

                    Cl_Name = "[None]";
                }

                else {

                    Cl_Name.reserve(Tool_Name.size());

                    for (char c : Tool_Name) {

                        if (c != '[' && c != ']')
                            Cl_Name.push_back(c);
                    }

                    if (!Cl_Name.empty()) {

                        Cl_Name.insert(Cl_Name.begin(), '[');
                        Cl_Name.push_back(']');
                    }
                }

                ImGui::PushFont(Tahoma_BoldXP);
                ImVec2 Text_Size = ImGui::CalcTextSize(Cl_Name.c_str());
                ImGui::PopFont();

                float Offset = Globals::Visuals::Distance ? 18.0f : 3.0f;

                ImVec2 Text_Position(std::round(Pos.x + (Size.x * 0.5f) - (Text_Size.x * 0.5f)), std::round(Pos.y + Size.y + Offset));

                Outline(Text_Position, Cl_Name.c_str(), Globals::Visuals::Colors::Tool);
            }

            if (Globals::Visuals::Skeleton) {

                const ImU32 SkelCol = FlotationDevice(Globals::Visuals::Colors::Skeleton);
                const ImU32 OutlineCol = IM_COL32(0, 0, 0, 255);
                const float Thickness = 1.0f;

                auto W2S = [&](const SDK::Vector3& WorldPos, ImVec2& Out) -> bool
                    {
                        auto ScreenPos = Globals::VisualEngine.World_To_Screen(WorldPos);
                        if (ScreenPos.x <= 0.f || ScreenPos.y <= 0.f) return false;
                        Out.x = std::roundf(ScreenPos.x);
                        Out.y = std::roundf(ScreenPos.y);
                        return true;
                    };

                auto DrawPoly = [&](const ImVec2* Points, int Count)
                    {
                        if (Count < 2) return;
                        Draw->AddPolyline(Points, Count, OutlineCol, Thickness + 2.f);
                        Draw->AddPolyline(Points, Count, SkelCol, Thickness);
                    };

                if (Player.UpperTorso.Address && Player.LowerTorso.Address)
                {

                    auto ProcessR6Chain = [&](const SDK::Instance* Instances, int Count)
                        {
                            ImVec2 ScreenPoints[4];
                            int ValidCount = 0;

                            for (int i = 0; i < Count; ++i)
                            {
                                if (!Instances[i].Address)
                                {
                                    DrawPoly(ScreenPoints, ValidCount);
                                    ValidCount = 0;
                                    continue;
                                }

                                SDK::Part Part(Instances[i].Address);
                                if (!Part.Address)
                                {
                                    DrawPoly(ScreenPoints, ValidCount);
                                    ValidCount = 0;
                                    continue;
                                }

                                ImVec2 ScreenPos;
                                if (!W2S(Part.Get_Primitive().Get_Position(), ScreenPos))
                                {
                                    DrawPoly(ScreenPoints, ValidCount);
                                    ValidCount = 0;
                                    continue;
                                }

                                ScreenPoints[ValidCount++] = ScreenPos;
                            }
                            DrawPoly(ScreenPoints, ValidCount);
                        };

                    const SDK::Instance Spine[] = { Player.Head, Player.UpperTorso, Player.LowerTorso };
                    ProcessR6Chain(Spine, 3);

                    const SDK::Instance LeftArm[] = { Player.UpperTorso, Player.LeftUpperArm, Player.LeftLowerArm, Player.LeftHand };
                    ProcessR6Chain(LeftArm, 4);

                    const SDK::Instance RightArm[] = { Player.UpperTorso, Player.RightUpperArm, Player.RightLowerArm, Player.RightHand };
                    ProcessR6Chain(RightArm, 4);

                    const SDK::Instance LeftLeg[] = { Player.LowerTorso, Player.LeftUpperLeg, Player.LeftLowerLeg, Player.LeftFoot };
                    ProcessR6Chain(LeftLeg, 4);

                    const SDK::Instance RightLeg[] = { Player.LowerTorso, Player.RightUpperLeg, Player.RightLowerLeg, Player.RightFoot };
                    ProcessR6Chain(RightLeg, 4);
                }

                else if (Player.Torso.Address && Player.Head.Address)
                {
                    SDK::Part TorsoPart(Player.Torso.Address);
                    SDK::Part HeadPart(Player.Head.Address);
                    const auto& TorsoPrim = TorsoPart.Get_Primitive();
                    const auto& HeadPrim = HeadPart.Get_Primitive();

                    const SDK::Vector3 TorsoPos = TorsoPrim.Get_Position();
                    const SDK::Vector3 TorsoSize = TorsoPrim.Get_Size();
                    const auto TorsoRot = TorsoPrim.Get_Rotation();
                    const SDK::Vector3 HeadPos = HeadPrim.Get_Position();
                    const SDK::Vector3 HeadSize = HeadPrim.Get_Size();

                    const SDK::Vector3 ShoulderCenter = TorsoPos + TorsoRot * SDK::Vector3{ 0, TorsoSize.y * 0.2f, 0 };
                    const SDK::Vector3 HipCenter = TorsoPos - TorsoRot * SDK::Vector3{ 0, TorsoSize.y * 0.4f, 0 };
                    const SDK::Vector3 HeadBottom = HeadPos - SDK::Vector3{ 0, HeadSize.y * 0.5f, 0 };
                    const SDK::Vector3 ShoulderLeft = ShoulderCenter + TorsoRot * SDK::Vector3{ -TorsoSize.x * 0.5f, 0, 0 };
                    const SDK::Vector3 ShoulderRight = ShoulderCenter + TorsoRot * SDK::Vector3{ TorsoSize.x * 0.5f, 0, 0 };

                    auto ProcessR15Chain = [&](const SDK::Vector3* Points, int Count)
                        {
                            ImVec2 ScreenPoints[8];
                            int ValidCount = 0;
                            for (int i = 0; i < Count; ++i)
                            {
                                ImVec2 ScreenPos;
                                if (W2S(Points[i], ScreenPos))
                                    ScreenPoints[ValidCount++] = ScreenPos;
                            }
                            DrawPoly(ScreenPoints, ValidCount);
                        };

                    {
                        const SDK::Vector3 SpinePts[] = { HeadPos, HeadBottom, ShoulderCenter, HipCenter };
                        ProcessR15Chain(SpinePts, 4);
                    }

                    {
                        SDK::Vector3 ArmPts[4];
                        int Count = 0;
                        ArmPts[Count++] = ShoulderCenter;
                        ArmPts[Count++] = ShoulderLeft;

                        if (Player.LeftArm.Address)
                        {
                            SDK::Part Arm(Player.LeftArm.Address);
                            const auto& ArmPrim = Arm.Get_Primitive();
                            const auto& ArmRot = ArmPrim.Get_Rotation();
                            const SDK::Vector3 ArmPos = ArmPrim.Get_Position();
                            const SDK::Vector3 ArmSize = ArmPrim.Get_Size();

                            ArmPts[Count++] = ArmPos + ArmRot * SDK::Vector3{ 0, ArmSize.y * 0.2f, 0 };
                            ArmPts[Count++] = ArmPos - ArmRot * SDK::Vector3{ 0, ArmSize.y * 0.5f, 0 };
                        }
                        ProcessR15Chain(ArmPts, Count);
                    }

                    {
                        SDK::Vector3 ArmPts[4];
                        int Count = 0;
                        ArmPts[Count++] = ShoulderCenter;
                        ArmPts[Count++] = ShoulderRight;

                        if (Player.RightArm.Address)
                        {
                            SDK::Part Arm(Player.RightArm.Address);
                            const auto& ArmPrim = Arm.Get_Primitive();
                            const auto& ArmRot = ArmPrim.Get_Rotation();
                            const SDK::Vector3 ArmPos = ArmPrim.Get_Position();
                            const SDK::Vector3 ArmSize = ArmPrim.Get_Size();

                            ArmPts[Count++] = ArmPos + ArmRot * SDK::Vector3{ 0, ArmSize.y * 0.2f, 0 };
                            ArmPts[Count++] = ArmPos - ArmRot * SDK::Vector3{ 0, ArmSize.y * 0.5f, 0 };
                        }
                        ProcessR15Chain(ArmPts, Count);
                    }

                    {
                        SDK::Vector3 LegPts[3];
                        int Count = 0;
                        LegPts[Count++] = HipCenter;

                        if (Player.LeftLeg.Address)
                        {
                            SDK::Part Leg(Player.LeftLeg.Address);
                            const auto& LegPrim = Leg.Get_Primitive();
                            const auto& LegRot = LegPrim.Get_Rotation();
                            const SDK::Vector3 LegPos = LegPrim.Get_Position();
                            const SDK::Vector3 LegSize = LegPrim.Get_Size();

                            LegPts[Count++] = LegPos + LegRot * SDK::Vector3{ 0, LegSize.y * 0.5f, 0 };
                            LegPts[Count++] = LegPos - LegRot * SDK::Vector3{ 0, LegSize.y * 0.5f, 0 };
                        }
                        ProcessR15Chain(LegPts, Count);
                    }

                    {
                        SDK::Vector3 LegPts[3];
                        int Count = 0;
                        LegPts[Count++] = HipCenter;

                        if (Player.RightLeg.Address)
                        {
                            SDK::Part Leg(Player.RightLeg.Address);
                            const auto& LegPrim = Leg.Get_Primitive();
                            const auto& LegRot = LegPrim.Get_Rotation();
                            const SDK::Vector3 LegPos = LegPrim.Get_Position();
                            const SDK::Vector3 LegSize = LegPrim.Get_Size();

                            LegPts[Count++] = LegPos + LegRot * SDK::Vector3{ 0, LegSize.y * 0.5f, 0 };
                            LegPts[Count++] = LegPos - LegRot * SDK::Vector3{ 0, LegSize.y * 0.5f, 0 };
                        }
                        ProcessR15Chain(LegPts, Count);
                    }
                }
            }

            if (Globals::Visuals::LookDirection && Player.Head.Address) {
                SDK::Part HeadPart(Player.Head.Address);
                auto HP = HeadPart.Get_Primitive();
                if (HP.Address) {
                    SDK::Vector3 hpos = HP.Get_Position();
                    SDK::Matrix3 hrot = HP.Get_Rotation();
                    SDK::Vector3 fwd = { -hrot.data[2], -hrot.data[5], -hrot.data[8] };
                    float rayLen = 7.f;
                    SDK::Vector3 rayEnd = { hpos.x + fwd.x * rayLen, hpos.y + fwd.y * rayLen, hpos.z + fwd.z * rayLen };
                    auto s0 = Globals::VisualEngine.World_To_Screen(hpos);
                    auto s1 = Globals::VisualEngine.World_To_Screen(rayEnd);
                    if (s0.x > 0 && s0.y > 0 && s1.x > 0 && s1.y > 0) {
                        ImU32 rayCol = col_fade(FlotationDevice(Globals::Visuals::LookDirColor));
                        Draw->AddLine(ImVec2(s0.x, s0.y), ImVec2(s1.x, s1.y), IM_COL32(0,0,0,180), 3.f);
                        Draw->AddLine(ImVec2(s0.x, s0.y), ImVec2(s1.x, s1.y), rayCol, 1.5f);
                    }
                }
            }

            if (Globals::Visuals::Snapline) {
                ImU32 snapCol = col_fade(FlotationDevice(Globals::Visuals::Colors::Snapline));
                auto screenSize = ImGui::GetIO().DisplaySize;
                float centerX = Pos.x + Size.x * 0.5f;
                float centerY = Pos.y + Size.y * 0.5f;
                float bottomY = Pos.y + Size.y;
                float endX = centerX, endY = 0.f;
                if (Globals::Visuals::SnaplinePosition == 0)
                    endY = screenSize.y;
                else if (Globals::Visuals::SnaplinePosition == 1)
                    endY = screenSize.y * 0.5f;
                else
                    endY = 0.f;
                Draw->AddLine(ImVec2(centerX, bottomY), ImVec2(endX, endY), IM_COL32(0,0,0,180), 2.f);
                Draw->AddLine(ImVec2(centerX, bottomY), ImVec2(endX, endY), snapCol, 1.f);
            }

            if (Globals::Visuals::HeadDot && Player.Head.Address) {
                SDK::Part hPart(Player.Head.Address);
                auto hPrim = hPart.Get_Primitive();
                if (hPrim.Address) {
                    auto hW2S = Globals::VisualEngine.World_To_Screen(hPrim.Get_Position());
                    if (hW2S.x > 0 && hW2S.y > 0) {
                        float r = Globals::Visuals::HeadDotSize;
                        ImU32 dotCol = col_fade(FlotationDevice(Globals::Visuals::Colors::HeadDot));
                        Draw->AddCircleFilled(ImVec2(hW2S.x, hW2S.y), r + 1.f, IM_COL32(0,0,0,180));
                        Draw->AddCircleFilled(ImVec2(hW2S.x, hW2S.y), r, dotCol);
                    }
                }
            }

            // ── Always-on filled chams ──
            if (Globals::Visuals::Chams)
            {
                float* cc = Globals::Visuals::ChamsColor;
                int fill_a = (int)(cc[3] * 255);
                ImU32 fill_col = IM_COL32((int)(cc[0]*255), (int)(cc[1]*255), (int)(cc[2]*255), fill_a);
                ImU32 out_col = IM_COL32((int)(cc[0]*255), (int)(cc[1]*255), (int)(cc[2]*255), ImMin(fill_a + 100, 255));

                auto chamsBones = Visuals::Get_Bones(Player);
                for (auto* inst : chamsBones) {
                    if (!inst || !inst->Address) continue;
                    SDK::Part p(inst->Address);
                    SDK::Part prim = p.Get_Primitive();
                    if (!prim.Address) continue;
                    std::string nm = p.Name();
                    if (nm == "HumanoidRootPart") continue;

                    SDK::Vector3 pos = prim.Get_Position();
                    SDK::Vector3 sz = prim.Get_Size();
                    if (fabsf(sz.x) < 0.01f && fabsf(sz.y) < 0.01f && fabsf(sz.z) < 0.01f) continue;
                    SDK::Matrix3 rot = prim.Get_Rotation();

                    std::string meshId = GetMeshIdFromPart(prim);
                    if (!meshId.empty()) {
                        RenderPartMesh(Draw, pos, rot, meshId, fill_col);
                    } else {
                        FallbackRenderPart(Draw, pos, sz, rot, fill_col, out_col);
                    }
                }
            }

        }

        if (Globals::Aimbot::DrawFov)
        {
            static float FovRotation = 0.f;

            if (Globals::Aimbot::FovSpin)
            {
                FovRotation += Globals::Aimbot::FovSpinSpeed / 1000.f;
            }

            POINT Cursor;
            GetCursorPos(&Cursor);

            HWND Roblox = FindWindowA(NULL, "Roblox");
            if (Roblox)
                ScreenToClient(Roblox, &Cursor);

            ImVec2 MousePos((float)Cursor.x, (float)Cursor.y);

            ImDrawList* Draw = ImGui::GetBackgroundDrawList();

            const int Sides = 10;
            ImVec2 Points[Sides];
            float Step = 2.f * IM_PI / Sides;

            for (int i = 0; i < Sides; i++)
            {
                float Angle = i * Step + FovRotation;
                Points[i] = ImVec2(MousePos.x + cosf(Angle) * Globals::Aimbot::FovSize, MousePos.y + sinf(Angle) * Globals::Aimbot::FovSize);
            }

            ImU32 Color = ImGui::ColorConvertFloat4ToU32(ImVec4(Globals::Aimbot::FovColor[0], Globals::Aimbot::FovColor[1], Globals::Aimbot::FovColor[2], Globals::Aimbot::FovColor[3]));

            if (Globals::Aimbot::FillFov)
            {
                Draw->AddConvexPolyFilled(Points, Sides, IM_COL32((Color >> 0) & 0xFF, (Color >> 8) & 0xFF, (Color >> 16) & 0xFF, 100));
            }

            Draw->AddPolyline(Points, Sides, Color, 2.f, ImDrawFlags_Closed);
        }

        if (Globals::Silent::DrawFov)
        {
            static float FovRotation = 0.f;

            if (Globals::Silent::FovSpin)
            {
                FovRotation += Globals::Silent::FovSpinSpeed / 1000.f;
            }

            POINT Cursor;
            GetCursorPos(&Cursor);

            HWND Roblox = FindWindowA(NULL, "Roblox");
            if (Roblox)
                ScreenToClient(Roblox, &Cursor);

            ImVec2 MousePos((float)Cursor.x, (float)Cursor.y);

            ImDrawList* Draw = ImGui::GetBackgroundDrawList();

            const int Sides = 10;
            ImVec2 Points[Sides];
            float Step = 2.f * IM_PI / Sides;

            for (int i = 0; i < Sides; i++)
            {
                float Angle = i * Step + FovRotation;
                Points[i] = ImVec2(MousePos.x + cosf(Angle) * Globals::Silent::Fov, MousePos.y + sinf(Angle) * Globals::Silent::Fov);
            }

            ImU32 Color = ImGui::ColorConvertFloat4ToU32(ImVec4(Globals::Silent::FovColor[0], Globals::Silent::FovColor[1], Globals::Silent::FovColor[2], Globals::Silent::FovColor[3]));

            if (Globals::Silent::FillFov)
            {
                Draw->AddConvexPolyFilled(Points, Sides, IM_COL32((Color >> 0) & 0xFF, (Color >> 8) & 0xFF, (Color >> 16) & 0xFF, 100));
            }

            Draw->AddPolyline(Points, Sides, Color, 2.f, ImDrawFlags_Closed);
        }
        if (Globals::Watermark::Enabled)
        {
            static std::string userName;
            if (userName.empty()) {
                char buf[256];
                DWORD sz = GetEnvironmentVariableA("USERNAME", buf, sizeof(buf));
                if (sz > 0 && sz < sizeof(buf)) userName = buf;
                else userName = "unknown";
            }

            static auto HttpGet = [](HINTERNET hSession, const wchar_t* host, const wchar_t* path) -> std::string {
                std::string body;
                HINTERNET hConnect = WinHttpConnect(hSession, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
                if (!hConnect) return body;
                HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
                if (hRequest) {
                    LPCWSTR hdrs = L"User-Agent: Coercing/1.0\r\nAccept: application/json\r\n";
                    if (WinHttpSendRequest(hRequest, hdrs, wcslen(hdrs), WINHTTP_NO_REQUEST_DATA, 0, 0, 0) && WinHttpReceiveResponse(hRequest, NULL)) {
                        DWORD dwSize = 0;
                        while (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0) {
                            std::string buf(dwSize, 0);
                            DWORD read = 0;
                            if (WinHttpReadData(hRequest, &buf[0], dwSize, &read)) { buf.resize(read); body += buf; }
                        }
                    }
                    WinHttpCloseHandle(hRequest);
                }
                WinHttpCloseHandle(hConnect);
                return body;
            };
            static auto FetchGameName = [](uint64_t id) -> std::string {
                HINTERNET hSession = WinHttpOpen(L"Coercing/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
                if (!hSession) return {};
                DWORD tls = 0x00000800;
                WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &tls, sizeof(tls));
                wchar_t path[128];
                swprintf_s(path, L"/universes/get-universe-containing-place?placeid=%I64u", id);
                std::string uniResp = HttpGet(hSession, L"api.roblox.com", path);
                if (uniResp.empty()) { WinHttpCloseHandle(hSession); return {}; }
                auto extractVal = [](const std::string& s, const char* key) -> std::string {
                    if (const char* p = strstr(s.c_str(), key)) {
                        p += strlen(key);
                        const char* e = strchr(p, '"');
                        if (e) return std::string(p, e - p);
                        const char* c = p;
                        while (*c && *c != ',' && *c != '}' && !isspace(*c)) c++;
                        return std::string(p, c - p);
                    }
                    return {};
                };
                std::string uniStr = extractVal(uniResp, "\"UniverseId\":");
                if (uniStr.empty()) { WinHttpCloseHandle(hSession); return {}; }
                uint64_t universeId = _strtoui64(uniStr.c_str(), NULL, 10);
                if (!universeId) { WinHttpCloseHandle(hSession); return {}; }
                swprintf_s(path, L"/v1/games?universeIds=%I64u", universeId);
                std::string gameResp = HttpGet(hSession, L"games.roblox.com", path);
                WinHttpCloseHandle(hSession);
                if (gameResp.empty()) return {};
                std::string n = extractVal(gameResp, "\"name\":\"");
                if (n.empty()) n = extractVal(gameResp, "\"Name\":\"");
                return n;
            };

            static std::unordered_map<uint64_t, std::string> gameNameCache;
            static std::mutex gameNameMutex;
            static std::atomic<uint64_t> fetchInProgress{ 0 };

            std::string playersStr;
            int playerCount = 0;
            {
                std::lock_guard<std::mutex> lock(Cache::Mutex);
                playerCount = (int)Globals::Player_Cache.size();
            }
            if (Globals::Watermark::ShowPlayers)
                playersStr = std::format("Players: {}", playerCount + 1);

            std::string gameStr;
            if (Globals::Watermark::ShowGame && Globals::GameID != 0) {
                {
                    std::lock_guard<std::mutex> lock(gameNameMutex);
                    auto it = gameNameCache.find(Globals::GameID);
                    if (it != gameNameCache.end()) {
                        gameStr = std::format("Game: {}", it->second);
                    }
                }
                if (gameStr.empty()) {
                    uint64_t expected = 0;
                    if (fetchInProgress.compare_exchange_strong(expected, Globals::GameID)) {
                        std::thread([id = Globals::GameID]() {
                            std::string name = FetchGameName(id);
                            if (!name.empty()) {
                                std::lock_guard<std::mutex> lock(gameNameMutex);
                                gameNameCache[id] = name;
                            }
                            fetchInProgress.store(0);
                        }).detach();
                    }
                    gameStr = std::format("Game: {}", Globals::GameID);
                }
            }

            float fps = ImGui::GetIO().Framerate;
            std::string fullText = "scare.lol";
            if (!playersStr.empty()) fullText += " | " + playersStr;
            if (!gameStr.empty())    fullText += " | " + gameStr;
            fullText += std::format(" | {:.0f} FPS | Private Build", fps);

            ImVec2 textSize = ImGui::CalcTextSize(fullText.c_str());
            float padX = 14.f, padY = 8.f;
            ImVec2 bgMin(8.f, 8.f);
            ImVec2 bgMax(bgMin.x + textSize.x + padX * 2.f, bgMin.y + textSize.y + padY * 2.f);

            Draw->AddRectFilled(bgMin, bgMax, Menu::Theme::Surface(210), 6.f);
            Draw->AddRect(bgMin, bgMax, Menu::Theme::Outline(100), 6.f);

            float cx = bgMin.x, cy = bgMin.y, cw = bgMax.x - bgMin.x, ch = bgMax.y - bgMin.y;
            for (float gx = cx + 12.f; gx < cx + cw; gx += 28.f)
                Draw->AddLine(ImVec2(gx, cy), ImVec2(gx, cy + ch), Menu::Theme::Outline(40));
            for (float gy = cy + 12.f; gy < cy + ch; gy += 28.f)
                Draw->AddLine(ImVec2(cx, gy), ImVec2(cx + cw, gy), Menu::Theme::Outline(40));

            Draw->AddText(ImVec2(bgMin.x + padX, bgMin.y + padY), Menu::Theme::Text(), fullText.c_str());
        }

        Radar::Render();

        // ── Hit effects + Shot tracers ──
        {
            static std::unordered_map<std::string, float> prev_hp;
            static std::vector<struct HitmarkerEvent> hitmarkers;
            static std::vector<struct GhostSnapshot> ghosts;
            static std::vector<struct HitNotification> hit_notifs;
            static std::vector<struct ShotTracer> tracers;
            static bool lmb_prev = false;
            static float last_lmb_time = 0.f;
            static uintptr_t shot_target_addr = 0;

            float now = (float)ImGui::GetTime();

            bool lmb_now = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            bool lmb_edge = lmb_now && !lmb_prev;
            if (lmb_now) last_lmb_time = now;
            if (lmb_edge)
                shot_target_addr = Globals::Aimbot::AimTarget.Address;
            if (now - last_lmb_time > 0.6f)
                shot_target_addr = 0;
            lmb_prev = lmb_now;

            // ── Hit detection ──
            if (Globals::Visuals::HitEffectsType > 0)
            {
                for (auto& Player : Snapshot)
                {
                    if (!Player.Character.Address) continue;
                    std::string pname = Player.Name;
                    float cur_hp = Player.Health;
                    auto it = prev_hp.find(pname);
                    if (it != prev_hp.end() && cur_hp < it->second && cur_hp > 0)
                    {
                        if (shot_target_addr != 0 && Player.Character.Address != shot_target_addr)
                        {
                            prev_hp[pname] = cur_hp; continue;
                        }
                        float hp_prev = it->second;
                        float hp_cur = cur_hp;
                        if (hp_prev - hp_cur >= 3.f && (now - last_lmb_time) <= 0.5f)
                        {
                            SDK::Vector3 hit_pos = {0,0,0};
                            if (Player.HumanoidRootPart.Address)
                            {
                                SDK::Part hrp(Player.HumanoidRootPart.Address);
                                SDK::Part prim = hrp.Get_Primitive();
                                if (prim.Address) hit_pos = prim.Get_Position();
                            }
                            else if (Player.Torso.Address)
                            {
                                SDK::Part torso(Player.Torso.Address);
                                SDK::Part prim = torso.Get_Primitive();
                                if (prim.Address) hit_pos = prim.Get_Position();
                            }

                            int ht = Globals::Visuals::HitEffectsType;
                            if (ht == 1) {
                                hitmarkers.push_back({ now, hit_pos });
                            }
                            else if (ht == 2 || ht == 3 || ht == 4) {
                                GhostSnapshot snap;
                                snap.time = now;
                                for (auto* inst : Visuals::Get_Bones(Player)) {
                                    if (inst && inst->Address) {
                                        SDK::Part p(inst->Address);
                                        SDK::Part prim = p.Get_Primitive();
                                        if (prim.Address) {
                                            std::string n = p.Name();
                                            snap.positions[n] = prim.Get_Position();
                                            snap.sizes[n] = prim.Get_Size();
                                            snap.rotations[n] = prim.Get_Rotation();
                                            std::string mid = GetMeshIdFromPart(prim);
                                            if (!mid.empty()) snap.meshIds[n] = mid;
                                        }
                                    }
                                }
                                ghosts.push_back(std::move(snap));
                            }

                            if (Globals::Visuals::HitNotifications) {
                                float dmg = hp_prev - hp_cur;
                                const char* part = (Player.Head.Address) ? "body" : "body";
                                hit_notifs.push_back({ now, pname, dmg, part });
                            }
                        }
                    }
                    prev_hp[pname] = cur_hp;
                }
            }

            // ── Shot tracer creation ──
            if (Globals::Visuals::ShotTracers && lmb_edge)
            {
                SDK::Vector3 cam_pos = Globals::Camera.Get_CameraPos();
                SDK::Vector3 target_pos = {0,0,0};

                uintptr_t target_char = Globals::Aimbot::AimTarget.Address;
                if (target_char)
                {
                    SDK::Instance charInst(target_char);
                    SDK::Instance head = charInst.Find_First_Child("Head");
                    if (head.Address)
                    {
                        SDK::Part headPart(head.Address);
                        SDK::Part headPrim = headPart.Get_Primitive();
                        if (headPrim.Address)
                            target_pos = headPrim.Get_Position();
                    }
                    if (target_pos.x == 0 && target_pos.y == 0 && target_pos.z == 0)
                    {
                        SDK::Instance hrp = charInst.Find_First_Child("HumanoidRootPart");
                        if (hrp.Address)
                        {
                            SDK::Part hrpPart(hrp.Address);
                            SDK::Part hrpPrim = hrpPart.Get_Primitive();
                            if (hrpPrim.Address)
                                target_pos = hrpPrim.Get_Position();
                        }
                    }
                }

                if (target_pos.x != 0 || target_pos.y != 0 || target_pos.z != 0)
                {
                    float dx = target_pos.x - cam_pos.x;
                    float dy = target_pos.y - cam_pos.y;
                    float dz = target_pos.z - cam_pos.z;
                    float dist = sqrtf(dx*dx + dy*dy + dz*dz);
                    if (dist > 0.5f)
                    {
                        ShotTracer tr;
                        tr.time = now;
                        tr.origin = cam_pos;
                        tr.direction = { dx/dist, dy/dist, dz/dist };
                        tr.dist = dist;
                        tr.finished = false;
                        tracers.push_back(tr);
                    }
                }
            }

            // ── Cleanup expired events ──
            float hd = Globals::Visuals::HitEffectsDuration;
            hitmarkers.erase(std::remove_if(hitmarkers.begin(), hitmarkers.end(),
                [now, hd](auto& h) { return now - h.time > hd; }), hitmarkers.end());
            ghosts.erase(std::remove_if(ghosts.begin(), ghosts.end(),
                [now, hd](auto& g) { return now - g.time > hd; }), ghosts.end());
            hit_notifs.erase(std::remove_if(hit_notifs.begin(), hit_notifs.end(),
                [now](auto& n) { return now - n.time > 3.f; }), hit_notifs.end());
            tracers.erase(std::remove_if(tracers.begin(), tracers.end(),
                [now](auto& t) { return t.finished || now - t.time > 2.f; }), tracers.end());

            // ── Render hit notifications (Coercing panel style) ──
            if (Globals::Visuals::HitNotifications && !hit_notifs.empty())
            {
                ImDrawList* bg = ImGui::GetBackgroundDrawList();
                ImVec2 vp = ImGui::GetIO().DisplaySize;
                float fsz = 13.f;
                float panelPad = 12.f;
                float gap = 6.f;
                float rounding = 4.f;
                float sepY = 30.f;
                float stack_x = vp.x - 10.f;
                float stack_y = 60.f;

                for (size_t i = 0; i < hit_notifs.size(); i++)
                {
                    auto& n = hit_notifs[i];
                    float elapsed = now - n.time;

                    char titleBuf[128], infoBuf[128];
                    snprintf(titleBuf, sizeof(titleBuf), "%s", n.target_name.c_str());
                    snprintf(infoBuf, sizeof(infoBuf), "%.0f damage in %s", n.damage, n.hit_part.c_str());

                    ImVec2 titleSz = ImGui::CalcTextSize(titleBuf);
                    ImVec2 infoSz = ImGui::CalcTextSize(infoBuf);
                    float contentW = (titleSz.x > infoSz.x) ? titleSz.x : infoSz.x;
                    float w = panelPad + contentW + panelPad;
                    float h = sepY + 6.f + infoSz.y + panelPad;

                    float slide = (elapsed < 0.15f) ? elapsed / 0.15f : 1.f;
                    float alpha = 1.f;
                    if (elapsed > 2.2f) alpha = 1.f - (elapsed - 2.2f) / 0.8f;
                    if (alpha <= 0.f) continue;

                    float x = stack_x - w + (1.f - slide) * 80.f;
                    float y = stack_y + i * (h + gap);

                    int a = (int)(alpha * 255);

                    // Panel background (matches Widgets::Panel style)
                    bg->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h),
                        Menu::Theme::Surface(a), rounding);

                    // Grid lines (matches main menu background style)
                    for (float gx = x + 6.f; gx <= x + w - 6.f; gx += 24.f)
                        bg->AddLine(ImVec2(gx, y), ImVec2(gx, y + h), Menu::Theme::Outline(ImMin(72, a)));
                    for (float gy = y + 6.f; gy <= y + h - 6.f; gy += 24.f)
                        bg->AddLine(ImVec2(x, gy), ImVec2(x + w, gy), Menu::Theme::Outline(ImMin(72, a)));

                    // Panel border (matches Widgets::Panel style)
                    bg->AddRect(ImVec2(x, y), ImVec2(x + w, y + h),
                        Menu::Theme::Outline(ImMin(180, a)), rounding);

                    // Title separator line (matches Widgets::Panel style)
                    bg->AddLine(ImVec2(x, y + sepY), ImVec2(x + w, y + sepY),
                        Menu::Theme::Outline(ImMin(145, a)));

                    // Title text
                    float tx = x + panelPad;
                    bg->AddText(ImGui::GetFont(), fsz, ImVec2(tx, y + 8.f),
                        Menu::Theme::Text(a), titleBuf);

                    // Info text: "X damage" in accent, "in body" in muted
                    bg->AddText(ImGui::GetFont(), fsz, ImVec2(tx, y + sepY + 6.f),
                        Menu::Theme::MutedText(a), infoBuf);
                }
            }

            // ── Render hitmarkers (type 1) ──
            if (Globals::Visuals::HitEffectsType == 1)
            {
                float* hc = Globals::Visuals::HitEffectsColor;
                int cr = (int)(hc[0]*255), cg = (int)(hc[1]*255), cb = (int)(hc[2]*255);

                for (auto& hm : hitmarkers)
                {
                    float elapsed = now - hm.time;
                    float alpha = 1.f - elapsed / hd;
                    if (alpha <= 0) continue;

                    auto sp = Globals::VisualEngine.World_To_Screen(hm.world_pos);
                    if (sp.x <= 0 || sp.y <= 0) continue;

                    float expand = elapsed * 15.f;
                    float inner = 4.f + expand;
                    float outer = 10.f + expand;
                    int a = (int)(alpha * 255);
                    ImU32 col = IM_COL32(cr, cg, cb, a);
                    ImU32 shadow = IM_COL32(0, 0, 0, a/2);

                    float angles[4] = { 0.785f, 2.356f, 3.927f, 5.498f };
                    for (int i = 0; i < 4; i++) {
                        float ang = angles[i];
                        ImVec2 p1(sp.x + cosf(ang) * inner, sp.y + sinf(ang) * inner);
                        ImVec2 p2(sp.x + cosf(ang) * outer, sp.y + sinf(ang) * outer);
                        Draw->AddLine(p1, p2, shadow, 3.f);
                        Draw->AddLine(p1, p2, col, 1.5f);
                    }
                }
            }

            // ── Render ghost skeletons (type 2) ──
            if (Globals::Visuals::HitEffectsType == 2)
            {
                float* hc = Globals::Visuals::HitEffectsColor;

                for (auto& ghost : ghosts)
                {
                    float elapsed = now - ghost.time;
                    float alpha = 1.f - elapsed / hd;
                    if (alpha <= 0) continue;

                    int a = (int)(alpha * hc[3] * 255);
                    ImU32 col = IM_COL32((int)(hc[0]*255), (int)(hc[1]*255), (int)(hc[2]*255), a);
                    ImU32 shadow = IM_COL32(0,0,0,a/2);

                    auto gs = [&](const std::string& p) -> SDK::Vector2 {
                        auto it = ghost.positions.find(p);
                        if (it == ghost.positions.end()) return {-1,-1};
                        return Globals::VisualEngine.World_To_Screen(it->second);
                    };

                    auto gbone = [&](const std::string& a, const std::string& b) {
                        auto sa = gs(a), sb = gs(b);
                        if (sa.x < 0 || sb.x < 0) return;
                        Draw->AddLine(ImVec2(sa.x, sa.y), ImVec2(sb.x, sb.y), shadow, 3.f);
                        Draw->AddLine(ImVec2(sa.x, sa.y), ImVec2(sb.x, sb.y), col, 1.f);
                    };

                    if (ghost.positions.count("UpperTorso") > 0) {
                        gbone("Head", "UpperTorso");
                        gbone("UpperTorso", "LowerTorso");
                        gbone("UpperTorso", "LeftUpperArm");
                        gbone("UpperTorso", "RightUpperArm");
                        gbone("LeftUpperArm", "LeftLowerArm");
                        gbone("RightUpperArm", "RightLowerArm");
                        gbone("LeftLowerArm", "LeftHand");
                        gbone("RightLowerArm", "RightHand");
                        gbone("LowerTorso", "LeftUpperLeg");
                        gbone("LowerTorso", "RightUpperLeg");
                        gbone("LeftUpperLeg", "LeftLowerLeg");
                        gbone("RightUpperLeg", "RightLowerLeg");
                        gbone("LeftLowerLeg", "LeftFoot");
                        gbone("RightLowerLeg", "RightFoot");
                    }
                    else if (ghost.positions.count("Torso") > 0) {
                        gbone("Head", "Torso");
                        gbone("Torso", "Left Arm");
                        gbone("Torso", "Right Arm");
                        gbone("Torso", "Left Leg");
                        gbone("Torso", "Right Leg");
                    }
                }
            }

            // ── Render ghost chams (type 3) ──
            if (Globals::Visuals::HitEffectsType == 3)
            {
                float* hc = Globals::Visuals::HitEffectsColor;

                for (auto& ghost : ghosts)
                {
                    float elapsed = now - ghost.time;
                    float alpha = 1.f - elapsed / hd;
                    if (alpha <= 0) continue;

                    int fill_a = (int)(alpha * hc[3] * 100);
                    int outline_a = (int)(alpha * hc[3] * 255);
                    ImU32 fill_col = IM_COL32((int)(hc[0]*255), (int)(hc[1]*255), (int)(hc[2]*255), fill_a);
                    ImU32 outline_col = IM_COL32((int)(hc[0]*255), (int)(hc[1]*255), (int)(hc[2]*255), outline_a);

                    for (auto& [part_name, world_pos] : ghost.positions)
                    {
                        if (part_name == "HumanoidRootPart") continue;
                        auto sz_it = ghost.sizes.find(part_name);
                        auto rot_it = ghost.rotations.find(part_name);
                        if (sz_it == ghost.sizes.end() || rot_it == ghost.rotations.end()) continue;

                        SDK::Vector3 sz = sz_it->second;
                        if (fabsf(sz.x) < 0.01f && fabsf(sz.y) < 0.01f && fabsf(sz.z) < 0.01f) continue;
                        SDK::Matrix3 rot = rot_it->second;

                        auto mid_it = ghost.meshIds.find(part_name);
                        if (mid_it != ghost.meshIds.end() && !mid_it->second.empty()) {
                            RenderPartMesh(Draw, world_pos, rot, mid_it->second, fill_col);
                        } else {
                            FallbackRenderPart(Draw, world_pos, sz, rot, fill_col, outline_col);
                        }
                    }
                }
            }

            // ── Render filled chams (type 4) ──
            if (Globals::Visuals::HitEffectsType == 4)
            {
                float* hc = Globals::Visuals::HitEffectsColor;

                for (auto& ghost : ghosts)
                {
                    float elapsed = now - ghost.time;
                    float alpha = 1.f - elapsed / hd;
                    if (alpha <= 0) continue;

                    int a = (int)(alpha * hc[3] * 255);
                    ImU32 fill_col = IM_COL32((int)(hc[0]*255), (int)(hc[1]*255), (int)(hc[2]*255), a/2);
                    ImU32 outline_col = IM_COL32((int)(hc[0]*255), (int)(hc[1]*255), (int)(hc[2]*255), a);

                    for (auto& [part_name, world_pos] : ghost.positions)
                    {
                        if (part_name == "HumanoidRootPart") continue;
                        auto sz_it = ghost.sizes.find(part_name);
                        auto rot_it = ghost.rotations.find(part_name);
                        if (sz_it == ghost.sizes.end() || rot_it == ghost.rotations.end()) continue;

                        SDK::Vector3 sz = sz_it->second;
                        if (fabsf(sz.x) < 0.01f && fabsf(sz.y) < 0.01f && fabsf(sz.z) < 0.01f) continue;
                        SDK::Matrix3 rot = rot_it->second;

                        auto mid_it = ghost.meshIds.find(part_name);
                        if (mid_it != ghost.meshIds.end() && !mid_it->second.empty()) {
                            RenderPartMesh(Draw, world_pos, rot, mid_it->second, fill_col);
                        } else {
                            FallbackRenderPart(Draw, world_pos, sz, rot, fill_col, outline_col);
                        }
                    }
                }
            }

            // ── Render shot tracers ──
            if (Globals::Visuals::ShotTracers && !tracers.empty())
            {
                float* tc = Globals::Visuals::ShotTracerColor;
                int cr = (int)(tc[0]*255), cg = (int)(tc[1]*255), cb = (int)(tc[2]*255);
                float fade_time = 0.5f;
                int stype = Globals::Visuals::ShotTracerType;

                for (auto& tr : tracers)
                {
                    float elapsed = now - tr.time;
                    float overall_alpha = 1.f;
                    float flight_time = tr.dist / 500.f;
                    if (flight_time < 0.1f) flight_time = 0.1f;

                    if (elapsed > flight_time) {
                        float fade = 1.f - (elapsed - flight_time) / fade_time;
                        if (fade <= 0.f) { tr.finished = true; continue; }
                        overall_alpha = fade;
                    }

                    int segments = 30;
                    ImVec2 prev_screen = {-1, -1};

                    for (int i = 0; i <= segments; i++)
                    {
                        float t = (std::min)(elapsed, flight_time) * (float)i / segments;
                        SDK::Vector3 pos = {
                            tr.origin.x + tr.direction.x * 500.f * t,
                            tr.origin.y + tr.direction.y * 500.f * t,
                            tr.origin.z + tr.direction.z * 500.f * t,
                        };

                        auto sp = Globals::VisualEngine.World_To_Screen(pos);
                        if (sp.x < -500) { prev_screen = {-1,-1}; continue; }

                        if (prev_screen.x > 0) {
                            float seg_t = (float)i / segments;
                            int alpha = (int)(seg_t * 255 * overall_alpha * tc[3]);

                            if (stype == 0) {
                                Draw->AddLine(ImVec2(prev_screen.x, prev_screen.y), ImVec2(sp.x, sp.y), IM_COL32(0,0,0,alpha/2), 3.f);
                                Draw->AddLine(ImVec2(prev_screen.x, prev_screen.y), ImVec2(sp.x, sp.y), IM_COL32(cr,cg,cb,alpha), 1.5f);
                            }
                            else if (stype == 1) {
                                Draw->AddLine(ImVec2(prev_screen.x, prev_screen.y), ImVec2(sp.x, sp.y), IM_COL32(cr,cg,cb,alpha/4), 7.f);
                                Draw->AddLine(ImVec2(prev_screen.x, prev_screen.y), ImVec2(sp.x, sp.y), IM_COL32(cr,cg,cb,alpha/2), 4.f);
                                Draw->AddLine(ImVec2(prev_screen.x, prev_screen.y), ImVec2(sp.x, sp.y), IM_COL32(cr,cg,cb,alpha), 1.5f);
                            }
                        }

                        if (stype == 2 && i % 3 == 0) {
                            float seg_t = (float)i / segments;
                            int alpha = (int)(seg_t * 255 * overall_alpha * tc[3]);
                            Draw->AddCircleFilled(ImVec2(sp.x, sp.y), 2.f, IM_COL32(0,0,0,alpha/2), 6);
                            Draw->AddCircleFilled(ImVec2(sp.x, sp.y), 1.5f, IM_COL32(cr,cg,cb,alpha), 6);
                        }

                        prev_screen = {sp.x, sp.y};
                    }
                }
            }
        }

        Draw->Flags = drawFlags;
    }
}