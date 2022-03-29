#pragma once

#include <cstdint>
#include <Windows.h>
#include <string>
#include <locale>
#include <iostream>
#include <fstream>
#include <Psapi.h>
#include <winscard.h>
#include "skCryptor.h"
#include <vector>
#include "Util.h"

struct UObject;

static UObject* StaticLoadObject(UObject* Class, UObject* InOuter, const TCHAR* Name, const TCHAR* FileName = nullptr, uint32_t LoadFlags = 0, void* Sandbox = nullptr, bool bAllowObjectReconciliation = false, void* InstancingContext = nullptr)
{
	auto staticloadobjectaddr = Util::FindPattern(crypt("48 8B C4 48 89 58 08 4C 89 48 20 4C 89 40 18 48 89 50 10 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 70 33 FF 48 8D 05 ? ? ? ? 40 38 3D ? ? ? ? 4C 8B F9 49 8B D0 48 8D 4D E0 48 0F 45 C7 49 8B D8 48 89 45 D0 E8 ? ? ? ? 48 8D 15 ? ? ? ? 48 8B CB 8B F7"));
	auto fStaticLoadObject = reinterpret_cast<UObject * (__fastcall*)(UObject*, UObject*, const TCHAR*, const TCHAR*, uint32_t, void*, bool, void*)>(staticloadobjectaddr);
	return fStaticLoadObject(Class, InOuter, Name, FileName, LoadFlags, Sandbox, bAllowObjectReconciliation, InstancingContext);
}

template<class T>
struct TArray
{
	friend class FString;

public:
	inline TArray()
	{
		Data = NULL;
		Count = Max = 0;
	};

	inline INT Num() const
	{
		return Count;
	};

	inline T& operator[](int i)
	{
		return Data[i];
	};

	inline T& operator[](int i) const
	{
		return Data[i];
	};

	inline BOOL IsValidIndex(int i) const
	{
		return i < Num();
	}

	inline void Add(T InputData)
	{
		Data = (T*)realloc(Data, sizeof(T) * (Count + 1));
		Data[Count++] = InputData;
		Max = Count;
	};

	inline void Remove(int32_t Index)
	{
		TArray<T> NewArray;
		for (int i = 0; i < this->Count; i++)
		{
			if (i == Index)
				continue;

			NewArray.Add(this->operator[](i));
		}
		this->Data = (T*)realloc(NewArray.Data, sizeof(T) * (NewArray.Count));
		this->Count = NewArray.Count;
		this->Max = NewArray.Count;
	}

	T* Data;
	INT32 Count;
	INT32 Max;

	inline void Remove(T InData)
	{
		TArray<T> NewArray;
		for (int i = 0; i < this->Count; i++)
		{
			if (this->operator[](i) != InData)
			{
				NewArray.Add(this->operator[](i));
			}
		}
		this = NewArray;
	}
};


struct FString : private TArray<wchar_t>
{
	FString()
	{
	};

	FString(const wchar_t* other)
	{
		Max = Count = *other ? std::wcslen(other) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	}

	bool IsValid() const
	{
		return Data != nullptr;
	}

	const wchar_t* c_str() const
	{
		return Data;
	}

	std::string ToString() const
	{
		auto length = std::wcslen(Data);

		std::string str(length, '\0');

		std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

		return str;
	}
};

template <class TEnum>
class TEnumAsByte
{
public:
	TEnumAsByte()
	{
	}

