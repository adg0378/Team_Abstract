#pragma once

#include "CoreMinimal.h"
#include "State/SPManager.h"
#include "MissionAssetsManager.generated.h"

// Forward declarations
struct sqlite3;
class AAOI;
class AInterestPoint;
class UGeoJsonManager;
class UStorageManager;
struct FPOIDataStruct;
struct FAOIDataStruct;
struct FSPMissionStruct;
struct FGeoFenceDataStruct;
struct FTakeoffPointDataStruct;

enum class DBInterestType : uint8 {
	POI = 0,
	AOI = 1,
	GeoFence = 2,
	TakeoffPoint = 3,
};

/**
 * Manages the main database of SkyPlex-C2, storing drones, interests, telemetry, etc
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API UMissionAssetsManager : public USPManager
{
	GENERATED_BODY()

public:
	UMissionAssetsManager();
	~UMissionAssetsManager();

	void Setup_Implementation(bool& outSuccess) override;

	//
	// Interests
	//

	void InsertAOI(FString name, TArray<FVector> pointLocations, int32 groupID, FString& ParamsJson, int32& outID);
	void InsertPOI(FString name, FVector pointLocation, int32 groupID, FString& ParamsJson, int32& outID);
	void InsertGeoFence(FString Name, TArray<FVector> PointLocations, int32 GroupID, FString& ParamsJson, int32& OutID);
	void InsertTakeoffPoint(FString Name, FVector PointLocation, int32 MissionID, FString& ParamsJson, int32& OutID);
	void ReassignInterest(int32 InID, int32 InNewGroupID);
	void RenameInterest(int32 InID, FString InNewName);
	void MovePOI(int32 InID, FVector InNewPointLocation);
	void MoveTakeoffPoint(int32 InID, FVector InNewPointLocation);
	void MoveAOI(int32 InID, TArray<FVector> InNewPointLocations);
	void MoveGeoFence(int32 InID, TArray<FVector> InNewPointLocations);
	void UpdateInterestParams(int32 InID, FString& ParamsJson);
	void DeleteInterest(int32 id);

	//
	// Missions
	//

	void InsertMission(FString name, int32& outID);
	void RenameMission(int32 InID, FString InNewName);
	void UpdateMissionOrder(int32 InID, TArray<int32>& InterestOrder);
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

	/*void LoadInterestsViaFile(FString filePath, 
		TArray<FPOIDataStruct>& outPOIs,
		TArray<FAOIDataStruct>& outAOIs,
		TArray<FGeoFenceDataStruct>& OutGeoFences,
		TArray<FTakeoffPointDataStruct>& OutTakeoffPoints
	);*/

private:
	sqlite3* dbConnection;

	UFUNCTION()
	void OnPreExitCleanup();

	void InitializeDatabase(bool& outSuccess);

	void InsertInterest(FString name, DBInterestType type, FString pointCSV, int32 groupID, FString& ParamsJson, int32& outID);

	void MoveInterest(int32 InID, FString PointCSV);

	TObjectPtr<UGeoJsonManager> GeoJsonManagerRef;

	const FString LOG_ORIGIN = FString(TEXT("MissionAssetsManager"));
};
