#pragma once

#include "UE5.h"
#include "OffSetTable.hpp"
#include "skCryptor.h"
#include "minhook/MinHook.h"

#pragma comment(lib, "minhook/minhook.lib")

UObject* Controller;
UObject** World;
UObject* Pawn;
UObject* GameState;
PVOID LocalPawn;
UObject* QuickBar;
UObject* FortInventory;

std::vector<UObject*> LevelActors;

#define RELATIVE_ADDR(addr, size) ((PBYTE)((UINT_PTR)(addr) + *(PINT)((UINT_PTR)(addr) + ((size) - sizeof(INT))) + (size)))
#define ReadPointer(base, offset) (*(PVOID *)(((PBYTE)base + offset)))

static bool IsMatchingGuid(FGuid A, FGuid B)
{
	return A.A == B.A && A.B == B.B && A.C == B.C && A.D == B.D;
}

class ActorFunctions* actorFunctions;
class PlayerControllerFunctions* playerControllerFunctions;
class KismetLibraryFunctions* kismetLibraryFunctions;
class GameplayAblitiesFunctions* gameplayAbilitiesFunctions;
class GameModeFunctions* gamemodeFunctions;
class GameStateFunctions* gamestateFunctions;
class CheatManagerFunctions* cheatManagerFunctions;
class InventoryFunctions* inventoryFunctions;
class PawnFunctions* pawnFunctions;
class BuildingActorFunctions* buildingActorFunctions;
class WorldFunctions* worldFunctions;
class PickupFunctions* pickupFunctions;

class ActorFunctions
{
public:
	FVector GetActorLocation(UObject* Actor)
	{
		static auto fn = FindObject(crypt("Function /Script/Engine.Actor.K2_GetActorLocation"));

		struct Params
		{
			FVector ReturnValue;
		};
		Params params;

		ProcessEvent(Actor, fn, &params);

		return params.ReturnValue;
	}

	void DestroyActor(UObject* actor)
	{
		const auto fn = FindObject(crypt("Function /Script/Engine.Actor.K2_DestroyActor"));

		ProcessEvent(actor, fn, nullptr);
	}

	void SetOwner(UObject* TargetActor, UObject* NewOwner)
	{
		static UObject* SetOwner = FindObject(crypt("Function /Script/Engine.Actor.SetOwner"));
		ProcessEvent(TargetActor, SetOwner, &NewOwner);
	}
};

class PlayerControllerFunctions
{
public:
	void SwitchLevel(FString URL)
	{
		auto fn = FindObject(crypt("Function /Script/Engine.PlayerController.SwitchLevel"));
		ProcessEvent(Controller, fn, &URL);
	}

	void SetInfiniteAmmo(UObject* InController)
	{
		RBitField* BitField = reinterpret_cast<RBitField*>(__int64(InController) + __int64(FindOffset("FortPlayerController", "bInfiniteAmmo")));
		BitField->C = 1;
	}

	void ServerReadyToStartMatch()
	{
		auto fn = FindObject(crypt("Function /Script/FortniteGame.FortPlayerController.ServerReadyToStartMatch"));
		ProcessEvent(Controller, fn, nullptr);
	}

	void Possess(UObject* Pawn)
	{
		auto fn = FindObject(crypt("Function /Script/Engine.Controller.Possess"));
		ProcessEvent(Controller, fn, &Pawn);
	}

	void ServerSetClientHasFinishedLoading(UObject* Target)
	{
		static UObject* ServerSetClientHasFinishedLoading = FindObject(crypt("Function /Script/FortniteGame.FortPlayerController.ServerSetClientHasFinishedLoading"));

		bool HasFinishedLoading = true;

		ProcessEvent(Target, ServerSetClientHasFinishedLoading, &HasFinishedLoading);
	}
};

class KismetLibraryFunctions
{
public:
	void OverridePawnCosmetic(UObject* InFortPlayerPawn, std::string CosmeticFullName)
	{
		auto Cosmetic = FindObject(CosmeticFullName);

		struct
		{
			UObject* Pawn;
			UObject* Cosmetic;
		} params;

		params.Pawn = InFortPlayerPawn;
		params.Cosmetic = Cosmetic;

		auto Default__FortItemAndVariantSwapHelpers = FindObject("Default__FortItemAndVariantSwapHelpers");

		ProcessEvent(Default__FortItemAndVariantSwapHelpers, FindObject("Function /Script/FortniteGame.FortItemAndVariantSwapHelpers.PushCosmeticOverrideOntoPawn"), &params);
	}

