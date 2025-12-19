// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "State/SPManager.h"
#include "State/Obstacles/ObstaclesUtil.h"
#include "Util/GeoClusterer.h"
#include "SPObstacleManager.generated.h"

// Forward declarations
class UInterest;
class AFAADOFObstacle;
class AADSBObject;

USTRUCT()
struct FFAAClusterEntry {
	GENERATED_BODY()

public:
	TSet<FGuid> ClusterAssignments;
	TObjectPtr<AFAADOFObstacle> FAAObstacle;
};

USTRUCT()
struct FADSBClusterEntry {
	GENERATED_BODY()

public:
	TSet<FGuid> ClusterAssignments;
	FVector LastPoint;
	FVector NewPoint;
	TObjectPtr<AADSBObject> ADSBObject;
	float ElapsedLerpTime = 0.0f;
};

USTRUCT()
struct FFAACacheEntry {
	GENERATED_BODY()

public:
	UPROPERTY()
	FVector LonLatHeight;

	UPROPERTY()
	FFAAObjectDataStruct Data;
};

USTRUCT()
struct FFAACache {
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FFAACacheEntry> Obstacles;
};

USTRUCT(BlueprintType)
struct FADSBCacheEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector LonLatHeight;

	UPROPERTY(BlueprintReadWrite)
	FADSBAircraftStruct Data; 
};

/**
 * Manages ground and air obstacles like FAA objects and ADSB aircraft
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API USPObstacleManager : public USPManager, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Obstacle")
	TSubclassOf<AFAADOFObstacle> FAADOFObstacleToSpawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Obstacle")
	TSubclassOf<AADSBObject> ADSBObstacleToSpawn;


	void Setup_Implementation(bool& outSuccess) override;
	void Teardown_Implementation() override;
	void ApplyPreferencesUpdates_Implementation(FSPPreferencesStruct PrevPreferences, FSPPreferencesStruct NewPreferences) override;

	void HandleNewInterest(UInterest* Interest);
	void HandleDeleteInterest(UInterest* Interest);
	void HandleMoveInterest(UInterest* Interest);

	void OnClusterUpdated(const FGuid& ClusterID, const FVector& Center, float Radius);
	void OnClusterRemoved(const FGuid& ClusterID);
	void OnClusterMerged(const FGuid& BaseClusterID, const FGuid& MergedClusterID);
	void OnADSBClusterUpdated(const FGuid& ClusterID, const FVector& Center, float Radius);
	void OnADSBClusterRemoved(const FGuid& ClusterID);
	void OnADSBClusterMerged(const FGuid& BaseClusterID, const FGuid& MergedClusterID);

	void Tick(float DeltaTime) override;
	bool IsTickable() const override;
	bool IsTickableInEditor() const override;
	bool IsTickableWhenPaused() const override;
	TStatId GetStatId() const override;

	virtual void CullRelatedObjects(float MaximumDrawDistance, const FVector& OriginLocationUE, bool FromFlyTo = false) override;

private:
	void LoadInitialInterestClusters();
	void SpawnObstaclesAroundInterest(const FGuid& ClusterID, const TArray<FFAAObjectDataStruct>& Obstacles);
	void SpawnADSBObjectsAroundInterest(const FGuid& ClusterID, const TArray<FADSBAircraftStruct>& ADSBObjects);

	void WriteFAAObstacleCache(const FFAACache& Obstacles);
	void ReadFAAObstacleCache(FFAACache& OutObstacles, bool& OutFailed);

	void StartADSBQueryTimer();
	void StopADSBQueryTimer();
	void QueryADSB();

	UPROPERTY()
	TMap<FString, FFAAClusterEntry> FAAObjects;

	UPROPERTY()
	TMap<FString, FADSBClusterEntry> ADSBObjects;	

	UPROPERTY()
	TObjectPtr<UGeoClusterer> FAAClusters;

	UPROPERTY()
	TObjectPtr<UGeoClusterer> ADSBClusters;

	UPROPERTY()
	FTimerHandle ADSBQueryTimerHandle;

	const float ADSBTimerTickTime = 1.5f;
	bool TimerActive = false;

	const FString LOG_ORIGIN = TEXT("ObstacleManager");

	bool CanTick = false;
};
