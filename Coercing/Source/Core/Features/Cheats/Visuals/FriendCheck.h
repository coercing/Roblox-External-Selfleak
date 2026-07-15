#pragma once
#include <cstdint>

enum class FriendStatus : uint32_t {
    LocalPlayer = 0,
    NotFriends = 1,
    Friends = 2,
    PendingFriendRequest = 3,
};

FriendStatus GetFriendStatus(int64_t otherUserId);