	void ShowSkin() {
		OverridePawnCosmetic(Pawn, "AthenaCharacterItemDefinition /Game/Heroes/Commando/CosmeticCharacterItemDefinitions/CID_Commando_016_F_V1.CID_Commando_016_F_V1");

		struct UFortKismetLibrary_UpdatePlayerCustomCharacterPartsVisualization_Params
		{
			UObject* PlayerState;
		};
		auto PlayerState = *reinterpret_cast<UObject**>(__int64(Pawn) + __int64(FindOffset("Pawn", "PlayerState")));
		auto KismetLib = FindObject(crypt("FortKismetLibrary /Script/FortniteGame.Default__FortKismetLibrary"));
		static auto fn = FindObject(crypt("Function /Script/FortniteGame.FortKismetLibrary.UpdatePlayerCustomCharacterPartsVisualization"));

		UFortKismetLibrary_UpdatePlayerCustomCharacterPartsVisualization_Params params;
		params.PlayerState = PlayerState;
		ProcessEvent(KismetLib, fn, &params);
	}

	UObject* FindAthenaGameMode()
	{
		auto GameplayStatics = FindObject(crypt("GameplayStatics /Script/Engine.Default__GameplayStatics"));
		auto getgamemode = FindObject(crypt("Function /Script/Engine.GameplayStatics.GetGameMode"));
		struct UGameplayStatics_GetGameMode_Params
		{
			class UObject* WorldContextObject;                                       // (ConstParm, Parm, ZeroConstructor, IsPlainOldData)
			class UObject* ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData)
		};

		UGameplayStatics_GetGameMode_Params ggmparams;
		ggmparams.WorldContextObject = Controller;

		ProcessEvent(GameplayStatics, getgamemode, &ggmparams);

		return ggmparams.ReturnValue;
	}

	UObject* FindAthenaGameState()
	{
		auto dWorld = Util::FindPattern(crypt("48 8B 05 ? ? ? ? 4D 8B C1"), true, 3);
		CHECKSIG(dWorld, "Failed to find UWorld address!");
		auto Worldd = *reinterpret_cast<UObject**>(dWorld);

		struct UGameplayStatics_GetGameState_Params
		{
			class UObject* WorldContextObject;
			class UObject* ReturnValue;
		};

		auto GameplayStatics = FindObject(crypt("GameplayStatics /Script/Engine.Default__GameplayStatics"));
		auto GetGameState = FindObject(crypt("Function /Script/Engine.GameplayStatics.GetGameState"));


		UGameplayStatics_GetGameState_Params ggsparams;
		ggsparams.WorldContextObject = Worldd;

		ProcessEvent(GameplayStatics, GetGameState, &ggsparams);

		return ggsparams.ReturnValue;
	}

	TArray<UObject*> GetAllActorsOfClass(UObject* ActorClass)
	{
		auto fn = FindObject("Function /Script/Engine.GameplayStatics.GetAllActorsOfClass");
		auto gps = FindObject("Class /Script/Engine.GameplayStatics");

		struct
		{
			UObject* WorldContextObject;
			UObject* ActorClass;
			TArray<UObject*> OutActors;
		}params;
		params.WorldContextObject = (*World);
		params.ActorClass = ActorClass;

		ProcessEvent(gps, fn, &params);

		return params.OutActors;
	}
};

class GameplayAblitiesFunctions
{
public:
	void BP_ApplyGameplayEffectToSelf(UObject* AbilitySystemComponent, UObject* GameplayEffectClass)
	{
		static UObject* BP_ApplyGameplayEffectToSelf = FindObject(crypt("Function /Script/GameplayAbilities.AbilitySystemComponent.BP_ApplyGameplayEffectToSelf"));

		struct
		{
			UObject* GameplayEffectClass;
			float Level;
			FGameplayEffectContextHandle EffectContext;
			FActiveGameplayEffectHandle ret;
		} Params;

		Params.EffectContext = FGameplayEffectContextHandle();
		Params.GameplayEffectClass = GameplayEffectClass;
		Params.Level = 1.0;

		ProcessEvent(AbilitySystemComponent, BP_ApplyGameplayEffectToSelf, &Params);
	}

