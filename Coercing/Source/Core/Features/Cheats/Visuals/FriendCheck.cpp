#include "FriendCheck.h"
#include "Globals.hxx"
#include "Engine/Engine.h"
#include "Driver/Driver.h"

struct FriendNode {
    uintptr_t prev;
    uintptr_t next;
    int64_t   userId;
    int64_t   otherId;
    uint32_t  status;
};

FriendStatus GetFriendStatus(int64_t otherUserId) {
    SDK::Instance FriendService = Globals::Datamodel.Find_First_Child_Of_Class("FriendService");
    if (!FriendService.Address) return FriendStatus::NotFriends;

    SDK::Players LocalPlayerInstance = Globals::Players.Get_Local_Player();
    int64_t LocalUserId = static_cast<int64_t>(LocalPlayerInstance.Get_UserID());
    if (LocalUserId == otherUserId) return FriendStatus::LocalPlayer;
    if (LocalUserId < 0 || otherUserId < 0) return FriendStatus::NotFriends;

    uint64_t mask = Driver->Read<uint64_t>(FriendService.Address + 0x2A0);
    uintptr_t bucketBase = Driver->Read<uintptr_t>(FriendService.Address + 0x288);
    uintptr_t myCock = Driver->Read<uintptr_t>(FriendService.Address + 0x278);

    uint64_t hash = (static_cast<uint64_t>(LocalUserId) + 0x9E3779B9LL);
    hash ^= static_cast<uint64_t>(otherUserId) + ((hash << 6) + (hash >> 2) + 0x9E3779B9LL);
    size_t index = (mask & hash);
    uintptr_t bucket = bucketBase + (index * 0x10);

    uintptr_t firstNode = Driver->Read<uintptr_t>(bucket + 0x8);
    if (firstNode == myCock || firstNode == 0) return FriendStatus::LocalPlayer;

    uintptr_t end = Driver->Read<uintptr_t>(bucket + 0x0);
    uintptr_t current = firstNode;

    while (current != 0 && current != myCock) {
        FriendNode node = Driver->Read<FriendNode>(current);
        if (node.userId == LocalUserId && node.otherId == otherUserId)
            return static_cast<FriendStatus>(node.status);
        if (current == end) break;
        current = node.next;
    }

    return FriendStatus::NotFriends;
}
