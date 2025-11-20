// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Core/State/SPManagerBase.h"
#include "Util/SPObstacleUtility.h"
#include "SPObstacleManager.generated.h"

USTRUCT()
struct FFAAClusterEntry {
	GENERATED_BODY()

public:
	TSet<FGuid> ClusterAssignments;
	TObjectPtr<class ASPFAADOF> FAAObstacle;
};

USTRUCT()
struct FADSBClusterEntry {
	GENERATED_BODY()

public:
	TSet<FGuid> ClusterAssignments;
	FVector LastPoint;
	FVector NewPoint;
	TObjectPtr<class ASPADSB> ADSBObject;
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
class SKYPLEXC2_API USPObstacleManager : public USPManagerBase, public FTickableGameObject
{
	GENERATED_BODY()
	

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Obstacle")
	TSubclassOf<ASPFAADOF> FAADOFObstacleToSpawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Obstacle")
	TSubclassOf<ASPADSB> ADSBObstacleToSpawn;

	USPObstacleManager();


	void Setup_Implementation() override;
	void Teardown_Implementation() override;
	void ApplyPreferencesUpdates_Implementation(FSPPreferencesStruct PrevPreferences, FSPPreferencesStruct NewPreferences) override;

	void HandleNewInterest(class USPInterest* Interest);
	void HandleDeleteInterest(USPInterest* Interest);
	void HandleMoveInterest(USPInterest* Interest);

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

	virtual void CullRelatedObjects(float MaximumDrawDistance, const FVector& OriginLocationUE);

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
	TObjectPtr<class USPGeoCluster> FAAClusters;

	UPROPERTY()
	TObjectPtr<USPGeoCluster> ADSBClusters;

	UPROPERTY()
	FTimerHandle ADSBQueryTimerHandle;

	const float ADSBTimerTickTime = 1.5f;
	bool TimerActive = false;

	const FString LOG_ORIGIN = TEXT("ObstacleManager");

	bool CanTick = false;
};