	void GrantGameplayAbility(UObject* TargetPawn, UObject* GameplayAbilityClass)
	{
		UObject** AbilitySystemComponent = reinterpret_cast<UObject**>(__int64(TargetPawn) + __int64(FindOffset("FortPawn", "AbilitySystemComponent")));
		static UObject* DefaultGameplayEffect = FindObject(crypt("GE_Athena_PurpleStuff_C /Game/Athena/Items/Consumables/PurpleStuff/GE_Athena_PurpleStuff.Default__GE_Athena_PurpleStuff_C"));
		if (!DefaultGameplayEffect)
		{
			DefaultGameplayEffect = FindObject(crypt("GE_Athena_PurpleStuff_Health_C /Game/Athena/Items/Consumables/PurpleStuff/GE_Athena_PurpleStuff_Health.Default__GE_Athena_PurpleStuff_Health_C"));
		}

		TArray<struct FGameplayAbilitySpecDef>* GrantedAbilities = reinterpret_cast<TArray<struct FGameplayAbilitySpecDef>*>(__int64(DefaultGameplayEffect) + __int64(FindOffset("GameplayEffect", "GrantedAbilities")));

		GrantedAbilities->operator[](0).Ability = GameplayAbilityClass;

		*reinterpret_cast<EGameplayEffectDurationType*>(__int64(DefaultGameplayEffect) + __int64(FindOffset("GameplayEffect", "DurationPolicy"))) = EGameplayEffectDurationType::Infinite;

		static auto GameplayEffectClass = FindObject(crypt("BlueprintGeneratedClass /Game/Athena/Items/Consumables/PurpleStuff/GE_Athena_PurpleStuff.GE_Athena_PurpleStuff_C"));
		if (!GameplayEffectClass)
		{
			GameplayEffectClass = FindObject(crypt("BlueprintGeneratedClass /Game/Athena/Items/Consumables/PurpleStuff/GE_Athena_PurpleStuff_Health.GE_Athena_PurpleStuff_Health_C"));
		}
		BP_ApplyGameplayEffectToSelf(*AbilitySystemComponent, GameplayEffectClass);
	}
};

class GameModeFunctions
{
public:
	void StartMatch()
	{
		auto fn = FindObject(crypt("Function /Script/Engine.GameMode.StartMatch"));
		ProcessEvent(kismetLibraryFunctions->FindAthenaGameMode(), fn, nullptr);
	}

	void Say(FString message)
	{
		static auto fn = FindObject(crypt("Function /Script/Engine.GameMode.Say"));
		ProcessEvent(kismetLibraryFunctions->FindAthenaGameMode(), fn, &message);
	}
};

class GameStateFunctions
{
public:
	void SetGamePhase(EAthenaGamePhase NewPhase, EAthenaGamePhase OldPhase)
	{
		EAthenaGamePhase* CurrentGamePhase = reinterpret_cast<EAthenaGamePhase*>(__int64(kismetLibraryFunctions->FindAthenaGameState()) + __int64(FindOffset("FortGameStateAthena", "GamePhase")));
		*CurrentGamePhase = NewPhase;

		static UObject* OnRep_GamePhase = FindObject(crypt("Function /Script/FortniteGame.FortGameStateAthena.OnRep_GamePhase"));
		ProcessEvent(kismetLibraryFunctions->FindAthenaGameState(), OnRep_GamePhase, &OldPhase);
	}

	void SetPlaylist(UObject* Playlist)
	{
		auto bSkipAircraft = reinterpret_cast<bool*>((uintptr_t)Playlist + __int64(FindOffset("FortPlaylistAthena", "bSkipAircraft")));
		*bSkipAircraft = false;

		auto bSkipWarmup = reinterpret_cast<bool*>((uintptr_t)Playlist + __int64(FindOffset("FortPlaylistAthena", "bSkipWarmup")));
		*bSkipWarmup = false;

		auto Gamestate = kismetLibraryFunctions->FindAthenaGameState();
		auto BasePlaylist = reinterpret_cast<UObject**>((uintptr_t)Gamestate + __int64(FindOffset("FortGameStateAthena", "CurrentPlaylistInfo")) + __int64(FindOffset("PlaylistPropertyArray", "BasePlaylist")));
		*BasePlaylist = Playlist;

		auto fn = FindObject(crypt("Function /Script/FortniteGame.FortGameStateAthena.OnRep_CurrentPlaylistInfo"));
		ProcessEvent(Gamestate, fn, nullptr);
	}
};

