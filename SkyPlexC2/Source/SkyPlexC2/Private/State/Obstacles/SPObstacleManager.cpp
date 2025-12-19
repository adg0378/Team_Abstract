// Copyright (c) 2025 Synetos Aerospace


#include "State/Obstacles/SPObstacleManager.h"
#include "Objects/Interests/POI.h"
#include "Objects/Interests/AreaOfInterest.h"
#include "Objects/Interests/SPGeoFence.h"
#include "Objects/Interests/SPTakeoffPoint.h"
#include "Objects/Interests/Interest.h"
#include "State/Missions/InterestsUtil.h"
#include "State/Missions/SPMissionManager.h"
#include "Objects/Geometry/PlaceablePoint.h"
#include "Objects/Geometry/Polygon.h"
#include "State/SPGameState.h"
#include "SPUtility.h"
#include "State/Obstacles/ObstaclesUtil.h"
#include "Objects/InteractionBoxProvider.h"
#include "Objects/Obstacles/FAADOFObstacle.h"
#include <CesiumGlobeAnchorComponent.h>
#include "State/PreferencesManager.h"
#include "JsonObjectConverter.h"
#include "State/SPWorldManager.h"
#include "Objects/Obstacles/ADSBObject.h"
#include "FileUtility.h"

void USPObstacleManager::OnClusterUpdated(const FGuid& ClusterID, const FVector& Center, float Radius) {
	if (!PreferencesManagerRef->GetPreferencesRef().ObstaclesPreferences.ShowFAAObstacles) {
		return;
	}
	UObstaclesUtil::GetFAADOFObjects(LOG, LOG_ORIGIN, Center.Y, Center.X, Radius)
		.Then([this, ClusterID](TFuture<TArray<FFAAObjectDataStruct>> ResFut) {
			const TArray<FFAAObjectDataStruct>& Result = ResFut.Get();
			TArray<FFAAObjectDataStruct> FilteredResults;

			for (const FFAAObjectDataStruct& Data : Result) {
				if (!FAAObjects.Contains(Data.oas)) {
					FilteredResults.Add(Data);
				}
				else if (!FAAObjects[Data.oas].ClusterAssignments.Contains(ClusterID)) {
					FAAObjects[Data.oas].ClusterAssignments.Add(ClusterID);
				}
			}

			if (FilteredResults.Num() > 0) {
				AsyncTask(ENamedThreads::GameThread, [this, ClusterID, FilteredResults]() {
					this->SpawnObstaclesAroundInterest(ClusterID, FilteredResults);
				});
			}
		});
}

void USPObstacleManager::OnClusterRemoved(const FGuid& ClusterID) {
	if (!PreferencesManagerRef->GetPreferencesRef().ObstaclesPreferences.ShowFAAObstacles) {
		return;
	}
	for (auto It = FAAObjects.CreateIterator(); It; ++It) {
		if (It->Value.ClusterAssignments.Contains(ClusterID)) {
			if (It->Value.ClusterAssignments.Num() == 1) {
				IInteractionBoxProvider::Execute_DestroySelf(It->Value.FAAObstacle);
				It.RemoveCurrent();
			}
			else {
				It->Value.ClusterAssignments.Remove(ClusterID);
			}
		}
	}
}

