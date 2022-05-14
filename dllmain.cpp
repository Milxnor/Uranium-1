#include <Windows.h>
#include "Util.h"
#include "UE5.h"
#include <iostream>
#include "Functions.h"
#include "minhook/MinHook.h"
#include "GUI.h"

bool bHasSpawned = false;

#include "Hooks.h"
#include "StringUtils.h"

#pragma comment(lib, "minhook/minhook.lib")

DWORD WINAPI DumpObjectThread(LPVOID param)
{
    DumpObjects();
    system(("notepad \"Dump.txt\""));
    return NULL;
}

FVector LastEmoteLoc;
bool bIsEmoting;
UObject* CurrentEmote;
bool bIsPickingUp = false;
UObject* OldCheat;

void* (*PEOG)(void*, void*, void*);
void* ProcessEventDetour(UObject* pObject, UObject* pFunction, void* pParams)
{
    if (pObject && pFunction) {

        auto FuncName = pFunction->GetName();
        auto FullFuncName = pFunction->GetFullName();

        //printf("FuncName: %s\n", FuncName.c_str());

        if (FullFuncName.find(crypt("ServerExecuteInventoryItem")) != std::string::npos && FortInventory)
        {
            FGuid* guid = reinterpret_cast<FGuid*>(pParams);

            auto entries = reinterpret_cast<TArray<FFortItemEntry>*>(__int64(FortInventory) + __int64(Offsets::InventoryOffset) + __int64(Offsets::EntriesOffset));

            for (int i = 0; i < entries->Num(); i++)
            {
                auto entry = entries->operator[](i);
                auto entryGuid = reinterpret_cast<FGuid*>((uintptr_t)&entry + __int64(Offsets::ItemGuidOffset));

                if (IsMatchingGuid(*entryGuid, *guid))
                {
                    struct
                    {
                        UObject* WeaponData;
                        FGuid ItemEntryGuid;
                        UObject* ReturnValue;
                    } EquipWeaponDefinitionParams;

                    EquipWeaponDefinitionParams.WeaponData = *reinterpret_cast<UObject**>((uintptr_t)&entry + __int64(Offsets::ItemDefinitionOffset));
                    EquipWeaponDefinitionParams.ItemEntryGuid = *guid;

                    auto EquiptWeaponFunc = FindObject(crypt("Function /Script/FortniteGame.FortPawn.EquipWeaponDefinition"));
                    ProcessEvent((UObject*)Pawn, EquiptWeaponFunc, &EquipWeaponDefinitionParams);
                }
            }
        }

        if (FuncName.find(crypt("ServerAttemptInteract_Implementation")) != std::string::npos)
        {
            ShowMessage("attempted interaction");
            //CreateThread(0, 0, BuildingActorFunctions::BuildAsync, 0, 0, 0);
        }

        if (FuncName.find(crypt("ServerHandlePickup")) != std::string::npos && FortInventory)
        {
            if (!bIsPickingUp)
                bIsPickingUp = true;

            struct Params
            {
                UObject* PickUp;
                float InFlyTime;
                FVector InStartDirection;
                bool bPlayPickupSound;
            };
            auto params = (Params*)(pParams);

            if (params->PickUp == nullptr) {
                printf(crypt("Invalid pickup!\n"));
                return NULL;
            }

            auto primQuickbar = reinterpret_cast<FQuickBar*>((uintptr_t)QuickBar + __int64(Offsets::PrimQuickBarOffset));
            auto entries = reinterpret_cast<TArray<FFortItemEntry>*>(__int64(FortInventory) + static_cast<__int64>(Offsets::InventoryOffset) + static_cast<__int64>(Offsets::EntriesOffset));

            auto PickupEntry = reinterpret_cast<FFortItemEntry*>((uintptr_t)params->PickUp + __int64(Offsets::PickupEntryOffset));
            inventoryFunctions->AddItemToInventoryWithEntry(*PickupEntry, *(int*)((uintptr_t)PickupEntry + __int64(Offsets::EntryCountOffset)));

            actorFunctions->DestroyActor(params->PickUp);
        }

        if (FuncName.find(crypt("ServerAttemptInventoryDrop")) != std::string::npos && FortInventory)
        {
            if (bIsPickingUp) {
                bIsPickingUp = false;
                return NULL;
            }

            printf(crypt("Called remove item!\n"));

            struct Params
            {
                FGuid ItemGuid;
                int32_t Count;
                bool bTrash;
            };
            auto params = (Params*)(pParams);

            auto entries = reinterpret_cast<TArray<FFortItemEntry>*>(__int64(FortInventory) + static_cast<__int64>(Offsets::InventoryOffset) + static_cast<__int64>(Offsets::EntriesOffset));

            for (int i = 0; i < entries->Num(); i++)
            {
                auto entry = entries->operator[](i);
                auto entryGuid = reinterpret_cast<FGuid*>((uintptr_t)&entry + __int64(Offsets::ItemGuidOffset));
                auto entryItemDef = *reinterpret_cast<UObject**>((uintptr_t)&entry + __int64(Offsets::ItemDefinitionOffset));
                auto entryCount = *reinterpret_cast<int*>((uintptr_t)&entry + 0xC);

                if (IsMatchingGuid(params->ItemGuid, *entryGuid)) {
                    entries->Remove(i);
                    pickupFunctions->SpawnPickup(entryItemDef, params->Count, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::TossedByPlayer);

                    if (params->Count != entryCount)
                    {
                        inventoryFunctions->AddItemToInventory(entryItemDef, entryCount - params->Count);
                    }

                    inventoryFunctions->UpdateInventory();
                }
            }
        }

        if (FuncName.find(crypt("ServerReturnToMainMenu")) != std::string::npos)
        {
            auto CheatManager = reinterpret_cast<UObject**>((uintptr_t)Controller + __int64(FindOffset("PlayerController", "CheatManager")));
            *CheatManager = nullptr;
            Sleep(500);
            playerControllerFunctions->SwitchLevel(L"Frontend?Game=/Script/FortniteGame.FortGameModeFrontEnd");
        }

        if (FuncName.find(crypt("CheatScript")) != std::string::npos) {

            struct CheatScriptParams { struct FString ScriptName; UObject* ReturnValue; };
            auto params = reinterpret_cast<CheatScriptParams*>(pParams);
            auto fstring = params->ScriptName;
            auto string = params->ScriptName.ToString();
            auto strings = String::StringUtils::Split(string, " ");
            strings[0] = String::StringUtils::ToLower(strings[0]);

            if (strings[0] == crypt("dump")) {
                CreateThread(0, 0, DumpObjectThread, 0, 0, 0);
            }

            if (strings[0] == crypt("weapon")) {
                auto weapon = FindObject(strings[1] + "." + strings[1]);
                if (weapon == nullptr) {
                    gamemodeFunctions->Say(crypt(L"Failed to find weapon!\n"));
                    return NULL;
                }

                inventoryFunctions->AddItemToInventory(weapon, 1);
            }

            if (strings[0] == crypt("pickup")) {
                auto weapon = FindObject(strings[1] + "." + strings[1]);
                if (weapon == nullptr) {
                    gamemodeFunctions->Say(crypt(L"Failed to find pickup!\n"));
                    return NULL;
                }

                pickupFunctions->SpawnPickup(weapon, 1, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::Unset);
            }

            if (strings[0] == crypt("loadbp")) {
                auto BP = strings[1];
                StaticLoadObject(FindObject(crypt("Class /Script/Engine.BlueprintGeneratedClass")), nullptr, (std::wstring(BP.begin(), BP.end()).c_str()));
            }

            if (strings[0] == "play")
            {
                auto func = FindObject(crypt("Function /Script/MovieScene.MovieSceneSequencePlayer.Play"));
                auto obj = FindObject(std::string(strings[1].begin(), strings[1].end()));

                if (!obj)
                {
                    std::cout << crypt("Failed To Find Sequence") << std::endl;
                    return NULL;
                }

                ProcessEvent(obj, func, nullptr);
            }
        }

        if (FuncName.find("Tick") != std::string::npos)
        {
            if (GetAsyncKeyState(VK_F1) & 0x1 && bIsReady) {
                Functions::InitMatch();
            }

            if (GetAsyncKeyState(VK_INSERT) & 0x1) {
                bShowWindow = !bShowWindow;

                if (bShowWindow && bIsReady) {
                    ImGui::GetIO().MouseDrawCursor = true;
                }
                else {
                    ImGui::GetIO().MouseDrawCursor = false;
                }
            }
        }

        if (FuncName.find(crypt("ServerLoadingScreenDropped")) != std::string::npos)
        {
            Functions::UpdatePlayerController();
            kismetLibraryFunctions->ShowSkin();
            Functions::EnableCheatManager();
            Functions::DestroyAll(FindObject(crypt("Class /Script/FortniteGame.FortHLODSMActor")));

            gameplayAbilitiesFunctions->GrantGameplayAbility(Pawn, FindObject(crypt("Class /Script/FortniteGame.FortGameplayAbility_Sprint")));
            gameplayAbilitiesFunctions->GrantGameplayAbility(Pawn, FindObject(crypt("Class /Script/FortniteGame.FortGameplayAbility_Jump")));
            gameplayAbilitiesFunctions->GrantGameplayAbility(Pawn, FindObject(crypt("Class /Script/FortniteGame.FortGameplayAbility_Crouch")));
            gameplayAbilitiesFunctions->GrantGameplayAbility(Pawn, FindObject(crypt("Class /Script/FortniteGame.FortGameplayAbility_RangedWeapon")));
            gameplayAbilitiesFunctions->GrantGameplayAbility(Pawn, FindObject(crypt("Class /Script/FortniteGame.FortGameplayAbility_Reload")));
            gameplayAbilitiesFunctions->GrantGameplayAbility(Pawn, FindObject(crypt("BlueprintGeneratedClass /Game/Abilities/Player/Sliding/GA_Athena_Player_Slide.GA_Athena_Player_Slide_C")));
            gameplayAbilitiesFunctions->GrantGameplayAbility(Pawn, FindObject(crypt("BlueprintGeneratedClass /Game/Abilities/Player/Generic/Traits/DefaultPlayer/GA_DefaultPlayer_InteractSearch.GA_DefaultPlayer_InteractSearch_C")));
            gameplayAbilitiesFunctions->GrantGameplayAbility(Pawn, FindObject(crypt("BlueprintGeneratedClass /Game/Abilities/Player/Generic/Traits/DefaultPlayer/GA_DefaultPlayer_InteractUse.GA_DefaultPlayer_InteractUse_C")));
            gameplayAbilitiesFunctions->GrantGameplayAbility(Pawn, FindObject(crypt("BlueprintGeneratedClass /Game/Athena/DrivableVehicles/GA_AthenaEnterVehicle.GA_AthenaEnterVehicle_C")));
            gameplayAbilitiesFunctions->GrantGameplayAbility(Pawn, FindObject(crypt("BlueprintGeneratedClass /Game/Athena/DrivableVehicles/GA_AthenaExitVehicle.GA_AthenaExitVehicle_C")));
            gameplayAbilitiesFunctions->GrantGameplayAbility(Pawn, FindObject(crypt("BlueprintGeneratedClass /Game/Athena/DrivableVehicles/GA_AthenaInVehicle.GA_AthenaInVehicle_C")));
            gameplayAbilitiesFunctions->GrantGameplayAbility(Pawn, FindObject(crypt("BlueprintGeneratedClass /Game/Athena/Items/EnvironmentalItems/HidingProps/GA_Athena_HidingProp_JumpOut.GA_Athena_HidingProp_JumpOut_C")));
            gameplayAbilitiesFunctions->GrantGameplayAbility(Pawn, FindObject(crypt("BlueprintGeneratedClass /Game/Athena/Items/EnvironmentalItems/HidingProps/GA_Athena_HidingProp_Hide.GA_Athena_HidingProp_Hide_C")));
            gameplayAbilitiesFunctions->GrantGameplayAbility(Pawn, FindObject(crypt("BlueprintGeneratedClass /Game/Abilities/Player/Generic/Traits/DefaultPlayer/GA_DefaultPlayer_InteractUse.GA_DefaultPlayer_InteractUse_C")));

            FortInventory = reinterpret_cast<InventoryPointer*>((uintptr_t)Controller + __int64(FindOffset("FortPlayerController", "WorldInventory")))->Inventory;
            QuickBar = reinterpret_cast<QuickBarPointer*>((uintptr_t)Controller + __int64(FindOffset("FortPlayerController", "ClientQuickBars")))->QuickBar;

            printf("Pickaxe: %s\n", Functions::GetPickaxeDef()->GetFullName().c_str());

            inventoryFunctions->AddItemToInventory(Functions::GetPickaxeDef(), 1, true, EFortQuickBars::Primary, 0);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortBuildingItemDefinition /Game/Items/Weapons/BuildingTools/BuildingItemData_Wall.BuildingItemData_Wall")), 1);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortBuildingItemDefinition /Game/Items/Weapons/BuildingTools/BuildingItemData_Floor.BuildingItemData_Floor")), 1);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortBuildingItemDefinition /Game/Items/Weapons/BuildingTools/BuildingItemData_Stair_W.BuildingItemData_Stair_W")), 1);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortBuildingItemDefinition /Game/Items/Weapons/BuildingTools/BuildingItemData_RoofS.BuildingItemData_RoofS")), 1);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortResourceItemDefinition /Game/Items/ResourcePickups/WoodItemData.WoodItemData")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortResourceItemDefinition /Game/Items/ResourcePickups/StoneItemData.StoneItemData")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortResourceItemDefinition /Game/Items/ResourcePickups/MetalItemData.MetalItemData")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /ResolveGameplay/Items/RepairTool/AmmoDataResTorch.AmmoDataResTorch")), 999); // Torch Ammo
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AthenaOakCash.AthenaOakCash")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AthenaAmmoDataHooks.AthenaAmmoDataHooks")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AthenaAmmoDataFringePlank.AthenaAmmoDataFringePlank")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AthenaAmmoDataEnergyCell.AthenaAmmoDataEnergyCell")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AthenaAmmoDataBalloons.AthenaAmmoDataBalloons")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AthenaAmmo_Boss.AthenaAmmo_Boss")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AmmoInfiniteShells_Athena.AmmoInfiniteShells_Athena")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AmmoInfiniteCrossbow_Athena.AmmoInfiniteCrossbow_Athena")), 999);
            //inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AmmoInfinite.AmmoInfinite")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AmmoInfinite_NoIcon.AmmoInfinite_NoIcon")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AmmoDataRockets.AmmoDataRockets")), 999);
            inventoryFunctions->AddItemToInventory(FindObject(crypt("FortAmmoItemDefinition /Game/Athena/Items/Ammo/AmmoDataPetrol.AmmoDataPetrol")), 999);
            //playerControllerFunctions->SetInfiniteAmmo(Controller);
            gamestateFunctions->SetGamePhase(EAthenaGamePhase::None, EAthenaGamePhase::Warmup);
            pawnFunctions->TeleportToSkydive(60000);

            playerControllerFunctions->ServerSetClientHasFinishedLoading(Controller);

            auto bHasServerFinishedLoading = reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(Controller) + __int64(FindOffset("FortPlayerController", "bHasServerFinishedLoading")));
            *bHasServerFinishedLoading = true;

            Offsets::InitPreDefinedOffsets();
        }
    }

    return PEOG(pObject, pFunction, pParams);
}

