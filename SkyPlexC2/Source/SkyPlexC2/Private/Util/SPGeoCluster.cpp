// Copyright (c) 2025 Synetos Aerospace


#include "Util/SPGeoCluster.h"
#include "CesiumGeospatial/Ellipsoid.h"
#include "Util/SPGeoUtility.h"

void USPGeoCluster::SetClusterRestrictions(float InMaxClusterRadiusNM, float InPointBufferNM, float InInitialClusterRadiusNM) {
	MaxClusterRadiusNM = InMaxClusterRadiusNM;
	PointBufferNM = InPointBufferNM;
	InitialClusterRadiusNM = InInitialClusterRadiusNM;
}

void USPGeoCluster::GetClusters(TMap<FGuid, FClusterData>& OutClusters) const {
	OutClusters = Clusters;
}

void USPGeoCluster::SetClusterUpdatedCallback(FClusterUpdatedCallback InCallback) {
	OnClusterUpdated = InCallback;
}

void USPGeoCluster::SetClusterRemovedCallback(FClusterRemovedCallback InCallback) {
	OnClusterRemoved = InCallback;
}

void USPGeoCluster::SetClusterMergedCallback(FClusterMergedCallback InCallback) {
	OnClusterMerged = InCallback;
}

void USPGeoCluster::ToggleExternalUpdates(bool InProvideUpdates) {
	ProvideUpdates = InProvideUpdates;
}

void USPGeoCluster::RefreshClusters() {
	if (ProvideUpdates) {
		for (auto& [ClusterID, Cluster] : Clusters) {
			OnClusterUpdated(Cluster.ClusterID, Cluster.Center, Cluster.BufferRadiusNM);
		}
	}
}

void USPGeoCluster::GetClusterIDAtLocation(const FVector& InLonLatHeight, TSet<FGuid>& OutClusterIDs) {
	OutClusterIDs.Empty();

	for (auto& [ClusterID, Cluster] : Clusters) {
		float Dist = DistanceNM(InLonLatHeight, Cluster.Center);
		if (Dist < Cluster.BufferRadiusNM) {
			OutClusterIDs.Add(ClusterID);
		}
	}
}

void USPGeoCluster::AddPoint(const FString& Name, const FVector& LonLatHeight) {
	FPointData NewPoint(Name, LonLatHeight);

	FGuid BestClusterID;
	float ClosestDist = FLT_MAX;

	for (auto& [ClusterID, Cluster] : Clusters) {
		float Dist = DistanceNM(LonLatHeight, Cluster.Center);
		if (Dist < ClosestDist) {
			ClosestDist = Dist;
			BestClusterID = ClusterID;
		}
	}

	if (BestClusterID.IsValid() && ClosestDist <= Clusters.Find(BestClusterID)->BufferRadiusNM + InitialClusterRadiusNM && ClosestDist < MaxClusterRadiusNM) {
		// Point should fit inside the cluster, but we may need to expand and/or recenter
		FClusterData* Cluster = Clusters.Find(BestClusterID);
		if (!Cluster) {
			UE_LOG(LogTemp, Error, TEXT("BestClusterID does not exist"))
				return;
		}
		Cluster->PointNames.Add(NewPoint.PointName);
		NewPoint.ClusterID = BestClusterID;
		Points.Add(NewPoint.PointName, NewPoint);
		if (!(ClosestDist <= Cluster->BufferRadiusNM - PointBufferNM)) {
			// Point is less than PointBufferNM from the edge of the cluster, so we will need to fetch some more items
			UpdateCluster(BestClusterID);
		}
	}
	else {
		AddCluster(NewPoint);
	}

	// Check if any clusters can be merged as a result of the point addition
	FGuid ClusterToMerge;
	for (auto& [ClusterID, Cluster] : Clusters) {
		if (ClusterID == NewPoint.ClusterID) {
			continue;
		}

		float Dist = DistanceNM(Cluster.Center, Clusters[NewPoint.ClusterID].Center);
		float CombinedRadius = Cluster.BufferRadiusNM + Clusters[NewPoint.ClusterID].BufferRadiusNM;

		if (Dist <= CombinedRadius && CombinedRadius <= MaxClusterRadiusNM) {
			ClusterToMerge = ClusterID;
			break;
		}
	}

	if (ClusterToMerge.IsValid()) {
		MergeClusters(ClusterToMerge, NewPoint.ClusterID);
	}
}

void USPGeoCluster::AddCluster(FPointData& InitialPoint) {
	FClusterData NewCluster(InitialClusterRadiusNM);
	NewCluster.Center = InitialPoint.LonLatHeight;
	NewCluster.PointNames.Add(InitialPoint.PointName);
	Clusters.Add(NewCluster.ClusterID, NewCluster);

	InitialPoint.ClusterID = NewCluster.ClusterID;
	Points.Add(InitialPoint.PointName, InitialPoint);

	//UE_LOG(LogTemp, Log, TEXT("Created new point cluster: %s %s"), *NewCluster.ClusterID.ToString(), *NewCluster.Center.ToString())

	if (ProvideUpdates) {
		OnClusterUpdated(NewCluster.ClusterID, NewCluster.Center, NewCluster.BufferRadiusNM);
	}
}