void USPObstacleManager::OnClusterMerged(const FGuid& BaseClusterID, const FGuid& MergedClusterID) {
	if (!PreferencesManagerRef->GetPreferencesRef().ObstaclesPreferences.ShowFAAObstacles) {
		return;
	}
	for (TPair<FString, FFAAClusterEntry>& Pair : FAAObjects) {
		if (Pair.Value.ClusterAssignments.Remove(MergedClusterID)) {
			Pair.Value.ClusterAssignments.Add(BaseClusterID);
		}
	}
	
}
void USPObstacleManager::OnADSBClusterUpdated(const FGuid& ClusterID, const FVector& Center, float Radius) {
	if (!PreferencesManagerRef->GetPreferencesRef().ObstaclesPreferences.ShowADSB) {
		return;
	}
	UObstaclesUtil::GetADSBObjects(LOG, LOG_ORIGIN, Center.X, Center.Y, Radius)
		.Then([this, ClusterID](TFuture<FADSBObjectDataResult> ResFut) {
			if (!TimerActive) {
				return;
			}
			const FADSBObjectDataResult& Result = ResFut.Get();

			TSet<FString> AircraftIDs;
			TArray<FString> IDsToRemove;
			bool SpawnGrounded = gameStateRef->PreferencesManager->GetPreferencesRef().ObstaclesPreferences.RenderGroundedADSB;

			for (FADSBAircraftStruct Data : Result.ac) {
				if (SpawnGrounded || Data.alt_baro != "ground") {
					AircraftIDs.Add(FString::Printf(TEXT("%s_%s"), *Data.hex, *Data.flight));
				}
			}

			for (auto It = ADSBObjects.CreateIterator(); It; ++It) {
				if (It->Value.ClusterAssignments.Contains(ClusterID) && !AircraftIDs.Contains(It->Key)) {
					IDsToRemove.Add(It->Key);
				}
			}

			if (Result.ac.Num() > 0 || IDsToRemove.Num() > 0) {
				AsyncTask(ENamedThreads::GameThread, [this, ClusterID, Result, IDsToRemove]() {
					if (IDsToRemove.Num() > 0) {
						for (int i = 0; i < IDsToRemove.Num(); ++i) {
							FString ID = IDsToRemove[i];
							FADSBClusterEntry* Entry = ADSBObjects.Find(ID);
							if (Entry) {
								if (Entry->ClusterAssignments.Num() == 1) {
									IInteractionBoxProvider::Execute_DestroySelf(Entry->ADSBObject.Get());
									ADSBObjects.Remove(ID);
								}
								else {
									Entry->ClusterAssignments.Remove(ClusterID);
								}
							}
						}
					}

					if (Result.ac.Num() > 0) {
						this->SpawnADSBObjectsAroundInterest(ClusterID, Result.ac);
					}
				});
			}
		});
}

void USPObstacleManager::OnADSBClusterRemoved(const FGuid& ClusterID) {
	if (!PreferencesManagerRef->GetPreferencesRef().ObstaclesPreferences.ShowADSB) {
		return;
	}
	for (auto It = ADSBObjects.CreateIterator(); It; ++It) {
		if (It->Value.ClusterAssignments.Contains(ClusterID)) {
			if (It->Value.ClusterAssignments.Num() == 1) {
				IInteractionBoxProvider::Execute_DestroySelf(It->Value.ADSBObject.Get());
				It.RemoveCurrent();
			}
			else {
				It->Value.ClusterAssignments.Remove(ClusterID);
			}
		}
	}
}

void USPObstacleManager::OnADSBClusterMerged(const FGuid& BaseClusterID, const FGuid& MergedClusterID) {
	if (!PreferencesManagerRef->GetPreferencesRef().ObstaclesPreferences.ShowADSB) {
		return;
	}
	for (TPair<FString, FADSBClusterEntry>& Pair : ADSBObjects) {
		if (Pair.Value.ClusterAssignments.Remove(MergedClusterID)) {
			Pair.Value.ClusterAssignments.Add(BaseClusterID);
		}
	}
}

void USPObstacleManager::Setup_Implementation(bool& outSuccess) {
	BindToPreferencesUpdates = true;
	Super::Setup_Implementation(outSuccess);
	FAAClusters = NewObject<UGeoClusterer>(this);
	FAAClusters->SetClusterRestrictions(5.0, 0.75, 3.0);
	FAAClusters->SetClusterUpdatedCallback(
		[this](const FGuid& ClusterID, const FVector& Center, float Radius) {
			this->OnClusterUpdated(ClusterID, Center, Radius);
		}
	);
	FAAClusters->SetClusterRemovedCallback(
		[this](const FGuid& ClusterID) {
			this->OnClusterRemoved(ClusterID);
		}
	);
	FAAClusters->SetClusterMergedCallback(
		[this](const FGuid& BaseClusterID, const FGuid& MergedClusterID) {
			this->OnClusterMerged(BaseClusterID, MergedClusterID);
		}
	);

	ADSBClusters = NewObject<UGeoClusterer>(this);
	ADSBClusters->SetClusterRestrictions(35.0, 5.0, 15.0);
	ADSBClusters->SetClusterUpdatedCallback(
		[this](const FGuid& ClusterID, const FVector& Center, float Radius) {
			this->OnADSBClusterUpdated(ClusterID, Center, Radius);
		}
	);
	ADSBClusters->SetClusterRemovedCallback(
		[this](const FGuid& ClusterID) {
			this->OnADSBClusterRemoved(ClusterID);
		}
	);
	ADSBClusters->SetClusterMergedCallback(
		[this](const FGuid& BaseClusterID, const FGuid& MergedClusterID) {
			this->OnADSBClusterMerged(BaseClusterID, MergedClusterID);
		}
	);


	LoadInitialInterestClusters();
	CanTick = PreferencesManagerRef->GetPreferencesRef().ObstaclesPreferences.InterpolateADSBLocations;
	StartADSBQueryTimer();
}