class CheatManagerFunctions
{
public:
	void Summon(FString ClassToSummon)
	{
		auto fn = FindObject(crypt("Function /Script/Engine.CheatManager.Summon"));
		ProcessEvent((UObject*)ReadPointer((uintptr_t)Controller, __int64(FindOffset("PlayerController", "CheatManager"))), fn, &ClassToSummon);
	}

	void SetGodMode()
	{
		auto fn = FindObject("Function /Script/Engine.CheatManager.God");
		ProcessEvent(*reinterpret_cast<UObject**>(__int64(Controller) + __int64(FindOffset("PlayerController", "CheatManager"))), fn, nullptr);
	}
};

class InventoryFunctions
{
public:
	void UpdateInventory()
	{
		static auto HandleWorldInventoryLocalUpdate = FindObject(crypt("Function /Script/FortniteGame.FortPlayerController.HandleWorldInventoryLocalUpdate"));
		static auto HandleInventoryLocalUpdate = FindObject(crypt("Function /Script/FortniteGame.FortInventory.HandleInventoryLocalUpdate"));
		static auto ClientForceUpdateQuickbar = FindObject(crypt("Function /Script/FortniteGame.FortPlayerController.ClientForceUpdateQuickbar"));

		ProcessEvent(FortInventory, HandleInventoryLocalUpdate, nullptr);
		ProcessEvent(Controller, HandleWorldInventoryLocalUpdate, nullptr);

		auto PrimaryQuickbar = EFortQuickBars::Primary;
		auto SecondaryQuickbar = EFortQuickBars::Secondary;
		ProcessEvent(Controller, ClientForceUpdateQuickbar, &PrimaryQuickbar);
		ProcessEvent(Controller, ClientForceUpdateQuickbar, &SecondaryQuickbar);
	}

	UObject* CreateTemporaryItemInstanceBP(UObject* ItemDefinition, int Count, int Level)
	{
		static UObject* CreateTemporaryItemInstanceBP = FindObject(crypt("Function /Script/FortniteGame.FortItemDefinition.CreateTemporaryItemInstanceBP"));

		struct
		{
			int Count;
			int Level;
			UObject* ReturnValue;
		} Params;

		Params.Count = Count;
		Params.Level = Level;

		ProcessEvent(ItemDefinition, CreateTemporaryItemInstanceBP, &Params);

		return Params.ReturnValue;
	}

	void SetOwningControllerForTemporaryItem(UObject* Item, UObject* Controller)
	{
		static UObject* SetOwningControllerForTemporaryItem = FindObject(crypt("Function /Script/FortniteGame.FortItem.SetOwningControllerForTemporaryItem"));
		ProcessEvent(Item, SetOwningControllerForTemporaryItem, &Controller);
	}

	UObject* CreateItem(UObject* ItemDefinition, int Count)
	{
		UObject* TemporaryItemInstance = CreateTemporaryItemInstanceBP(ItemDefinition, Count, 1);

		if (TemporaryItemInstance)
		{
			SetOwningControllerForTemporaryItem(TemporaryItemInstance, Controller);
		}

		int* CurrentCount = reinterpret_cast<int*>(__int64(TemporaryItemInstance) + static_cast<__int64>(FindOffset("FortWorldItem", "ItemEntry")) + static_cast<__int64>(FindOffset("FortItemEntry", "Count")));
		*CurrentCount = Count;

		return TemporaryItemInstance;
	}

	void AddItemToInventory(UObject* ItemDef, int Count, bool bAddToQuickBars = false, EFortQuickBars QuickBarType = EFortQuickBars::Max_None, int32_t slot = 0)
	{
		if (ItemDef)
		{
			UObject* ItemInstance = CreateItem(ItemDef, Count);

			if (ItemInstance)
			{
				auto ItemEntry = reinterpret_cast<FFortItemEntry*>(reinterpret_cast<uintptr_t>(ItemInstance) + __int64(FindOffset("FortWorldItem", "ItemEntry")));
				//reinterpret_cast<TArray<UObject*>*>(__int64(FortInventory) + static_cast<__int64>(FindOffset("FortInventory", "Inventory")) + static_cast<__int64>(FindOffset("FortItemList", "ItemInstances")))->Add(ItemInstance);
				reinterpret_cast<TArray<FFortItemEntry>*>(__int64(FortInventory) + static_cast<__int64>(__int64(FindOffset("FortInventory", "Inventory"))) + static_cast<__int64>(FindOffset("FortItemList", "ReplicatedEntries")))->Add(*ItemEntry);

				//return reinterpret_cast<FGuid*>((uintptr_t)ItemEntry + 0x68);
			}

			if (bAddToQuickBars) {
				//AddItemToQuickBars(ItemDef, QuickBarType, slot);
			}

			UpdateInventory();
		}
	}

