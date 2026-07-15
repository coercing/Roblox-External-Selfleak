#include "WallCheck.h"
#include <Globals.hxx>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include "Core/Features/Cheats/Visuals/Visuals.h"
#include "Core/Features/Cache/Cache.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

// ─────────────────────────────────────────────────────────────────────────────
//  Internal helpers
// ─────────────────────────────────────────────────────────────────────────────
static bool valid_vec(const SDK::Vector3& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

static bool valid_addr(uintptr_t a) {
    return a > 0x1000 && a != ~0ULL;
}

namespace wallcheck {

// ─────────────────────────────────────────────────────────────────────────────
//  OcclusionCache
// ─────────────────────────────────────────────────────────────────────────────

    OcclusionCache::OcclusionCache() {}

    uint64_t OcclusionCache::Hash(const SDK::Vector3& f, const SDK::Vector3& t) const {
        auto q = [](float v) -> uint32_t { return static_cast<uint32_t>(std::round(v * 10.f)); };
        uint64_t h1 = (static_cast<uint64_t>(q(f.x)) << 32) | q(f.y);
        uint64_t h2 = (static_cast<uint64_t>(q(f.z)) << 32) | q(t.x);
        uint64_t h3 = (static_cast<uint64_t>(q(t.y)) << 32) | q(t.z);
        return h1 ^ (h2 * 2654435761ULL) ^ (h3 * 2246822519ULL);
    }

    bool OcclusionCache::Get(const SDK::Vector3& from, const SDK::Vector3& to, bool& out) {
        std::lock_guard<std::mutex> lk(mutex_);
        auto it = map_.find(Hash(from, to));
        if (it == map_.end()) return false;
        if (std::chrono::steady_clock::now() - it->second.ts >= lifetime_) {
            map_.erase(it);
            return false;
        }
        out = it->second.blocked;
        return true;
    }

    void OcclusionCache::Set(const SDK::Vector3& from, const SDK::Vector3& to, bool blocked) {
        std::lock_guard<std::mutex> lk(mutex_);
        if (map_.size() >= max_size_) map_.clear();
        map_[Hash(from, to)] = { blocked, std::chrono::steady_clock::now() };
    }

    void OcclusionCache::Clear() {
        std::lock_guard<std::mutex> lk(mutex_);
        map_.clear();
    }

// ─────────────────────────────────────────────────────────────────────────────
//  WallChecker — in-game guard
// ─────────────────────────────────────────────────────────────────────────────

    bool WallChecker::is_in_game() const {
        if (!Globals::Workspace.Address) return false;
        if (!Globals::Players.Address)   return false;
        if (!Globals::GameID)            return false;
        try {
            auto ch = Globals::Players.Children();
            if (ch.empty()) return false;
        } catch (...) { return false; }
        return true;
    }

// ─────────────────────────────────────────────────────────────────────────────
//  Primitive-pointer-walk scan
//  Chain: Workspace +0x400 -> World, World +0x248 -> primitive array base
// ─────────────────────────────────────────────────────────────────────────────

    void WallChecker::scan() {
        if (!is_in_game()) return;

        // Collect player primitives to exclude from wall list
        std::unordered_set<uintptr_t> playerPrims;
        try {
            for (const auto& plr : Globals::Player_Cache) {
                auto collect = [&](const SDK::Instance& inst) {
                    if (!valid_addr(inst.Address)) return;
                    uintptr_t prim = Driver->Read<uintptr_t>(
                        inst.Address + Offsets::BasePart::Primitive);
                    if (valid_addr(prim)) playerPrims.insert(prim);
                };
                collect(plr.Head);
                collect(plr.HumanoidRootPart);
                collect(plr.UpperTorso);
                collect(plr.LowerTorso);
                collect(plr.Torso);
            }
        } catch (...) {}

        // Resolve primitive array base
        uintptr_t worldAddr = 0, primBase = 0;
        try {
            worldAddr = Driver->Read<uintptr_t>(
                Globals::Workspace.Address + Offsets::Workspace::World);
            if (!valid_addr(worldAddr)) return;
            primBase = Driver->Read<uintptr_t>(
                worldAddr + Offsets::World::Primitives);
            if (!valid_addr(primBase)) return;
        } catch (...) { return; }

        std::vector<CachedWall> fresh;
        fresh.reserve(32000);

        constexpr int kMaxSlots   = 0xFFFFFF;
        constexpr int kMaxWalls   = 80000;
        constexpr int kMaxInvalid = 1000;
        int invalidRun = 0;

        for (int i = 0; i < kMaxSlots && (int)fresh.size() < kMaxWalls; i += sizeof(uintptr_t)) {
            if (stop_.load() || rescan_requested_.load()) break;

            uintptr_t prim = 0;
            try { prim = Driver->Read<uintptr_t>(primBase + i); }
            catch (...) { break; }

            if (!valid_addr(prim)) {
                if (++invalidRun > kMaxInvalid) break;
                continue;
            }
            invalidRun = 0;

            if (playerPrims.count(prim)) continue;

            try {
                if (Driver->Read<int>(prim + 0x8) != Offsets::Primitive::Validate)
                    continue;

                uint8_t flags = Driver->Read<uint8_t>(prim + Offsets::Primitive::Flags);
                if (!(flags & Offsets::PrimitiveFlags::Anchored))  continue;
                if (!(flags & Offsets::PrimitiveFlags::CanCollide)) continue;
                if (!(flags & Offsets::PrimitiveFlags::CanQuery))  continue;

                SDK::Vector3 size = Driver->Read<SDK::Vector3>(prim + Offsets::Primitive::Size);
                if (!valid_vec(size)) continue;
                if (size.x < 0.5f && size.y < 0.5f && size.z < 0.5f) continue;
                if (size.x > 2048.f || size.y > 2048.f || size.z > 2048.f) continue;

                SDK::Vector3 pos = Driver->Read<SDK::Vector3>(prim + Offsets::Primitive::Position);
                if (!valid_vec(pos)) continue;
                if (pos.x == 0.f && pos.y == 0.f && pos.z == 0.f) continue;
                float distSq = pos.x*pos.x + pos.y*pos.y + pos.z*pos.z;
                if (distSq < 25.f) continue;

                SDK::Matrix3 rot = Driver->Read<SDK::Matrix3>(prim + Offsets::Primitive::Rotation);

                fresh.push_back({ pos, size, rot });
            } catch (...) { continue; }
        }

        {
            std::unique_lock<std::shared_mutex> lk(walls_mutex_);
            walls_ = std::move(fresh);
        }
        cache_->Clear();
    }

// ─────────────────────────────────────────────────────────────────────────────
//  Background scan loop — 15s periodic + immediate on notify_game_changed()
// ─────────────────────────────────────────────────────────────────────────────

    void WallChecker::scan_loop() {
        while (!stop_.load()) {
            if (Cache::Reattaching.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                continue;
            }
            if (rescan_requested_.exchange(false) || is_in_game()) {
                scan();
            }
            for (int i = 0; i < 50 && !stop_.load() && !rescan_requested_.load(); ++i)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

// ─────────────────────────────────────────────────────────────────────────────
//  OBB ray-segment intersection
// ─────────────────────────────────────────────────────────────────────────────

    bool WallChecker::ray_vs_obb(const SDK::Vector3& rayOrigin,
                                  const SDK::Vector3& rayEnd,
                                  const CachedWall&   wall) const {
        SDK::Vector3 dir = rayEnd - rayOrigin;
        float len = dir.magnitude();
        if (len < 1e-6f) return false;
        SDK::Vector3 unitDir = dir / len;

        // Manual transpose (no Transpose() method on SDK::Matrix3)
        const float* m = wall.Rotation.data;
        SDK::Matrix3 inv;
        inv.data[0]=m[0]; inv.data[1]=m[3]; inv.data[2]=m[6];
        inv.data[3]=m[1]; inv.data[4]=m[4]; inv.data[5]=m[7];
        inv.data[6]=m[2]; inv.data[7]=m[5]; inv.data[8]=m[8];

        SDK::Vector3 relO  = rayOrigin - wall.Position;
        SDK::Vector3 lO    = inv * relO;
        SDK::Vector3 lD    = inv * unitDir;

        if (!valid_vec(lO) || !valid_vec(lD)) return false;

        constexpr float eps = 1e-6f;
        float hx = wall.Size.x * 0.5f - eps;
        float hy = wall.Size.y * 0.5f - eps;
        float hz = wall.Size.z * 0.5f - eps;
        float tmin = 0.f, tmax = len;

        // X
        if (std::abs(lD.x) < eps) {
            if (lO.x < -hx || lO.x > hx) return false;
        } else {
            float t1 = (-hx - lO.x) / lD.x, t2 = (hx - lO.x) / lD.x;
            if (t1 > t2) std::swap(t1, t2);
            if (!std::isfinite(t1) || !std::isfinite(t2)) return false;
            tmin = tmin > t1 ? tmin : t1;
            tmax = tmax < t2 ? tmax : t2;
            if (tmin > tmax + eps) return false;
        }
        // Y
        if (std::abs(lD.y) < eps) {
            if (lO.y < -hy || lO.y > hy) return false;
        } else {
            float t1 = (-hy - lO.y) / lD.y, t2 = (hy - lO.y) / lD.y;
            if (t1 > t2) std::swap(t1, t2);
            if (!std::isfinite(t1) || !std::isfinite(t2)) return false;
            tmin = tmin > t1 ? tmin : t1;
            tmax = tmax < t2 ? tmax : t2;
            if (tmin > tmax + eps) return false;
        }
        // Z
        if (std::abs(lD.z) < eps) {
            if (lO.z < -hz || lO.z > hz) return false;
        } else {
            float t1 = (-hz - lO.z) / lD.z, t2 = (hz - lO.z) / lD.z;
            if (t1 > t2) std::swap(t1, t2);
            if (!std::isfinite(t1) || !std::isfinite(t2)) return false;
            tmin = tmin > t1 ? tmin : t1;
            tmax = tmax < t2 ? tmax : t2;
            if (tmin > tmax + eps) return false;
        }

        return tmax >= -eps && tmin <= len + eps;
    }

// ─────────────────────────────────────────────────────────────────────────────
//  Public visibility query
// ─────────────────────────────────────────────────────────────────────────────

    bool WallChecker::is_visible(const SDK::Vector3& from, const SDK::Vector3& to,
                                  std::uint64_t ignoreModel, bool includePlayers) {
        if (!valid_vec(from) || !valid_vec(to)) return true;

        if (!includePlayers) {
            bool cached;
            if (cache_->Get(from, to, cached)) return !cached;
        }

        {
            std::shared_lock<std::shared_mutex> lk(walls_mutex_);
            for (const auto& w : walls_) {
                if (ray_vs_obb(from, to, w)) {
                    if (!includePlayers) cache_->Set(from, to, true);
                    return false;
                }
            }
        }

        if (includePlayers) {
            const auto snap = Globals::Player_Cache;
            for (const auto& plr : snap) {
                if (plr.Character.Address == ignoreModel) continue;
                if (plr.Health <= 0.f) continue;
                auto bones = Visuals::Get_Bones(plr);
                for (const auto* bone : bones) {
                    if (!bone || !bone->Address) continue;
                    try {
                        SDK::Part part(bone->Address);
                        SDK::Part prim = part.Get_Primitive();
                        if (!prim.Address) continue;
                        CachedWall tmp{ prim.Get_Position(), prim.Get_Size(), prim.Get_Rotation() };
                        if (ray_vs_obb(from, to, tmp)) return false;
                    } catch (...) {}
                }
            }
        }

        if (!includePlayers) cache_->Set(from, to, false);
        return true;
    }

    size_t WallChecker::part_count() const {
        std::shared_lock<std::shared_mutex> lk(walls_mutex_);
        return walls_.size();
    }

    void WallChecker::notify_game_changed() {
        rescan_requested_.store(true);
        cache_->Clear();
    }

// ─────────────────────────────────────────────────────────────────────────────
//  Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

    WallChecker::WallChecker() : cache_(std::make_unique<OcclusionCache>()) {}
    WallChecker::~WallChecker() { stop(); }

    void WallChecker::start() {
        if (running_.exchange(true)) return;
        stop_.store(false);
        thread_ = std::thread(&WallChecker::scan_loop, this);
    }

    void WallChecker::stop() {
        if (!running_.load()) return;
        stop_.store(true);
        if (thread_.joinable()) thread_.join();
        running_.store(false);
        cache_->Clear();
    }

// ─────────────────────────────────────────────────────────────────────────────
//  Module API
// ─────────────────────────────────────────────────────────────────────────────

    std::unique_ptr<WallChecker> g_checker;

    void initialize() {
        if (!g_checker) {
            g_checker = std::make_unique<WallChecker>();
            g_checker->start();
        }
    }

    void shutdown() {
        if (g_checker) {
            g_checker->stop();
            g_checker.reset();
        }
    }

    void on_game_changed() {
        if (g_checker) g_checker->notify_game_changed();
    }

} // namespace wallcheck