void USPObstacleManager::ApplyPreferencesUpdates_Implementation(FSPPreferencesStruct PrevPreferences, FSPPreferencesStruct NewPreferences) {
	Super::ApplyPreferencesUpdates_Implementation(PrevPreferences, NewPreferences);
	if (PrevPreferences.ObstaclesPreferences.ShowFAAObstacles != NewPreferences.ObstaclesPreferences.ShowFAAObstacles) {
		if (NewPreferences.ObstaclesPreferences.ShowFAAObstacles) {
			FAAClusters->RefreshClusters();
		}
		else {
			for (const TPair<FString, FFAAClusterEntry>& Entry : FAAObjects) {
				IInteractionBoxProvider::Execute_DestroySelf(Entry.Value.FAAObstacle);
			}
			FAAObjects.Empty();
		}
	}

	if (PrevPreferences.ObstaclesPreferences.ShowADSB != NewPreferences.ObstaclesPreferences.ShowADSB) {
		if (NewPreferences.ObstaclesPreferences.ShowADSB) {
			ADSBClusters->RefreshClusters();
			StartADSBQueryTimer();
		}
		else {
			StopADSBQueryTimer();
			for (const TPair<FString, FADSBClusterEntry>& Entry : ADSBObjects) {
				IInteractionBoxProvider::Execute_DestroySelf(Entry.Value.ADSBObject);
			}
			ADSBObjects.Empty();
		}
	}

	if (PrevPreferences.ObstaclesPreferences.InterpolateADSBLocations != NewPreferences.ObstaclesPreferences.InterpolateADSBLocations) {
		CanTick = NewPreferences.ObstaclesPreferences.InterpolateADSBLocations;
	}

	if (PrevPreferences.ObstaclesPreferences.ADSBFlightTrailLength != NewPreferences.ObstaclesPreferences.ADSBFlightTrailLength) {
		for (const TPair<FString, FADSBClusterEntry>& Entry : ADSBObjects) {
			Entry.Value.ADSBObject->SetTrailLength(NewPreferences.ObstaclesPreferences.ADSBFlightTrailLength);
		}
	}
}

void USPObstacleManager::Teardown_Implementation() {
	Super::Teardown_Implementation();
	StopADSBQueryTimer();
	TArray<FFAACacheEntry> CacheObjects;
	for (TPair<FString, FFAAClusterEntry>& Pair : FAAObjects) {
		FFAAObjectDataStruct Data;
		FVector LonLatHeight;
		Pair.Value.FAAObstacle->GetData(Data);
		Pair.Value.FAAObstacle->GetLongitudeLatitudeHeight(LonLatHeight);
		LonLatHeight.Z = Pair.Value.FAAObstacle->InitialHeight;
		CacheObjects.Add(FFAACacheEntry{ .LonLatHeight = LonLatHeight, .Data = Data });
	}
	WriteFAAObstacleCache(FFAACache{ .Obstacles = CacheObjects });
}

void USPObstacleManager::WriteFAAObstacleCache(const FFAACache& Obstacles) {
	FString JsonString;

	if (!FJsonObjectConverter::UStructToJsonObjectString<FFAACache>(Obstacles, JsonString)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to stringify faa cache data"));
	}


	if (!FileUtility::WriteStringToFile(*USPEnvConstants::GetObstacleCachePath(), *JsonString)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to save faa cache data"));
	}
}