	TEnumAsByte(TEnum _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit TEnumAsByte(int32_t _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit TEnumAsByte(uint8_t _value)
		: value(_value)
	{
	}

	operator TEnum() const
	{
		return static_cast<TEnum>(value);
	}

	TEnum GetValue() const
	{
		return static_cast<TEnum>(value);
	}

private:
	uint8_t value;
};

struct FName;

void (*FNameToString)(FName* pThis, FString& out);
void (*FreeMemory)(__int64);

struct FName
{
	uint32_t ComparisonIndex;
	uint32_t DisplayIndex;

	FName() = default;

	explicit FName(int64_t name)
	{
		DisplayIndex = (name & 0xFFFFFFFF00000000LL) >> 32;
		ComparisonIndex = (name & 0xFFFFFFFFLL);
	};

	std::string ToString()
	{
		FString temp;
		FNameToString(this, temp);

		std::string ret(temp.ToString());
		FreeMemory((__int64)temp.c_str());

		return ret;
	}
};

struct FActorSpawnParameters
{
	unsigned char Unk00[0x40];
};

struct UObject
{
	void** VFT;
	int32_t ObjectFlags;
	int32_t InternalIndex;
	UObject* Class;
	FName Name;
	UObject* Outer;

	bool IsA(UObject* cmp) const
	{
		if (cmp == Class)
			return false;
		return false;
	}

	std::string GetName()
	{
		return Name.ToString();
	}

	std::string GetFullName()
	{
		std::string temp;

		for (auto outer = Outer; outer; outer = outer->Outer)
		{
			temp = outer->GetName() + "." + temp;
		}

		temp = reinterpret_cast<UObject*>(Class)->GetName() + " " + temp + this->GetName();
		return temp;
	}
};

struct FGuid
{
	int A;
	int B;
	int C;
	int D;
};

struct FUObjectItem
{
	UObject* Object;
	DWORD Flags;
	DWORD ClusterIndex;
	DWORD SerialNumber;
	DWORD SerialNumber2;
};

struct PreFUObjectItem
{
	FUObjectItem* FUObject[10];
};

struct FUObjectArray
{
	PreFUObjectItem* ObjectArray;
	BYTE unknown1[8];
	int32_t MaxElements;
	int32_t NumElements;

	void NumChunks(int* start, int* end) const
	{
		int cStart = 0, cEnd = 0;

		if (!cEnd)
		{
			while (true)
			{
				if (ObjectArray->FUObject[cStart] == nullptr)
				{
					cStart++;
				}
				else
				{
					break;
				}
			}

			cEnd = cStart;
			while (true)
			{
				if (ObjectArray->FUObject[cEnd] == nullptr)
				{
					break;
				}
				cEnd++;
			}
		}

		*start = cStart;
		*end = cEnd;
	}

	UObject* GetByIndex(int32_t index) const
	{
		int cStart = 0, cEnd = 0;
		int chunkIndex, chunkSize = 0xFFFF, chunkPos;
		FUObjectItem* Object;

		NumChunks(&cStart, &cEnd);

		chunkIndex = index / chunkSize;
		if (chunkSize * chunkIndex != 0 &&
			chunkSize * chunkIndex == index)
		{
			chunkIndex--;
		}

		chunkPos = cStart + chunkIndex;
		if (chunkPos < cEnd)
		{
			Object = ObjectArray->FUObject[chunkPos] + (index - chunkSize * chunkIndex);
			if (!Object) { return nullptr; }

			return Object->Object;
		}

		return nullptr;
	}
};

FUObjectArray* GObjects;

struct FVector
{
	double X;
	double Y;
	double Z;

	FVector()
		: X(0), Y(0), Z(0)
	{
	}

	FVector(float x, float y, float z)
		: X(x),
		Y(y),
		Z(z)
	{
	}

	FVector operator-(FVector v)
	{
		return FVector(X - v.X, Y - v.Y, Z - v.Z);
	}

	FVector operator+(FVector v)
	{
		return FVector(X + v.X, Y + v.Y, Z + v.Z);
	}

	float Distance(FVector v)
	{
		return ((X - v.X) * (X - v.X) +
			(Y - v.Y) * (Y - v.Y) +
			(Z - v.Z) * (Z - v.Z));
	}
};

struct FRotator
{
	float Pitch;
	float Yaw;
	float Roll;

	FRotator()
		: Pitch(0),
		Yaw(0),
		Roll(0)
	{
	}

	FRotator(float pitch, float yaw, float roll)
		: Pitch(pitch),
		Yaw(yaw),
		Roll(roll)
	{
	}
};

struct FQuat
{
	float W, X, Y, Z;

	FQuat()
		: W(0),
		X(0),
		Y(0),
		Z(0)
	{
	}

	FQuat(float w, float x, float y, float z)
		: W(w),
		X(x),
		Y(y),
		Z(z)
	{
	}
};

struct FTransform
{
	FQuat Rotation;
	FVector Translation;
	char UnknownData_1C[0x4];
	FVector Scale3D;
	char UnknownData_2C[0x4];
};

enum class EAthenaGamePhase
{
	None = 0,
	Setup = 1,
	Warmup = 2,
	Aircraft = 3,
	SafeZones = 4,
	EndGame = 5,
	Count = 6,
	EAthenaGamePhase_MAX = 7
};

struct GetAllActorsOfClass_Params
{
	UObject* WorldContextObject;
	UObject* ActorClass; //AActor
	TArray<UObject*> OutActors; //AActor
};

struct RBitField
{
	unsigned char A : 1;
	unsigned char B : 1;
	unsigned char C : 1;
	unsigned char D : 1;
	unsigned char E : 1;
	unsigned char F : 1;
	unsigned char G : 1;
	unsigned char H : 1;
};

struct FPlaylistPropertyArray
{
	unsigned char padding_180[0x68];//0x180 (0x68)
	UObject* BasePlaylist; //0x120 (0x8)
	UObject* OverridePlaylist; //0x128 (0x8)
};

enum class ESpawnActorCollisionHandlingMethod : uint8_t
{
	Undefined = 0,
	AlwaysSpawn = 1,
	AdjustIfPossibleButAlwaysSpawn = 2,
	AdjustIfPossibleButDontSpawnIfColliding = 3,
	DontSpawnIfColliding = 4,
	ESpawnActorCollisionHandlingMethod_MAX = 5
};

struct UGameplayStatics_BeginDeferredActorSpawnFromClass_Params
{
	UObject* WorldContextObject;
	UObject* ActorClass;
	FTransform SpawnTransform;
	ESpawnActorCollisionHandlingMethod CollisionHandlingOverride;
	UObject* Owner;
	UObject* ReturnValue;
};

struct UGameplayStatics_FinishSpawningActor_Params
{
	UObject* Actor;
	FTransform SpawnTransform;
	UObject* ReturnValue;
};

struct SpawnObjectParams
{
	UObject* ObjectClass;
	UObject* Outer;
	UObject* ReturnValue;
};

struct FFortGiftingInfo {};
struct FGameplayAbilitySpecHandle {};
struct FFastArraySerializer {};

struct FGameplayAbilitySpecDef
{
	UObject* Ability;
	unsigned char Unk00[0x90];
};

struct FActiveGameplayEffectHandle
{
	int Handle; // 0x00(0x04)
	bool bPassedFiltersAndWasExecuted; // 0x04(0x01)
	char UnknownData_5[0x3]; // 0x05(0x03)
};

struct FGameplayEffectContextHandle
{
	char UnknownData_0[0x30]; // 0x00(0x18)
};

//Inventory Structs Below
struct QuickbarSlot
{
	TArray<struct FGuid> Items;
	bool bEnabled;
	char Unk00[0x7];
};

struct InventoryPointer
{
	UObject* Inventory;
};

struct QuickBarPointer
{
	UObject* QuickBar;
};

enum EObjectFlags
{
	RF_NoFlags = 0x00000000,
	RF_Public = 0x00000001,
	RF_Standalone = 0x00000002,
	RF_MarkAsNative = 0x00000004,
	RF_Transactional = 0x00000008,
	RF_ClassDefaultObject = 0x00000010,
	RF_ArchetypeObject = 0x00000020,
	RF_Transient = 0x00000040,
	RF_MarkAsRootSet = 0x00000080,
	RF_TagGarbageTemp = 0x00000100,
	RF_NeedInitialization = 0x00000200,
	RF_NeedLoad = 0x00000400,
	RF_KeepForCooker = 0x00000800,
	RF_NeedPostLoad = 0x00001000,
	RF_NeedPostLoadSubobjects = 0x00002000,
	RF_NewerVersionExists = 0x00004000,
	RF_BeginDestroyed = 0x00008000,
	RF_FinishDestroyed = 0x00010000,
	RF_BeingRegenerated = 0x00020000,
	RF_DefaultSubObject = 0x00040000,
	RF_WasLoaded = 0x00080000,
	RF_TextExportTransient = 0x00100000,
	RF_LoadCompleted = 0x00200000,
	RF_InheritableComponentTemplate = 0x00400000,
	RF_DuplicateTransient = 0x00800000,
	RF_StrongRefOnFrame = 0x01000000,
	RF_NonPIEDuplicateTransient = 0x02000000,
	RF_Dynamic = 0x04000000,
	RF_WillBeLoaded = 0x08000000,
};

enum EInternalObjectFlags
{
	None = 0,
	ReachableInCluster = 1 << 23,
	ClusterRoot = 1 << 24,
	Native = 1 << 25,
	Async = 1 << 26,
	AsyncLoading = 1 << 27,
	Unreachable = 1 << 28,
	PendingKill = 1 << 29,
	RootSet = 1 << 30,
	GarbageCollectionKeepFlags = Native | Async | AsyncLoading,
	AllFlags = ReachableInCluster | ClusterRoot | Native | Async | AsyncLoading | Unreachable | PendingKill | RootSet,
};

enum EPropertyFlags : uint64_t
{
	CPF_None = 0,

	CPF_Edit = 0x0000000000000001,
	///< Property is user-settable in the editor.
	CPF_ConstParm = 0x0000000000000002,
	///< This is a constant function parameter
	CPF_BlueprintVisible = 0x0000000000000004,
	///< This property can be read by blueprint code
	CPF_ExportObject = 0x0000000000000008,
	///< Object can be exported with actor.
	CPF_BlueprintReadOnly = 0x0000000000000010,
	///< This property cannot be modified by blueprint code
	CPF_Net = 0x0000000000000020,
	///< Property is relevant to network replication.
	CPF_EditFixedSize = 0x0000000000000040,
	///< Indicates that elements of an array can be modified, but its size cannot be changed.
	CPF_Parm = 0x0000000000000080,
	///< Function/When call parameter.
	CPF_OutParm = 0x0000000000000100,
	///< Value is copied out after function call.
	CPF_ZeroConstructor = 0x0000000000000200,
	///< memset is fine for construction
	CPF_ReturnParm = 0x0000000000000400,
	///< Return value.
	CPF_DisableEditOnTemplate = 0x0000000000000800,
	///< Disable editing of this property on an archetype/sub-blueprint
	//CPF_      						= 0x0000000000001000,	///< 
	CPF_Transient = 0x0000000000002000,
	///< Property is transient: shouldn't be saved or loaded, except for Blueprint CDOs.
	CPF_Config = 0x0000000000004000,
	///< Property should be loaded/saved as permanent profile.
	//CPF_								= 0x0000000000008000,	///< 
	CPF_DisableEditOnInstance = 0x0000000000010000,
	///< Disable editing on an instance of this class
	CPF_EditConst = 0x0000000000020000,
	///< Property is uneditable in the editor.
	CPF_GlobalConfig = 0x0000000000040000,
	///< Load config from base class, not subclass.
	CPF_InstancedReference = 0x0000000000080000,
	///< Property is a component references.
	//CPF_								= 0x0000000000100000,	///<
	CPF_DuplicateTransient = 0x0000000000200000,
	///< Property should always be reset to the default value during any type of duplication (copy/paste, binary duplication, etc.)
	//CPF_								= 0x0000000000400000,	///< 
	//CPF_    							= 0x0000000000800000,	///< 
	CPF_SaveGame = 0x0000000001000000,
	///< Property should be serialized for save games, this is only checked for game-specific archives with ArIsSaveGame
	CPF_NoClear = 0x0000000002000000,
	///< Hide clear (and browse) button.
	//CPF_  							= 0x0000000004000000,	///<
	CPF_ReferenceParm = 0x0000000008000000,
	///< Value is passed by reference; CPF_OutParam and CPF_Param should also be set.
	CPF_BlueprintAssignable = 0x0000000010000000,
	///< MC Delegates only.  Property should be exposed for assigning in blueprint code
	CPF_Deprecated = 0x0000000020000000,
	///< Property is deprecated.  Read it from an archive, but don't save it.
	CPF_IsPlainOldData = 0x0000000040000000,
	///< If this is set, then the property can be memcopied instead of CopyCompleteValue / CopySingleValue
	CPF_RepSkip = 0x0000000080000000,
	///< Not replicated. For non replicated properties in replicated structs 
	CPF_RepNotify = 0x0000000100000000,
	///< Notify actors when a property is replicated
	CPF_Interp = 0x0000000200000000,
	///< interpolatable property for use with matinee
	CPF_NonTransactional = 0x0000000400000000,
	///< Property isn't transacted
	CPF_EditorOnly = 0x0000000800000000,
	///< Property should only be loaded in the editor
	CPF_NoDestructor = 0x0000001000000000,
	///< No destructor
	//CPF_								= 0x0000002000000000,	///<
	CPF_AutoWeak = 0x0000004000000000,
	///< Only used for weak pointers, means the export type is autoweak
	CPF_ContainsInstancedReference = 0x0000008000000000,
	///< Property contains component references.
	CPF_AssetRegistrySearchable = 0x0000010000000000,
	///< asset instances will add properties with this flag to the asset registry automatically
	CPF_SimpleDisplay = 0x0000020000000000,
	///< The property is visible by default in the editor details view
	CPF_AdvancedDisplay = 0x0000040000000000,
	///< The property is advanced and not visible by default in the editor details view
	CPF_Protected = 0x0000080000000000,
	///< property is protected from the perspective of script
	CPF_BlueprintCallable = 0x0000100000000000,
	///< MC Delegates only.  Property should be exposed for calling in blueprint code
	CPF_BlueprintAuthorityOnly = 0x0000200000000000,
	///< MC Delegates only.  This delegate accepts (only in blueprint) only events with BlueprintAuthorityOnly.
	CPF_TextExportTransient = 0x0000400000000000,
	///< Property shouldn't be exported to text format (e.g. copy/paste)
	CPF_NonPIEDuplicateTransient = 0x0000800000000000,
	///< Property should only be copied in PIE
	CPF_ExposeOnSpawn = 0x0001000000000000,
	///< Property is exposed on spawn
	CPF_PersistentInstance = 0x0002000000000000,
	///< A object referenced by the property is duplicated like a component. (Each actor should have an own instance.)
	CPF_UObjectWrapper = 0x0004000000000000,
	///< Property was parsed as a wrapper class like TSubclassOf<T>, FScriptInterface etc., rather than a USomething*
	CPF_HasGetValueTypeHash = 0x0008000000000000,
	///< This property can generate a meaningful hash value.
	CPF_NativeAccessSpecifierPublic = 0x0010000000000000,
	///< Public native access specifier
	CPF_NativeAccessSpecifierProtected = 0x0020000000000000,
	///< Protected native access specifier
	CPF_NativeAccessSpecifierPrivate = 0x0040000000000000,
	///< Private native access specifier
	CPF_SkipSerialization = 0x0080000000000000,
	///< Property shouldn't be serialized, can still be exported to text
};

enum ELifetimeCondition
{
	COND_None = 0,
	COND_InitialOnly = 1,
	COND_OwnerOnly = 2,
	COND_SkipOwner = 3,
	COND_SimulatedOnly = 4,
	COND_AutonomousOnly = 5,
	COND_SimulatedOrPhysics = 6,
	COND_InitialOrOwner = 7,
	COND_Custom = 8,
	COND_ReplayOrOwner = 9,
	COND_ReplayOnly = 10,
	COND_SimulatedOnlyNoReplay = 11,
	COND_SimulatedOrPhysicsNoReplay = 12,
	COND_SkipReplay = 13,
	COND_Never = 15,
	COND_Max = 16,
};

struct FField
{
	void* VFT;
	void* Class;
	void* Owner;
	void* padding;
	FField* Next;
	FName Name;
	EObjectFlags FlagsPrivate;

	std::string GetName()
	{
		return Name.ToString();
	}

	std::string GetTypeName()
	{
		return (*static_cast<FName*>(Class)).ToString();
	}

	std::string GetFullName()
	{
		std::string temp;

		for (auto outer = Next; outer; outer = outer->Next)
		{
			temp = outer->GetName() + "." + temp;
		}

		temp = static_cast<UObject*>(Class)->GetName() + " " + temp + this->GetName();
		return temp;
	}
};

struct FProperty : FField
{
	int32_t ArrayDim;
	int32_t ElementSize;
	EPropertyFlags PropertyFlags;
	uint16_t RepIndex;
	TEnumAsByte<ELifetimeCondition> BlueprintReplicationCondition;
	int32_t Offset_Internal;
	FName RepNotifyFunc;
	FProperty* PropertyLinkNext;
	FProperty* NextRef;
	FProperty* DestructorLinkNext;
	FProperty* PostConstructLinkNext;
};

struct UField : UObject
{
	UField* Next;
	void* padding_01;
	void* padding_02;
};

struct UStruct : UField
{
	UStruct* SuperStruct;
	UField* Children;
	FField* ChildProperties;
	int32_t PropertiesSize;
	int32_t MinAlignment;
	TArray<uint8_t> Script;
	FProperty* PropertyLink;
	FProperty* RefLink;
	FProperty* DestructorLink;
	FProperty* PostConstructLink;
};

struct UClass : UStruct
{
};

FString* (*GetEngineVersion)(void*);

static void DumpObjects()
{
	std::ofstream log(crypt("Dump.txt"));

	auto TotalObjects = GObjects->NumElements;

	for (int i = 0; i < TotalObjects; ++i)
	{
		auto CurrentObject = GObjects->GetByIndex(i);

		if (CurrentObject)
		{
			auto name = CurrentObject->GetFullName();
			std::string str(name.begin(), name.end());

			log << str + "\n";
		}
	}
	return;
}

static UObject* FindObject(std::string name)
{
	for (int32_t i = 0; i < GObjects->NumElements; i++)
	{
		auto object = GObjects->GetByIndex(i);

		if (object == nullptr)
			continue;

		if (object->GetFullName().find(name) != std::string::npos) {
			//std::cout << "Found Object: " << object->GetFullName() << std::endl;
			return object;
		}
	}

	return nullptr;
}

std::vector<UObject*> FoundObjects;

static UObject* FindObjectWithSkip(UObject* Class, int Skip = 1)
{
	for (int32_t i = 0; i < GObjects->NumElements; i++)
	{
		auto object = GObjects->GetByIndex(i);

		if (object == nullptr)
			continue;

		if (object->Class == Class) 
		{
			if (Skip > 0)
			{
				Skip--;
			}
			else
			{
				bool bFoundObject = false;
				for (auto Obj : FoundObjects)
				{
					if (Obj == object)
					{
						bFoundObject = true;
					}
				}
				if (!bFoundObject)
				{
					std::cout << "Name: " << object->GetFullName() << std::endl;
					FoundObjects.push_back(object);
					return object;
				}
			}
		}
	}

	return nullptr;
}

static UObject* FindObjectStart(std::string name)
{
	for (int32_t i = 0; i < GObjects->NumElements; i++)
	{
		auto object = GObjects->GetByIndex(i);
		
		if (object == nullptr)
			continue;

		if (object->GetFullName().starts_with(name))
			return object;
	}

	return nullptr;
}

static UObject* FindObjectByShortName(std::string name)
{
	for (int32_t i = 0; i < GObjects->NumElements; i++)
	{
		auto object = GObjects->GetByIndex(i);

		if (object == nullptr)
			continue;

		if (object->GetName() == name)
			return object;
	}

	return nullptr;
}

static DWORD FindOffset(std::string ClassName, std::string VarName)
{
	auto ObjectClass = (UClass*)FindObjectByShortName(ClassName);

	if (ObjectClass != nullptr)
	{
		//std::cout << "[Offset Finder]: Found Class: " << ObjectClass->GetFullName() << std::endl;

		FField* next = ObjectClass->ChildProperties->Next;

		if (next == nullptr)
			return 0;

		auto fPropertyName = ObjectClass->ChildProperties->GetName();

		if (fPropertyName == VarName)
		{
			//std::cout << "[Offset Finder]: Found Offset: " << *(DWORD*)(__int64(ObjectClass->ChildProperties) + 0x4C) << " At: " << VarName << std::endl;
			return *(DWORD*)(__int64(ObjectClass->ChildProperties) + 0x44);
		}

		while (next)
		{
			auto nextName = next->GetName();

			if (nextName == VarName) {
				//std::cout << "[Offset Finder]: Found Offset: " << *(DWORD*)(__int64(next) + 0x4C) << " At: " << VarName << std::endl;
				return *(DWORD*)(__int64(next) + 0x44);
			} else {
				next = next->Next;
			}
		}
	}

	return 0;
}

inline bool ProcessEvent(UObject* pObject, UObject* pFunction, void* pParams) {
	auto vtable = *reinterpret_cast<void***>(pObject);
	auto ProcesseventVtable = static_cast<void(*)(void*, void*, void*)>(vtable[0x4B]); if (!ProcesseventVtable) return false;
	ProcesseventVtable(pObject, pFunction, pParams);
	return true;
}

enum class EGameplayEffectDurationType : uint8_t
{
	Instant,
	Infinite,
	HasDuration,
	EGameplayEffectDurationType_MAX
};

struct FFortItemEntry
{
	unsigned char Unk00[0x1A0];
};

enum class EFortQuickBars : uint8_t
{
	Primary,
	Secondary,
	Max_None,
	EFortQuickBars_MAX
};

struct FFortAthenaLoadout
{
	unsigned char UnknownData00[0x10];
	FString BannerIconId;
	FString BannerColorId;
	UObject* SkyDiveContrail;
	UObject* Glider;
	UObject* Pickaxe;
	bool bIsDefaultCharacter;
	unsigned char UnknownData01[0x7];
	UObject* Character;
	TArray<UObject*> CharacterVariantChannels;
	bool bForceUpdateVariants;
	unsigned char UnknownData02[0x7];
	UObject* Hat;
	UObject* Backpack;
	UObject* LoadingScreen;
	UObject* BattleBus;
	UObject* VehicleDecoration;
	UObject* CallingCard;
	UObject* MapMarker;
	TArray<UObject*> Dances;
	UObject* VictoryPose;
	UObject* MusicPack;
	UObject* ItemWrapOverride;
	TArray<UObject*> ItemWraps;
	UObject* CharmOverride;
	TArray<UObject*> Charms;
	UObject* PetSkin;
	unsigned char UnknownData03[0x60];
};

enum class EFortCustomBodyType : uint8_t
{
	NONE = 0,
	Small = 1,
	Medium = 2,
	MediumAndSmall = 3,
	Large = 4,
	LargeAndSmall = 5,
	LargeAndMedium = 6,
	All = 7,
	Deprecated = 8,
	EFortCustomBodyType_MAX = 9
};

enum class EFortCustomGender : uint8_t
{
	Invalid = 0,
	Male = 1,
	Female = 2,
	Both = 3,
	EFortCustomGender_MAX = 4
};

enum class EMontagePlayReturnType : uint8_t
{
	MontageLength = 0,
	Duration = 1,
	EMontagePlayReturnType_MAX = 2
};

struct FSlateBrush
{
	unsigned char Unk00[0x48];
	UObject* ObjectResource; // 0x08
};

struct QuickBarSlot
{
	TArray<FGuid> Items;
	bool bEnabled;
	char Unk00[0x7];
};

struct FQuickBar {
	int32_t CurrentFocusedSlot; // 0x00(0x04)
	int32_t PreviousFocusedSlot; // 0x04(0x04)
	int32_t SecondaryFocusedSlot; // 0x08(0x04)
	char pad_C[0x4]; // 0x0c(0x04)
	struct TArray<QuickBarSlot> Slots; // 0x10(0x10)
	char pad[0x3];
};

enum class EFortPickupSourceTypeFlag : uint8_t
{
	Other = 0,
	Player = 1,
	Destruction = 2,
	Container = 4,
	AI = 8,
	Tossed = 16,
	FloorLoot = 32,
	Fishing = 64,
	EFortPickupSourceTypeFlag_MAX = 65
};

enum class EFortPickupSpawnSource : uint8_t
{
	Unset = 0,
	PlayerElimination = 1,
	Chest = 2,
	SupplyDrop = 3,
	AmmoBox = 4,
	Drone = 5,
	ItemSpawner = 6,
	BotElimination = 7,
	NPCElimination = 8,
	LootDrop = 9,
	TossedByPlayer = 10,
	EFortPickupSpawnSource_MAX = 11
};

enum class ERHIType : uint8_t
{
	D3D11 = 0,
	D3D12 = 1,
	Performance = 2,
	ERHIType_MAX = 3
};

enum class EFortEmotePlayMode : uint8_t
{
	CheckIfOwned = 0,
	ForcePlay = 1,
	EFortEmotePlayMode_MAX = 2
};