	void AddItemToInventoryWithEntry(FFortItemEntry Entry, int Count)
	{
		reinterpret_cast<TArray<FFortItemEntry>*>(__int64(FortInventory) + static_cast<__int64>(__int64(FindOffset("FortInventory", "Inventory"))) + static_cast<__int64>(FindOffset("FortItemList", "ReplicatedEntries")))->Add(Entry);

		UpdateInventory();
	}
};

class PawnFunctions
{
public:
	void TeleportToSkydive(float InHeight)
	{
		static auto fn = FindObject(crypt("Function /Script/FortniteGame.FortPlayerPawnAthena.TeleportToSkyDive"));
		ProcessEvent(Pawn, fn, &InHeight);
	}
};

class WorldFunctions
{
public:
	static inline UObject* (*SpawnActorLong)(UObject* UWorld, UObject* Class, FTransform const* UserTransformPtr,
		const FActorSpawnParameters& SpawnParameters);

	UObject* SpawnActorFromLong(UObject* Class, FTransform trans)
	{
		SpawnActorLong = decltype(SpawnActorLong)(Util::FindPattern(crypt("48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 A8 0F 29 78 98 44 0F 29 40 ? 44 0F 29 88 ? ? ? ? 44 0F 29 90 ? ? ? ? 44 0F 29 98 ? ? ? ? 44 0F 29 A0 ? ? ? ? 44 0F 29 A8 ? ? ? ? 44 0F 29 B0 ? ? ? ? 44 0F 29 B8 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 45 33 ED 48 89 4C 24 ? 44 89 6C 24 ? 48 8D 05 ? ? ? ? 44 38 2D ? ? ? ? 4C 8B FA 48 8B D9 48 8D 15 ? ? ? ? 49 0F 45 C5 48 8D 4D 18 48 89 45 10 4D 8B F1 49 8B F0 E8 ? ? ? ? 4C 8B 63 30 4C 89 65 C0 4D 85 FF 0F 84 ? ? ? ? 41 8B 87 ? ? ? ? 0F BA E0 19 0F 82 ? ? ? ? A8 01 0F 85 ? ? ? ? E8 ? ? ? ? 48 8B D0 49 8B CF E8 ? ? ? ? 84 C0")));
		auto pWorld = reinterpret_cast<UObject**>(Util::FindPattern(crypt("48 8B 05 ? ? ? ? 4D 8B C1"), true, 3));

		auto parms = FActorSpawnParameters();
		return SpawnActorLong((*pWorld), Class, &trans, parms);
	}

	UObject* SpawnActor(UObject* ClassToSpawn, FVector loc, FRotator rot)
	{
		FQuat SpawnQuat;
		SpawnQuat.W = 0;
		SpawnQuat.X = rot.Pitch;
		SpawnQuat.Y = rot.Roll;
		SpawnQuat.Z = rot.Yaw;

		FTransform SpawnTrans;
		SpawnTrans.Scale3D = FVector(1, 1, 1);
		SpawnTrans.Translation = loc;
		SpawnTrans.Rotation = SpawnQuat;

		return SpawnActorFromLong(ClassToSpawn, SpawnTrans);
	}
};