void USPObstacleManager::ReadFAAObstacleCache(FFAACache& OutObstacles, bool& OutFailed) {
	if (FPaths::FileExists(USPEnvConstants::GetObstacleCachePath())) {
		FString FileContents;

		if (!FFileHelper::LoadFileToString(FileContents, *USPEnvConstants::GetObstacleCachePath())) {
			UE_LOG(LogTemp, Error, TEXT("Failed to load faa cache data"));
			OutFailed = true;
			return;
		}

		FFAACache ParsedCache;
		if (!FJsonObjectConverter::JsonObjectStringToUStruct<FFAACache>(FileContents, &ParsedCache, 0, 0)) {
			UE_LOG(LogTemp, Error, TEXT("Failed to parse faa cache data"));
			OutFailed = true;
		}
		else {
			OutObstacles = ParsedCache;
			OutFailed = false;
		}
	}
	else {
		OutFailed = true;
	}
}

void USPObstacleManager::LoadInitialInterestClusters() {
	FAAClusters->ToggleExternalUpdates(false);
	ADSBClusters->ToggleExternalUpdates(false);
	const TMap<int32, FSPInterestStruct>& Interests = gameStateRef->MissionManagerRef->GetInterests();

	for (const TPair<int32, FSPInterestStruct>& Pair : Interests) {
		for (const TPair<int32, UInterest*> Interest : Pair.Value.Interests) {
			EInterestType InterestType = Interest.Value->GetInterestType();

			if (InterestType == EInterestType::POI || InterestType == EInterestType::Takeoff) {
				APlaceablePoint* Point;

				if (InterestType == EInterestType::POI) {
					UPOI* POI = Cast<UPOI>(Interest.Value);
					POI->GetPoint(Point);
				}
				else {
					USPTakeoffPoint* Takeoff = Cast<USPTakeoffPoint>(Interest.Value);
					Takeoff->GetPoint(Point);
				}

				FVector LonLatHeight;
				FString Name;
				Point->GetLongitudeLatitudeHeight(LonLatHeight);
				Point->GetName(Name);
				FAAClusters->AddPoint(Name, LonLatHeight);
				ADSBClusters->AddPoint(Name, LonLatHeight);
			}
			else if (InterestType == EInterestType::AOI || InterestType == EInterestType::GeoFence) {
				APolygon* Polygon;

				if (InterestType == EInterestType::AOI) {
					UAreaOfInterest* AOI = Cast<UAreaOfInterest>(Interest.Value);
					AOI->GetPolygon(Polygon);
				}
				else {
					USPGeoFence* GeoFence = Cast<USPGeoFence>(Interest.Value);
					GeoFence->GetPolygon(Polygon);
				}

				Polygon->ForEachPoint([&](APlaceablePoint* Point) {
					FVector LonLatHeight;
					FString Name;
					Point->GetLongitudeLatitudeHeight(LonLatHeight);
					Point->GetName(Name);
					FAAClusters->AddPoint(Name, LonLatHeight);
					ADSBClusters->AddPoint(Name, LonLatHeight);
					});
			}
		}
	}

	FAAClusters->ToggleExternalUpdates(true);
	ADSBClusters->ToggleExternalUpdates(true);

	FFAACache Cache;
	bool Failed;
	ReadFAAObstacleCache(Cache, Failed);
	if (Failed || Cache.Obstacles.IsEmpty()) {
		FAAClusters->RefreshClusters();
	}
	else {
		for (FFAACacheEntry& Entry : Cache.Obstacles) {
			TSet<FGuid> ClusterIDs;
			FAAClusters->GetClusterIDAtLocation(Entry.LonLatHeight, ClusterIDs);

			if (ClusterIDs.Num() > 0) {
				AFAADOFObstacle* Obstacle = nullptr;
				gameStateRef->GenericSpawnActorAtLonLatHeight<AFAADOFObstacle>(FAADOFObstacleToSpawn, Entry.LonLatHeight, Obstacle);
				if (Obstacle == nullptr) {
					LOG->Error(FString::Printf(TEXT("Unable to spawn FAA object with id '%s'"), *Entry.Data.oas), LOG_ORIGIN);
					continue;
				}

				Obstacle->SetData(Entry.Data);
				Obstacle->InitialHeight = Entry.LonLatHeight.Z;
				FAAObjects.Add(Entry.Data.oas, FFAAClusterEntry{ .ClusterAssignments = ClusterIDs, .FAAObstacle = Obstacle });
			}
		}
	}

	StartADSBQueryTimer();
}

