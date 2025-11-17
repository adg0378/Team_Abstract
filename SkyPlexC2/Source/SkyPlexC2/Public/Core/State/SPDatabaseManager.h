// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Core/State/SPManagerBase.h"
#include "SPDatabaseManager.generated.h"

// for database use to allow for backwards compatibility with interest changes
enum class DBInterestType : uint8 {
	POI = 0,
	AOI = 1,
	GeoFence = 2,
	TakeoffPoint = 3,
};

/* Stores mission attributes and interest order */
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
};

/* Stores an intermediate representation of a POI */
USTRUCT(BlueprintType)
struct FPOIDataStruct
{
	GENERATED_BODY()
public:
	FPOIDataStruct();
	FPOIDataStruct(int32 inID, FString inName, int32 inGroupID, FVector inLongLatHeight);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 groupID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector longLatHeight;
};

/* Stores an intermediate representation of a TakeoffPoint */
USTRUCT(BlueprintType)
struct FTakeoffPointDataStruct {
	GENERATED_BODY()
public:
	FTakeoffPointDataStruct();
	FTakeoffPointDataStruct(int32 InID, FString InName, int32 InGroupID, FVector InLonLatHeight);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 ID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 GroupID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector LonLatHeight;
};

/* Stores an intermediate representation of an AOI */
USTRUCT(BlueprintType)
struct FAOIDataStruct
{
	GENERATED_BODY()
public:
	FAOIDataStruct();
	FAOIDataStruct(int32 inID, FString inName, int32 inGroupID, TArray<FVector> inLongLatHeights);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 groupID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FVector> longLatHeights;
};

/* Stores an intermediate representation of a GeoFence */
USTRUCT(BlueprintType)
struct FGeoFenceDataStruct
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 ID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 GroupID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FVector> LonLatHeights;
};

/**
 * Manages the main database of SkyPlex-C2, storing drones, interests, etc
 */
UCLASS()
class SKYPLEXC2_API USPDatabaseManager : public USPManagerBase
{
	GENERATED_BODY()
	
public:
	USPDatabaseManager();
	~USPDatabaseManager();

	void Setup_Implementation() override;

	//
	// Interests
	//

	void InsertAOI(FString name, TArray<FVector> pointLocations, int32 groupID, int32& outID);
	void InsertPOI(FString name, FVector pointLocation, int32 groupID, int32& outID);
	void InsertGeoFence(FString Name, TArray<FVector> PointLocations, int32 GroupID, int32& OutID);
	void InsertTakeoffPoint(FString Name, FVector PointLocation, int32 MissionID, int32& OutID);
	void ReassignInterest(int32 InID, int32 InNewGroupID);
	void RenameInterest(int32 InID, FString InNewName);
	void MovePOI(int32 InID, FVector InNewPointLocation);
	void MoveTakeoffPoint(int32 InID, FVector InNewPointLocation);
	void MoveAOI(int32 InID, TArray<FVector> InNewPointLocations);
	void MoveGeoFence(int32 InID, TArray<FVector> InNewPointLocations);
	void DeleteInterest(int32 id);

	//
	// Missions
	//

	void InsertMission(FString name, int32& outID);
	void RenameMission(int32 InID, FString InNewName);
	void UpdateMissionOrder(int32 InID, TArray<int32> InterestOrder);
	void DeleteMission(int32 InID);

	//
	// Swarms
	//

	void InsertSwarm(FString Name, int32& OutID);
	void RenameSwarm(int32 InID, FString InNewName);
	void DeleteSwarm(int32 InID);

	//
	// Loading and setup
	//

	void LoadMissions(TMap<int32, FSPMissionStruct>& outMissions);
	void LoadInterests(
		TArray<FPOIDataStruct>& outPOIs,
		TArray<FAOIDataStruct>& outAOIs,
		TArray<FGeoFenceDataStruct>& OutGeoFences,
		TArray<FTakeoffPointDataStruct>& OutTakeoffPoints
	);
	void LoadSwarms(TMap<int32, FString>& OutSwarms);

private:
	struct sqlite3* dbConnection;

	UFUNCTION()
	void OnPreExitCleanup();

	void InitializeDatabase();

	void InsertInterest(FString name, DBInterestType type, FString pointCSV, int32 groupID, int32& outID);

	void MoveInterest(int32 InID, FString PointCSV);
};