class PickupFunctions
{
public:
	void SpawnPickup(UObject* ItemDef, int Count, EFortPickupSourceTypeFlag InPickupSourceTypeFlags, EFortPickupSpawnSource InPickupSpawnSource)
	{
		auto FortPickup = worldFunctions->SpawnActor(FindObject(crypt("Class /Script/FortniteGame.FortPickupAthena")), actorFunctions->GetActorLocation(Pawn), FRotator());
		//cheatManagerFunctions->Summon(L"FortPickupAthena_C");
		//auto FortPickup = FindObjectWithSkip(FindObject("Class /Script/FortniteGame.FortPickupAthena"));

		auto entry = reinterpret_cast<FFortItemEntry*>((uintptr_t)FortPickup + __int64(FindOffset("FortPickup", "PrimaryPickupItemEntry")));
		*reinterpret_cast<UObject**>((uintptr_t)entry + __int64(FindOffset("FortItemEntry", "ItemDefinition"))) = ItemDef;
		*reinterpret_cast<int*>((uintptr_t)entry + __int64(FindOffset("FortItemEntry", "Count"))) = Count;

		auto Fn = FindObject(crypt("Function /Script/FortniteGame.FortPickup.TossPickup"));
		auto Fn2 = FindObject(crypt("Function /Script/FortniteGame.FortPickup.OnRep_PrimaryPickupItemEntry"));

		struct
		{
			FVector FinalLocation;
			UObject* Pawn;
			int32_t OverrideMaxStackCount;
			bool bToss;
			bool bShouldCombinePickupsWhenTossCompletes;
			EFortPickupSourceTypeFlag InPickupSourceTypeFlags;
			EFortPickupSpawnSource InPickupSpawnSource;
		}params;

		params.FinalLocation = actorFunctions->GetActorLocation(Pawn);
		params.bToss = true;
		params.bShouldCombinePickupsWhenTossCompletes = true;
		params.Pawn = Pawn;
		params.OverrideMaxStackCount = 999;
		params.InPickupSourceTypeFlags = InPickupSourceTypeFlags;
		params.InPickupSpawnSource = InPickupSpawnSource;

		ProcessEvent(FortPickup, Fn2, nullptr);
		ProcessEvent(FortPickup, Fn, &params);
	}
};

class BuildingActorFunctions
{
public:
	void InitializeBuildingActor(UObject* BuildingActor)
	{
		auto InitializeKismetSpawnedBuildingActor = FindObject(crypt("Function /Script/FortniteGame.BuildingActor.InitializeKismetSpawnedBuildingActor"));

		struct Params
		{
			UObject* BuildingOwner;
			UObject* SpawningController;
		};

		Params params;
		params.BuildingOwner = BuildingActor;
		params.SpawningController = Controller;

		ProcessEvent(BuildingActor, InitializeKismetSpawnedBuildingActor, &params);
	}
};

namespace Functions
{
	static UObject* UpdatePlayerController()
	{
		auto FortEngine = FindObject(crypt("FortEngine /Engine/Transient.FortEngine"));
		auto GameInstance = *reinterpret_cast<UObject**>((uintptr_t)FortEngine + __int64(FindOffset("GameEngine", "GameInstance")));
		auto LocalPlayers = *reinterpret_cast<TArray<UObject*>*>((uintptr_t)GameInstance + __int64(FindOffset("GameInstance", "LocalPlayers")));
		auto LocalPlayer = LocalPlayers[0];
		auto PlayerController = *reinterpret_cast<UObject**>((uintptr_t)LocalPlayer + __int64(FindOffset("Player", "PlayerController")));
		Controller = PlayerController;
		return PlayerController;
	}

	static void UnlockConsole()
	{
		auto FortGameViewportClient = FindObject(crypt("FortGameViewportClient /Engine/Transient.FortEngine_"));
		auto fn = FindObject(crypt("Function /Script/Engine.GameplayStatics.SpawnObject"));
		auto statics = FindObject(crypt("GameplayStatics /Script/Engine.Default__GameplayStatics"));
		auto ConsoleClass = FindObject(crypt("/Script/Engine.Console"));
		auto ViewportConsole = reinterpret_cast<UObject**>((uintptr_t)FortGameViewportClient + 0x40);

		//	std::cout << "FortGameViewportClient: " << FortGameViewportClient << std::endl;
		//	std::cout << "SpawnObject: " << fn << std::endl;
	//std::cout << "Gameplay Statics: " << statics << std::endl;
		//	std::cout << "Console Class: " << ConsoleClass << std::endl;
		//	std::cout << "ViewportConsole: " << ViewportConsole << std::endl;

		SpawnObjectParams params;
		params.ObjectClass = ConsoleClass;
		params.Outer = FortGameViewportClient;

		ProcessEvent(statics, fn, &params);

		//std::cout << "Return Value: " << params.ReturnValue;
		*ViewportConsole = params.ReturnValue;

	}