void USPObstacleManager::HandleNewInterest(UInterest* Interest) {
	TArray<FFAAObjectDataStruct> results;

	EInterestType InterestType = Interest->GetInterestType();
	if (InterestType == EInterestType::POI || InterestType == EInterestType::Takeoff) {
		APlaceablePoint* Point;

		if (InterestType == EInterestType::POI) {
			UPOI* POI = Cast<UPOI>(Interest);
			POI->GetPoint(Point);
		}
		else {
			USPTakeoffPoint* Takeoff = Cast<USPTakeoffPoint>(Interest);
			Takeoff->GetPoint(Point);
		}

		FVector LonLatHeight;
		FString Name;
		Point->GetLongitudeLatitudeHeight(LonLatHeight);
		Point->GetName(Name);
		FAAClusters->AddPoint(Name, LonLatHeight);
		ADSBClusters->AddPoint(Name, LonLatHeight);
	}
	else if (InterestType == EInterestType::GeoFence || InterestType == EInterestType::AOI) {
		// For now, each individual AOI point is added. 
		// This is not the best, as for larger AOIs, there may be area in between that is not included in a cluster
		// However, this works well enough for now
		APolygon* Polygon;

		if (InterestType == EInterestType::AOI) {
			UAreaOfInterest* AOI = Cast<UAreaOfInterest>(Interest);
			AOI->GetPolygon(Polygon);
		}
		else {
			USPGeoFence* GeoFence = Cast<USPGeoFence>(Interest);
			GeoFence->GetPolygon(Polygon);
		}

		Polygon->ForEachPoint([&](APlaceablePoint* Point) {
			FVector LonLatHeight;
			FString Name;
			Point->GetLongitudeLatitudeHeight(LonLatHeight);
			Point->GetName(Name);
			FAAClusters->AddPoint(Name, LonLatHeight);
			ADSBClusters->AddPoint(Name, LonLatHeight);
		});
	}
}

void USPObstacleManager::HandleDeleteInterest(UInterest* Interest) {
	EInterestType InterestType = Interest->GetInterestType();
	if (InterestType == EInterestType::POI || InterestType == EInterestType::Takeoff) {
		APlaceablePoint* Point;

		if (InterestType == EInterestType::POI) {
			UPOI* POI = Cast<UPOI>(Interest);
			POI->GetPoint(Point);
		}
		else {
			USPTakeoffPoint* Takeoff = Cast<USPTakeoffPoint>(Interest);
			Takeoff->GetPoint(Point);
		}

		FString Name;
		Point->GetName(Name);
		FAAClusters->RemovePoint(Name);
		ADSBClusters->RemovePoint(Name);
	}
	else if (InterestType == EInterestType::AOI || InterestType == EInterestType::GeoFence) {
		APolygon* Polygon;

		if (InterestType == EInterestType::AOI) {
			UAreaOfInterest* AOI = Cast<UAreaOfInterest>(Interest);
			AOI->GetPolygon(Polygon);
		}
		else {
			USPGeoFence* GeoFence = Cast<USPGeoFence>(Interest);
			GeoFence->GetPolygon(Polygon);
		}

		Polygon->ForEachPoint([&](APlaceablePoint* Point) {
			FString Name;
			Point->GetName(Name);
			FAAClusters->RemovePoint(Name);
			ADSBClusters->RemovePoint(Name);
		});
	}
}

