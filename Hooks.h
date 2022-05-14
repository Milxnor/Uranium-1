#pragma once

#include <Windows.h>
#include <cstdio>
#include <cstdint>
#include "Util.h"
#include "minhook/MinHook.h"
#include "skCryptor.h"

#pragma comment(lib, "minhook/minhook.lib")

typedef void* (__fastcall* fExitBypass)(__int64 arg1);
typedef void* (__fastcall* fCrashBypass)(__int64 arg1, __int64 arg2);
typedef void* (__fastcall* fRequestExit)(bool force);
typedef void* (__fastcall* fRequestExitWithStatus)(bool unknown, bool force);
typedef void* (__fastcall* fNotiBypass)(wchar_t** arg1, unsigned __int8 arg2, __int64 arg3, char arg4);
fExitBypass ExitBypass;
fCrashBypass CrashBypass;
fRequestExit RequestExitBypass;
fNotiBypass NotiBypass;
fRequestExitWithStatus RequestExitWithStatusBypass;

void* __fastcall ExitBypassHook(__int64 a1)
{
    return NULL;
}

void* __fastcall CrashHook(__int64 a1, __int64 a2)
{
    return NULL;
}

void* __fastcall NotificationHook(wchar_t** a1, unsigned __int8 a2, __int64 a3, char a4)
{
    std::cout << "Tried to exit: " << a1 << std::endl;

    return NULL;
}

void RequestExitWithStatusHook(bool Force, uint8_t ReturnCode)
{
    printf(crypt("BFC Check: %i ReturnCode: %u\n"), Force, ReturnCode);

    if (bIsReady && !bHasSpawned)
    {
        bHasSpawned = true;
        Functions::InitMatch();
    }
}

static void InitHooks()
{
    /*auto NotificationHookAddress = Util::FindPattern(crypt("4C 8B DC 55 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 89 5B 20"));
    if (!NotificationHookAddress) {
        NotificationHookAddress = Util::FindPattern(crypt("4C 8B DC 55 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 89 73 F0 49 89 7B E8 48 8B F9 4D 89 63 E0 4D 8B E0 4D 89 6B D8"));
    }*/

    auto RequestExitWithStatusAddress = Util::FindPattern(crypt("48 8B C4 48 89 58 18 88 50 10 88 48 08"));
    if (!RequestExitWithStatusAddress) {
        RequestExitWithStatusAddress = Util::FindPattern(crypt("48 89 5C 24 ? 57 48 83 EC 40 41 B9 ? ? ? ? 0F B6 F9 44 38 0D ? ? ? ? 0F B6 DA 72 24 89 5C 24 30 48 8D 05 ? ? ? ? 89 7C 24 28 4C 8D 05 ? ? ? ? 33 D2 48 89 44 24 ? 33 C9 E8 ? ? ? ?"));
    }

    //CHECKSIG(NotificationHookAddress, crypt("Failed to find Notification address!"));
    CHECKSIG(RequestExitWithStatusAddress, crypt("Failed to find RequestExitWithStatus address"));

    MH_CreateHook(static_cast<LPVOID>((LPVOID)RequestExitWithStatusAddress), RequestExitWithStatusHook, reinterpret_cast<LPVOID*>(&RequestExitWithStatusBypass));
    MH_EnableHook(static_cast<LPVOID>((LPVOID)RequestExitWithStatusAddress));
    /*MH_CreateHook(static_cast<LPVOID>((LPVOID)NotificationHookAddress), ExitBypassHook, reinterpret_cast<LPVOID*>(&NotiBypass));
    MH_EnableHook(static_cast<LPVOID>((LPVOID)NotificationHookAddress));*/
}