DWORD WINAPI MainThread(LPVOID)
{
    Util::InitConsole();

    actorFunctions = new ActorFunctions;
    playerControllerFunctions = new PlayerControllerFunctions;
    kismetLibraryFunctions = new KismetLibraryFunctions;
    gameplayAbilitiesFunctions = new GameplayAblitiesFunctions;
    gamemodeFunctions = new GameModeFunctions;
    gamestateFunctions = new GameStateFunctions;
    cheatManagerFunctions = new CheatManagerFunctions;
    inventoryFunctions = new InventoryFunctions;
    pawnFunctions  = new PawnFunctions;
    buildingActorFunctions = new BuildingActorFunctions;
    pickupFunctions = new PickupFunctions;
    worldFunctions = new WorldFunctions;

    std::cout << "Finding Patterns!\n";

    auto pGObjects = Util::FindPattern(crypt("48 8B 05 ? ? ? ? 48 8B 0C C8 48 8B 04 D1"), true, 3);
    CHECKSIG(pGObjects, "Failed to find GObjects address!");
    GObjects = decltype(GObjects)(pGObjects);
    std::cout << "Found GObjects address!\n";

    auto pFNameToString = Util::FindPattern(crypt("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 8B"));
    CHECKSIG(pFNameToString, "Failed to find FNameToString address!");
    FNameToString = decltype(FNameToString)(pFNameToString);
    std::cout << "Found FNameToString address!\n";

    /*auto pFNameToString = Util::FindByString(L"%s %s SetTimer passed a negative or zero time. The associated timer may fail to be created/fire! If using InitialStartDelayVariance, be sure it is smaller than (Time + InitialStartDelay).", {CALL}, true, 1);
    CHECKSIG(pFNameToString, "Failed to find FNameToString address!");
    FNameToString = decltype(FNameToString)(pFNameToString);*/

    auto pFreeMemory = Util::FindPattern(crypt("48 85 C9 0F 84 ? ? ? ? 48 89 5C 24 ? 57 48 83 EC 20 48 8B 3D ? ? ? ? 48 8B D9 48"));
    CHECKSIG(pFreeMemory, "Failed to find FreeMemory address!");
    FreeMemory = decltype(FreeMemory)(pFreeMemory);
    std::cout << "Found FreeMemory address!\n";
    
    auto pWorld = Util::FindPattern(crypt("48 8B 05 ? ? ? ? 4D 8B C1"), true, 3);
    CHECKSIG(pWorld, "Failed to find UWorld address!");
    World = reinterpret_cast<UObject**>(pWorld);
    std::cout << "Found UWorld address!\n";

    auto FortEngine = FindObject(crypt("FortEngine /Engine/Transient.FortEngine"));
    auto FEVFT = *reinterpret_cast<void***>(FortEngine);
    auto PEAddr = FEVFT[0x4C];

    MH_Initialize();
    MH_CreateHook((void*)PEAddr, ProcessEventDetour, (void**)(&PEOG));
    MH_EnableHook((void*)PEAddr);

    InitHooks();

    Functions::UnlockConsole();
    Functions::UpdatePlayerController();

    SetupGUI();

    std::cout << "Setup!\n";

    return NULL;
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID res)
{
    if (reason == DLL_PROCESS_ATTACH)
        CreateThread(0, 0, MainThread, mod, 0, 0);

    return TRUE;
}