void USPObstacleManager::HandleMoveInterest(UInterest* Interest) {
	EInterestType InterestType = Interest->GetInterestType();
	if (InterestType == EInterestType::POI || InterestType == EInterestType::Takeoff) {
		APlaceablePoint* Point;

		if (InterestType == EInterestType::POI) {
			UPOI* POI = Cast<UPOI>(Interest);
			POI->GetPoint(Point);
		}
		else {
			USPTakeoffPoint* Takeoff = Cast<USPTakeoffPoint>(Interest);
			Takeoff->GetPoint(Point);
		}

		FVector LonLatHeight;
		FString Name;
		Point->GetLongitudeLatitudeHeight(LonLatHeight);
		Point->GetName(Name);
		FAAClusters->MovePoint(Name, LonLatHeight);
		ADSBClusters->MovePoint(Name, LonLatHeight);
	}
	else if (InterestType == EInterestType::AOI || InterestType == EInterestType::GeoFence) {
		APolygon* Polygon;

		if (InterestType == EInterestType::AOI) {
			UAreaOfInterest* AOI = Cast<UAreaOfInterest>(Interest);
			AOI->GetPolygon(Polygon);
		}
		else {
			USPGeoFence* GeoFence = Cast<USPGeoFence>(Interest);
			GeoFence->GetPolygon(Polygon);
		}

		Polygon->ForEachPoint([&](APlaceablePoint* Point) {
			FVector LonLatHeight;
			FString Name;
			Point->GetLongitudeLatitudeHeight(LonLatHeight);
			Point->GetName(Name);
			FAAClusters->MovePoint(Name, LonLatHeight);
			ADSBClusters->MovePoint(Name, LonLatHeight);
		});
	}
}

void USPObstacleManager::SpawnObstaclesAroundInterest(const FGuid& ClusterID, const TArray<FFAAObjectDataStruct>& Obstacles) {
	LOG->Info(FString::Printf(TEXT("Found %d obstacles"), Obstacles.Num()), LOG_ORIGIN);

	TArray<FVector> LonLatHeightsToSample;
	LonLatHeightsToSample.Reserve(Obstacles.Num());

	for (const FFAAObjectDataStruct& ObstacleData : Obstacles) {
		LonLatHeightsToSample.Push(FVector(ObstacleData.longitude, ObstacleData.latitude, 0.0));
	}

	gameStateRef->SampleHeights(LonLatHeightsToSample, [Obstacles, this, ClusterID](const TArray<FVector>& SampledPositions) {
		if (Obstacles.Num() != SampledPositions.Num()) {
			LOG->Error(TEXT("Failed to spawn FAA obstacles: Misaligned height samples"), LOG_ORIGIN);
			return;
		}

		for (int i = 0; i < SampledPositions.Num(); i++) {
			FVector Position = SampledPositions[i];
			const FFAAObjectDataStruct& ObstacleData = Obstacles[i];

			if (Position.Z <= -9999.0) {
				continue;
			}

			AFAADOFObstacle* Obstacle = nullptr;
			gameStateRef->GenericSpawnActorAtLonLatHeight<AFAADOFObstacle>(FAADOFObstacleToSpawn, Position, Obstacle);
			if (Obstacle == nullptr) {
				LOG->Error(FString::Printf(TEXT("Unable to spawn FAA object with id '%s'"), *ObstacleData.oas), LOG_ORIGIN);
				continue;
			}

			Obstacle->SetData(ObstacleData);
			Obstacle->InitialHeight = Position.Z;
			FAAObjects.Add(ObstacleData.oas, FFAAClusterEntry{ .ClusterAssignments = TSet({ ClusterID }), .FAAObstacle = Obstacle });
		}
	});
}