void USPGeoCluster::RemoveCluster(const FGuid& ClusterID) {
	FClusterData* Cluster = Clusters.Find(ClusterID);
	if (!Cluster) {
		return;
	}

	if (Cluster->PointNames.Num() == 0) {
		Clusters.Remove(ClusterID);
		//UE_LOG(LogTemp, Log, TEXT("Removing cluster: %s"), *ClusterID.ToString())
		if (ProvideUpdates) {
			OnClusterRemoved(ClusterID);
		}
	}
}

void USPGeoCluster::MovePoint(const FString& PointName, const FVector& NewLonLatHeight) {
	FPointData* Point = Points.Find(PointName);
	if (!Point || Point->LonLatHeight == NewLonLatHeight) {
		return;
	}

	FGuid OldClusterID = Point->ClusterID;

	// This pattern prevents a cluster from being removed and readded if it only has one point and the point only moves a few NM
	RemovePoint(PointName, true);
	AddPoint(PointName, NewLonLatHeight);
	RemoveCluster(OldClusterID);
}

void USPGeoCluster::RemovePoint(const FString& PointName, bool KeepClusterIfEmpty) {
	FPointData* Point = Points.Find(PointName);
	if (!Point) {
		return;
	}

	FGuid ClusterID = Point->ClusterID;
	FClusterData* Cluster = Clusters.Find(ClusterID);
	if (!Cluster) {
		return;
	}

	Cluster->PointNames.Remove(PointName);
	Points.Remove(PointName);
	if (Cluster->PointNames.Num() == 0) {
		if (!KeepClusterIfEmpty) {
			Clusters.Remove(ClusterID);
			UE_LOG(LogTemp, Log, TEXT("Removing cluster: %s"), *ClusterID.ToString())
				if (ProvideUpdates) {
					OnClusterRemoved(ClusterID);
				}
		}
	}
	else {
		UpdateCluster(ClusterID);
	}
}

void USPGeoCluster::UpdateCluster(const FGuid& ClusterID) {
	FClusterData* Cluster = Clusters.Find(ClusterID);
	if (!Cluster) {
		return;
	}

	FVector Sum = FVector::ZeroVector;
	float MaxDist = 0.0f;

	for (const FString& PointName : Cluster->PointNames) {
		FPointData* Point = Points.Find(PointName);
		if (!Point) {
			continue;
		}

		Sum += Point->LonLatHeight;
	}

	FVector PrevCenter = Cluster->Center;
	Cluster->Center = (Cluster->PointNames.Num() > 0) ? Sum / Cluster->PointNames.Num() : FVector::ZeroVector;

	for (const FString& PointName : Cluster->PointNames) {
		FPointData* Point = Points.Find(PointName);
		if (!Point) {
			continue;
		}

		float Dist = DistanceNM(Point->LonLatHeight, Cluster->Center);
		if (Dist > MaxDist) {
			MaxDist = Dist;
		}
	}

	float PrevBufferRadiusNM = Cluster->BufferRadiusNM;
	Cluster->BufferRadiusNM = FMath::Clamp(MaxDist + PointBufferNM, InitialClusterRadiusNM, MaxClusterRadiusNM);

	if (ProvideUpdates &&
		(DistanceNM(PrevCenter, Cluster->Center) >= 0.8 * PointBufferNM ||
			(Cluster->BufferRadiusNM > PrevBufferRadiusNM ||
				PrevBufferRadiusNM - Cluster->BufferRadiusNM >= 0.45 * PrevBufferRadiusNM
				))) {
		OnClusterUpdated(ClusterID, Cluster->Center, Cluster->BufferRadiusNM);
	}
}

void USPGeoCluster::MergeClusters(const FGuid& ClusterAID, const FGuid& ClusterBID) {
	FClusterData* ClusterA = Clusters.Find(ClusterAID);
	FClusterData* ClusterB = Clusters.Find(ClusterBID);
	if (!ClusterA || !ClusterB) {
		return;
	}

	//UE_LOG(LogTemp, Log, TEXT("Merging clusters: %s %s"), *ClusterAID.ToString(), *ClusterBID.ToString())

	for (const FString& PointName : ClusterB->PointNames) {
		ClusterA->PointNames.Add(PointName);
		FPointData* Point = Points.Find(PointName);
		if (Point) {
			Point->ClusterID = ClusterAID;
		}
	}

	Clusters.Remove(ClusterBID);
	if (ProvideUpdates) {
		OnClusterMerged(ClusterAID, ClusterBID);
	}
	UpdateCluster(ClusterAID);
}

float USPGeoCluster::DistanceNM(const FVector& LonLatHeightA, const FVector& LonLatHeightB) {
	const CesiumGeospatial::Ellipsoid& Ellipsoid = CesiumGeospatial::Ellipsoid::WGS84;
	return USPGeoUtility::VincentyInverseFormula(
		Ellipsoid.getMaximumRadius(),
		Ellipsoid.getMinimumRadius(),
		FMath::DegreesToRadians(LonLatHeightA.X),
		FMath::DegreesToRadians(LonLatHeightB.X),
		FMath::DegreesToRadians(LonLatHeightA.Y),
		FMath::DegreesToRadians(LonLatHeightB.Y)
	) / 1852.0;
}