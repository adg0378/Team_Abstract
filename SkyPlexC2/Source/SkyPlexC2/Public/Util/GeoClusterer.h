// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GeoClusterer.generated.h"

USTRUCT()
struct FPointData {
	GENERATED_BODY()

	FString PointName;
	FVector LonLatHeight;
	FGuid ClusterID;

	FPointData() {}
	FPointData(FString InPointName, FVector InLonLatHeight)
		: PointName(InPointName), LonLatHeight(InLonLatHeight), ClusterID() {}
};

USTRUCT()
struct FClusterData {
	GENERATED_BODY()

	FGuid ClusterID;
	TArray<FString> PointNames;
	FVector Center;
	float BufferRadiusNM;

	FClusterData(float InBufferRadiusNM = 5.0)
		: ClusterID(FGuid::NewGuid()), Center(FVector::ZeroVector), BufferRadiusNM(InBufferRadiusNM) {}
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class SKYPLEXC2_API UGeoClusterer : public UObject
{
	GENERATED_BODY()
	
public:
	using FClusterUpdatedCallback = TFunction<void(const FGuid& ClusterID, const FVector& Center, float Radius)>;
	using FClusterRemovedCallback = TFunction<void(const FGuid& ClusterID)>;
	using FClusterMergedCallback = TFunction<void(const FGuid& BaseClusterID, const FGuid& MergedClusterID)>;

	void GetClusters(TMap<FGuid, FClusterData>& OutClusters) const;

	void SetClusterRestrictions(float InMaxClusterRadiusNM, float InPointBufferNM, float InInitialClusterRadiusNM);

	void SetClusterUpdatedCallback(FClusterUpdatedCallback InCallback);
	void SetClusterRemovedCallback(FClusterRemovedCallback InCallback);
	void SetClusterMergedCallback(FClusterMergedCallback InCallback);

	UFUNCTION(BlueprintCallable)
	void AddPoint(const FString& Name, const FVector& LonLatHeight);

	UFUNCTION(BlueprintCallable)
	void MovePoint(const FString& PointName, const FVector& NewLonLatHeight);

	UFUNCTION(BlueprintCallable)
	void RemovePoint(const FString& PointName, bool KeepClusterIfEmpty = false);

	void GetClusterIDAtLocation(const FVector& InLonLatHeight, TSet<FGuid>& OutClusterID);

	void ToggleExternalUpdates(bool InProvideUpdates);

	// Iterates through clusters and calls the updated callback
	void RefreshClusters();

protected:
	void AddCluster(FPointData& InitialPoint);
	void RemoveCluster(const FGuid& ClusterID);
	void UpdateCluster(const FGuid& ClusterID);
	void MergeClusters(const FGuid& ClusterA, const FGuid& ClusterB);
	float DistanceNM(const FVector& LonLatHeightA, const FVector& LonLatHightB);

private:
	UPROPERTY()
	TMap<FString, FPointData> Points;

	UPROPERTY()
	TMap<FGuid, FClusterData> Clusters;

	float MaxClusterRadiusNM = 10.0f;
	float PointBufferNM = 1.0f;
	float InitialClusterRadiusNM = 3.0f;

	bool ProvideUpdates = true;

	FClusterUpdatedCallback OnClusterUpdated = [](const FGuid&, const FVector&, float) {};
	FClusterRemovedCallback OnClusterRemoved = [](const FGuid&) {};
	FClusterMergedCallback OnClusterMerged = [](const FGuid&, const FGuid&) {};
};