void USPObstacleManager::SpawnADSBObjectsAroundInterest(const FGuid& ClusterID, const TArray<FADSBAircraftStruct>& Objects) {
	if (!TimerActive) {
		return;
	}

	TArray<FVector> LonLatHeightsToSample;
	TArray<int> AircraftRequiringHeightSampleIndecies;
	FObstaclesPreferencesStruct Prefs = gameStateRef->PreferencesManager->GetPreferencesRef().ObstaclesPreferences;

	for (int32 i = 0; i < Objects.Num(); ++i) {
		const FADSBAircraftStruct& ADSBData = Objects[i];
		if (ADSBData.alt_geom == 0) {
			if (Prefs.RenderGroundedADSB || ADSBData.alt_baro != "ground") {
				LonLatHeightsToSample.Push(FVector(ADSBData.lon, ADSBData.lat, 0.0));
				AircraftRequiringHeightSampleIndecies.Push(i);
			}
			continue;
		}

		float HeightAboveEllipsoidFeet = ADSBData.alt_geom;

		FVector SpawnPosition(ADSBData.lon, ADSBData.lat, ADSBData.alt_geom * 0.3048);

		FString ID = FString::Printf(TEXT("%s_%s"), *ADSBData.hex, *ADSBData.flight);

		FADSBClusterEntry* Entry = ADSBObjects.Find(ID);
		if (Entry) {
			Entry->LastPoint = Entry->NewPoint;
			Entry->NewPoint = SpawnPosition;
			Entry->ADSBObject->cesiumGlobeAnchor->MoveToLongitudeLatitudeHeight(SpawnPosition);
			Entry->ElapsedLerpTime = 0.0f;
			Entry->ADSBObject->SetData(ADSBData);
			Entry->ClusterAssignments.Add(ClusterID);
		}
		else {
			AADSBObject* Obstacle = nullptr;
			gameStateRef->GenericSpawnActorAtLonLatHeight<AADSBObject>(ADSBObstacleToSpawn, SpawnPosition, Obstacle);
			if (Obstacle == nullptr) {
				LOG->Error(FString::Printf(TEXT("Unable to spawn ADSB object with id '%s'"), *ID), LOG_ORIGIN);
				continue;
			}
			Obstacle->SetTrailLength(Prefs.ADSBFlightTrailLength);
			Obstacle->SetData(ADSBData);
			ADSBObjects.Add(
				ID,
				FADSBClusterEntry{
					.ClusterAssignments = TSet({ ClusterID }),
					.LastPoint = SpawnPosition,
					.NewPoint = SpawnPosition,
					.ADSBObject = Obstacle
				}
			);
		}
	}

	if (LonLatHeightsToSample.Num() == 0) {
		return;
	}

	gameStateRef->SampleHeights(LonLatHeightsToSample, [AircraftRequiringHeightSampleIndecies, Objects, this, ClusterID, Prefs](const TArray<FVector>& SampledPositions) {
		if (!TimerActive) {
			return;
		}

		if (AircraftRequiringHeightSampleIndecies.Num() != SampledPositions.Num()) {
			LOG->Error(TEXT("Failed to spawn ADSB objects: Misaligned height samples"), LOG_ORIGIN);
			return;
		}

		for (int i = 0; i < SampledPositions.Num(); i++) {
			FVector GroundPosition = SampledPositions[i];
			const FADSBAircraftStruct& ADSBData = Objects[AircraftRequiringHeightSampleIndecies[i]];

			if (GroundPosition.Z <= -9999.0) {
				continue;
			}

			float AltitudeFeet = ADSBData.alt_baro.IsNumeric() ? FCString::Atof(*ADSBData.alt_baro) : 0;

			FVector SpawnPosition(GroundPosition.X, GroundPosition.Y, GroundPosition.Z + (AltitudeFeet * 0.3048));

			FString ID = FString::Printf(TEXT("%s_%s"), *ADSBData.hex, *ADSBData.flight);
			
			FADSBClusterEntry* Entry = ADSBObjects.Find(ID);
			if (Entry) {
				Entry->LastPoint = Entry->NewPoint;
				Entry->NewPoint = SpawnPosition;
				Entry->ADSBObject->cesiumGlobeAnchor->MoveToLongitudeLatitudeHeight(SpawnPosition);
				Entry->ElapsedLerpTime = 0.0f;
				Entry->ADSBObject->SetData(ADSBData);
				Entry->ClusterAssignments.Add(ClusterID);
			}
			else {
				AADSBObject* Obstacle = nullptr;
				gameStateRef->GenericSpawnActorAtLonLatHeight<AADSBObject>(ADSBObstacleToSpawn, SpawnPosition, Obstacle);
				if (Obstacle == nullptr) {
					LOG->Error(FString::Printf(TEXT("Unable to spawn ADSB object with id '%s'"), *ID), LOG_ORIGIN);
					continue;
				}
				Obstacle->SetTrailLength(Prefs.ADSBFlightTrailLength);
				Obstacle->SetData(ADSBData);
				ADSBObjects.Add(
					ID,
					FADSBClusterEntry{
						.ClusterAssignments = TSet({ ClusterID }),
						.LastPoint = SpawnPosition,
						.NewPoint = SpawnPosition,
						.ADSBObject = Obstacle 
					}
				);
			}
		}
	});
}