	static void EnableCheatManager()
	{
		auto fn = FindObject(crypt("Function /Script/Engine.GameplayStatics.SpawnObject"));
		auto statics = FindObject(crypt("GameplayStatics /Script/Engine.Default__GameplayStatics"));
		auto CheatManager = reinterpret_cast<UObject**>(__int64(Controller) + __int64(FindOffset("PlayerController", "CheatManager")));
		auto CheatManagerClass = FindObject(crypt("/Script/Engine.CheatManager"));

		SpawnObjectParams params;
		params.ObjectClass = CheatManagerClass;
		params.Outer = Controller;

		ProcessEvent(statics, fn, &params);

		*CheatManager = params.ReturnValue;
	}

	ERHIType GetRenderingApi()
	{
		auto FortUserSettings = FindObject("FortGameUserSettings /Engine/Transient.FortGameUserSettings_");
		auto fn = FindObject("Function /Script/FortniteGame.FortGameUserSettings.GetRenderingAPI");

		ERHIType returnVal;

		ProcessEvent(FortUserSettings, fn, &returnVal);

		return returnVal;
	}

	static UObject* GetPickaxeDef()
	{
		auto CosmeticLoadoutPC = reinterpret_cast<FFortAthenaLoadout*>((uintptr_t)Controller + __int64(FindOffset("FortPlayerController", "CosmeticLoadoutPC")));
		auto Pickaxe = CosmeticLoadoutPC->Pickaxe;
		auto PickaxeDef = *reinterpret_cast<UObject**>((uintptr_t)Pickaxe + __int64(FindOffset("AthenaPickaxeItemDefinition", "WeaponDefinition")));
		return PickaxeDef;
	}

	inline void DestroyAll(UObject* Class)
	{
		auto dWorld = Util::FindPattern(crypt("48 8B 05 ? ? ? ? 4D 8B C1"), true, 3);
		CHECKSIG(dWorld, "Failed to find UWorld address!");
		auto Worldd = *reinterpret_cast<UObject**>(dWorld);

		auto GameplayStatics = FindObject(crypt("GameplayStatics /Script/Engine.Default__GameplayStatics"));
		auto GetAllActorsOfClass = FindObject(crypt("Function /Script/Engine.GameplayStatics.GetAllActorsOfClass"));

		GetAllActorsOfClass_Params params;
		params.ActorClass = Class;
		params.WorldContextObject = Worldd;

		ProcessEvent(GameplayStatics, GetAllActorsOfClass, &params);

		auto Actors = params.OutActors;

		const auto K2_DestroyActor = FindObject(crypt("Function /Script/Engine.Actor.K2_DestroyActor"));

		for (auto i = 0; i < Actors.Num(); i++)
		{
			actorFunctions->DestroyActor(Actors[i]);
		}
	}

	static void InitMatch()
	{
		World = reinterpret_cast<UObject**>(Util::FindPattern("48 8B 05 ? ? ? ? 4D 8B C1", true, 3));

		Functions::UpdatePlayerController();
		Functions::EnableCheatManager();

		cheatManagerFunctions->Summon(TEXT("PlayerPawn_Athena_C"));
		auto OutActors = kismetLibraryFunctions->GetAllActorsOfClass(FindObject("BlueprintGeneratedClass /Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C"));
		Pawn = OutActors[0];

		if (Pawn) {
			gamestateFunctions->SetPlaylist(FindObject(crypt("/Game/Athena/Playlists/BattleLab/Playlist_BattleLab.Playlist_BattleLab")));
			playerControllerFunctions->Possess(Pawn);
			gamemodeFunctions->StartMatch();
			playerControllerFunctions->ServerReadyToStartMatch();
			kismetLibraryFunctions->ShowSkin();
		}

		Functions::UpdatePlayerController();
		Functions::EnableCheatManager();

		cheatManagerFunctions->Summon(TEXT("PlayerPawn_Athena_C"));
		auto OutActors2 = kismetLibraryFunctions->GetAllActorsOfClass(FindObject("BlueprintGeneratedClass /Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C"));
		Pawn = OutActors2[0];

		if (Pawn) {
			playerControllerFunctions->Possess(Pawn);
			cheatManagerFunctions->SetGodMode();
			kismetLibraryFunctions->ShowSkin();
		}
	}
}