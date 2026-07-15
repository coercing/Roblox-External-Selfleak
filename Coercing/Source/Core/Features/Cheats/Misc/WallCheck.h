#pragma once
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <atomic>
#include "Engine/Engine.h"

// ─────────────────────────────────────────────────────────────────────────────
//  WallCheck — primitive-pointer-walk visibility system
//
//  Usage:
//    wallcheck::initialize();                          // call once at startup
//    wallcheck::on_game_changed();                     // call when GameID changes
//    bool vis = wallcheck::is_visible(from, to);       // main query
//    wallcheck::shutdown();                            // call on exit
// ─────────────────────────────────────────────────────────────────────────────

namespace wallcheck {

    struct CachedWall {
        SDK::Vector3 Position;
        SDK::Vector3 Size;
        SDK::Matrix3 Rotation;
    };

    // ── Ray result cache ─────────────────────────────────────────────────────
    class OcclusionCache {
    public:
        OcclusionCache();
        bool Get(const SDK::Vector3& from, const SDK::Vector3& to, bool& out);
        void Set(const SDK::Vector3& from, const SDK::Vector3& to, bool blocked);
        void Clear();
    private:
        uint64_t Hash(const SDK::Vector3& from, const SDK::Vector3& to) const;
        struct Entry { bool blocked; std::chrono::steady_clock::time_point ts; };
        std::mutex                          mutex_;
        std::unordered_map<uint64_t, Entry> map_;
        std::chrono::milliseconds           lifetime_{ 200 };
        size_t                              max_size_{ 8000 };
    };

    // ── Main checker ─────────────────────────────────────────────────────────
    class WallChecker {
    public:
        WallChecker();
        ~WallChecker();

        void start();
        void stop();

        // Trigger an immediate rescan (called by Cache when GameID changes)
        void notify_game_changed();

        // Primary visibility query used by aimbot / silent aim
        bool is_visible(const SDK::Vector3& from, const SDK::Vector3& to,
                        std::uint64_t ignoreModel = 0, bool includePlayers = false);

        size_t part_count() const;

    private:
        void  scan_loop();
        void  scan();
        bool  is_in_game() const;
        bool  ray_vs_obb(const SDK::Vector3& origin, const SDK::Vector3& end,
                         const CachedWall& wall) const;

        std::vector<CachedWall>       walls_;
        mutable std::shared_mutex     walls_mutex_;
        std::unique_ptr<OcclusionCache> cache_;

        std::thread       thread_;
        std::atomic<bool> stop_{ false };
        std::atomic<bool> running_{ false };
        std::atomic<bool> rescan_requested_{ false };
    };

    // ── Module API ───────────────────────────────────────────────────────────
    extern std::unique_ptr<WallChecker> g_checker;

    void   initialize();
    void   shutdown();
    void   on_game_changed();   // called by Cache::Rescan when GameID changes

    // Direct visibility query — use this everywhere instead of MapParser
    inline bool is_visible(const SDK::Vector3& from, const SDK::Vector3& to,
                           std::uint64_t ignoreModel = 0, bool includePlayers = false) {
        if (!g_checker) return true;
        return g_checker->is_visible(from, to, ignoreModel, includePlayers);
    }
}