void USPObstacleManager::StartADSBQueryTimer() {
	FTimerManagerTimerParameters timerParams;
	timerParams.bLoop = timerParams.bMaxOncePerFrame = true;
	TimerActive = true;
	GetWorld()->GetTimerManager().SetTimer(ADSBQueryTimerHandle, this, &USPObstacleManager::QueryADSB, ADSBTimerTickTime, timerParams);
}

void USPObstacleManager::StopADSBQueryTimer() {
	TimerActive = false;
	GetWorld()->GetTimerManager().ClearTimer(ADSBQueryTimerHandle);
}

void USPObstacleManager::QueryADSB() {
	if (TimerActive) {
		TMap<FGuid, FClusterData> Clusters;
		ADSBClusters->GetClusters(Clusters);
		for (const TPair<FGuid, FClusterData>& Pair : Clusters) {
			OnADSBClusterUpdated(Pair.Value.ClusterID, Pair.Value.Center, Pair.Value.BufferRadiusNM);
		}
	}
}

void USPObstacleManager::Tick(float DeltaTime) {
	if (!CanTick) {
		return;
	}

	/*int32 MaxLerpDistanceCM = 1000000;*/
	/*ACesiumGeoreference* Georeference = gameStateRef->CesiumGeoreference;
	FVector PlayerLocation = Georeference->TransformUnrealPositionToLongitudeLatitudeHeight(gameStateRef->WorldManagerRef->ActiveCamera->GetActorLocation());*/

	for (TPair<FString, FADSBClusterEntry>& Pair : ADSBObjects) {
		if (
			Pair.Value.LastPoint == Pair.Value.NewPoint ||
			Pair.Value.ElapsedLerpTime >= ADSBTimerTickTime
			/*FVector::Distance(Pair.Value.NewPoint, PlayerLocation) > 1*/
		) {
			continue;
		}

		Pair.Value.ElapsedLerpTime += DeltaTime;
		FVector NewLonLatHeight = FMath::Lerp(Pair.Value.LastPoint, Pair.Value.NewPoint, Pair.Value.ElapsedLerpTime / ADSBTimerTickTime);

		Pair.Value.ADSBObject->MoveToLongitudeLatitudeHeight(NewLonLatHeight);
	}
}

bool USPObstacleManager::IsTickable() const {
	return CanTick;
}

bool USPObstacleManager::IsTickableInEditor() const {
	return false;
}
bool USPObstacleManager::IsTickableWhenPaused() const {
	return false;
}

TStatId USPObstacleManager::GetStatId() const {
	return TStatId();
}

void USPObstacleManager::CullRelatedObjects(float MaximumDrawDistance, const FVector& OriginLocationUE, bool FromFlyTo) {
	for (const TPair<FString, FFAAClusterEntry>& Pair : FAAObjects) {
		FVector LocationToCheck = Pair.Value.FAAObstacle->GetActorLocation();

		FVector DistanceVector = OriginLocationUE - LocationToCheck;
		bool IsCulled = DistanceVector.Size() > MaximumDrawDistance;
		IInteractionBoxProvider::Execute_ToggleCull(Pair.Value.FAAObstacle, IsCulled, FromFlyTo);
	}

	for (const TPair<FString, FADSBClusterEntry>& Pair : ADSBObjects) {
		FVector LocationToCheck = Pair.Value.ADSBObject->GetActorLocation();

		FVector DistanceVector = OriginLocationUE - LocationToCheck;
		bool IsCulled = DistanceVector.Size() > MaximumDrawDistance;
		IInteractionBoxProvider::Execute_ToggleCull(Pair.Value.ADSBObject, IsCulled, FromFlyTo);
	}
}
