#pragma once

#include "CoreMinimal.h"
#include "InterestsUtil.generated.h"

// Forward declarations
class UInterest;
class AInterestLines;

UENUM(BlueprintType)
enum class EInterestType : uint8
{
	POI UMETA(DisplayName = "POI"),
	AOI UMETA(DisplayName = "AOI"),
	GeoFence UMETA(DisplayName = "GeoFence"),
	Takeoff UMETA(DisplayName = "TakeoffPoint"),
	Unknown UMETA(DisplayName = "Unknown")
};

/** Stores an interest group's interests */
USTRUCT(BlueprintType)
struct FSPInterestStruct
{
	GENERATED_BODY()
public:
	FSPInterestStruct();
	FSPInterestStruct(int32 InID);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 ID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<class USPTakeoffPoint> TakeoffPoint;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<int32, TObjectPtr<UInterest>> Interests;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<int32, TObjectPtr<class USPGeoFence>> GeoFences;
};

USTRUCT(BlueprintType)
struct FSPMissionProgress {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<int32, float> Progresses;
};

/** Stores mission attributes and interest order */
USTRUCT(BlueprintType)
struct FSPMissionStruct {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 ID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<int32> InterestOrder;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TArray<int32> AssignedSwarmIDs;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TObjectPtr<AInterestLines> MissionLine;
};

/** Stores an intermediate representation of a POI */
USTRUCT(BlueprintType)
struct FPOIDataStruct
{
	GENERATED_BODY()
public:
	FPOIDataStruct();
	FPOIDataStruct(int32 inID, FString inName, int32 inGroupID, FVector inLongLatHeight, FString Params);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 groupID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector longLatHeight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Params;
};

USTRUCT(BlueprintType)
struct FTakeoffPointDataStruct {
	GENERATED_BODY()
public:
	FTakeoffPointDataStruct();
	FTakeoffPointDataStruct(int32 InID, FString InName, int32 InGroupID, FVector InLonLatHeight, FString Params);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 ID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 GroupID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector LonLatHeight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Params;
};

/** Stores an intermediate representation of an AOI */
USTRUCT(BlueprintType)
struct FAOIDataStruct
{
	GENERATED_BODY()
public:
	FAOIDataStruct();
	FAOIDataStruct(int32 inID, FString inName, int32 inGroupID, TArray<FVector> inLongLatHeights, FString Params);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 groupID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FVector> longLatHeights;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Params;
};

/** Stores an intermediate representation of a GeoFence */
USTRUCT(BlueprintType)
struct FGeoFenceDataStruct
{
	GENERATED_BODY()

public:
	FGeoFenceDataStruct();
	FGeoFenceDataStruct(int32 InID, FString InName, int32 InGroupID, TArray<FVector> InLonLatHeights, FString Params);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 ID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 GroupID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FVector> LonLatHeights;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Params;
};


/**
 * Interest manager utility functions
 */
class SKYPLEXC2_API InterestsUtil : UObject
{
public:
	InterestsUtil();
	~InterestsUtil();

	static int32 ReverseMapInterestGroup(TMap<int32, FString> Map, FString GroupName);
};
