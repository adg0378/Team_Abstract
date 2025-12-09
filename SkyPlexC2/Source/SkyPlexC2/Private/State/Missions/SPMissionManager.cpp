#include "State/Missions/SPMissionManager.h"
#include "SPUtility.h"
#include "State/SPGameState.h"
#include "State/Storage/SPStorageManager.h"
#include "State/Storage/MissionAssetsManager.h"
#include "State/SPWorldManager.h"
#include "Objects/Interests/AreaOfInterest.h"
#include "Objects/Interests/SPSimWorldOrigin.h"
#include "Objects/Interests/POI.h"
#include "SPDroneManager.h"
#include "Objects/Interests/SPGeoFence.h"
#include "Objects/Geometry/PlaceablePoint.h"
#include "Objects/Interests/SPTakeoffPoint.h"
#include "Objects/Geometry/Polygon.h"
#include "Objects/Drones/CCSimDrone.h"
#include "State/Obstacles/SPObstacleManager.h"
#include "Util/CCSimWebsocketMessenger.h"
#include "Util/MissionSchemaFormatter.h"
#include "Objects/Geometry/InterestLines.h"
#include <CesiumGlobeAnchorComponent.h>


void USPMissionManager::Setup_Implementation(bool& outSuccess) {
	Super::Setup_Implementation(outSuccess);
	LoadMissions();
	LoadInterests();
	SpawnSimWorldOrigin();
	StartMissionProgressTimer();
}

void USPMissionManager::Teardown_Implementation() {
	Super::Teardown_Implementation();
}

void USPMissionManager::PreTeardown() {
	StopMissionProgressTimer();
}

void USPMissionManager::SpawnSimWorldOrigin() {
	FHiddenConfigPreferencesStruct Prefs = PreferencesManagerRef->GetPreferencesRef().ConfigPreferences;
	ASPSimWorldOrigin* Origin;
	gameStateRef->GenericSpawnActor<ASPSimWorldOrigin>(SimOriginToSpawn, FVector::ZeroVector, Origin);
	SimWorldOrigin = Origin;
	
	TArray<FVector> PosToSample{ FVector(Prefs.SimOriginLon, Prefs.SimOriginLat, 0.0) };
	gameStateRef->SampleHeights(PosToSample, [&](const TArray<FVector>& SampledPositions) {
		if (SampledPositions.Num() > 0) {
			FVector Pos = SampledPositions[0];
			if (Pos.Z == -9999.0) {
				UE_LOG(LogTemp, Warning, TEXT("Error sampling height while placing world sim origin"));
			}
			else {
				UE_LOG(LogTemp, Log, TEXT("Spawning simulation world origin at %lf, %lf, %lf"), Pos.X, Pos.Y, Pos.Z);
				SimWorldOrigin->MoveToLongitudeLatitudeHeight(Pos);
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Got no height samples while placing world sim origin"));
		}
	});
}

FVector USPMissionManager::GetWorldOriginPos() {
	FVector Pos;
	if (SimWorldOrigin) {
		SimWorldOrigin->GetLongitudeLatitudeHeight(Pos);
	}
	return Pos;
}

void USPMissionManager::LoadMissions() {
	gameStateRef->storageManagerRef->missionAssetsManagerRef->LoadMissions(Missions);

	for (TPair<int32, FSPMissionStruct>& Pair : Missions) {
		Interests.Add(Pair.Key, FSPInterestStruct(Pair.Key));
	}
}

void USPMissionManager::LoadInterests() {
	TArray<FPOIDataStruct> POIData;
	TArray<FAOIDataStruct> AOIData;
	TArray<FGeoFenceDataStruct> GeoFenceData;
	TArray<FTakeoffPointDataStruct> TakeoffPointData;
	gameStateRef->storageManagerRef->missionAssetsManagerRef->LoadInterests(POIData, AOIData, GeoFenceData, TakeoffPointData);

	if (!POIPointToSpawn || !AOIPointToSpawn || !AOIPolygonToSpawn || !GeoFencePointToSpawn || !GeoFencePointToSpawn) {
		UE_LOG(LogTemp, Warning, TEXT("Must define POIPointToSpawn, AOIPointToSpawn, AOIPolygonToSpawn, etc. for InterestManager"))
		return;
	}

	for (FPOIDataStruct Data : POIData) {
		UPOI* POI = NewObject<UPOI>(this);
		APlaceablePoint* Point;
		gameStateRef->GenericSpawnActor<APlaceablePoint>(POIPointToSpawn, FVector::ZeroVector, Point);
		Point->MoveToLongitudeLatitudeHeight(Data.longLatHeight);
		Point->CustomOnPlaced();
		POI->SetPoint(Point);
		POI->SetAttributes(Data.id, Data.groupID, Data.name);
		POI->SetSerializedParams(Data.Params);
		FSPInterestStruct* GroupInterests = Interests.Find(Data.groupID);
		if (GroupInterests) {
			GroupInterests->Interests.Add(Data.id, POI);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Unable to find group interests for id: %i"), Data.id);
		}
	}

	for (FTakeoffPointDataStruct Data : TakeoffPointData) {
		USPTakeoffPoint* TakeoffPoint = NewObject<USPTakeoffPoint>(this);
		APlaceablePoint* Point;
		gameStateRef->GenericSpawnActor<APlaceablePoint>(TakeoffPointToSpawn, FVector::ZeroVector, Point);
		Point->MoveToLongitudeLatitudeHeight(Data.LonLatHeight);
		Point->CustomOnPlaced();
		TakeoffPoint->SetPoint(Point);
		TakeoffPoint->SetAttributes(Data.ID, Data.GroupID, Data.Name);
		TakeoffPoint->SetSerializedParams(Data.Params);
		FSPInterestStruct* GroupInterests = Interests.Find(Data.GroupID);
		if (GroupInterests) {
			GroupInterests->TakeoffPoint = TakeoffPoint;
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Unable to find group interests for id: %i"), Data.GroupID);
		}
	}

	for (FAOIDataStruct Data : AOIData) {
		UAreaOfInterest* AOI = NewObject<UAreaOfInterest>(this);
		APolygon* Polygon;
		gameStateRef->GenericSpawnActor<APolygon>(AOIPolygonToSpawn, FVector::ZeroVector, Polygon);
		for (const FVector& LonLatHeight : Data.longLatHeights) {
			APlaceablePoint* Point;
			gameStateRef->GenericSpawnActor<APlaceablePoint>(AOIPointToSpawn, FVector::ZeroVector, Point);
			Point->MoveToLongitudeLatitudeHeight(LonLatHeight);
			Point->CustomOnPlaced();
			Polygon->AddPoint(Point);
		}
		Polygon->Close();
		AOI->SetPolygon(Polygon);
		AOI->SetAttributes(Data.id, Data.groupID, Data.name);
		AOI->SetSerializedParams(Data.Params);
		FSPInterestStruct* GroupInterests = Interests.Find(Data.groupID);
		if (GroupInterests) {
			GroupInterests->Interests.Add(Data.id, AOI);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Unable to find group interests for id: %i"), Data.groupID);
		}
	}

	for (FGeoFenceDataStruct Data : GeoFenceData) {
		USPGeoFence* GeoFence = NewObject<USPGeoFence>(this);
		APolygon* Polygon;
		gameStateRef->GenericSpawnActor<APolygon>(GeoFencePolygonToSpawn, FVector::ZeroVector, Polygon);
		for (const FVector& LonLatHeight : Data.LonLatHeights) {
			APlaceablePoint* Point;
			gameStateRef->GenericSpawnActor<APlaceablePoint>(GeoFencePointToSpawn, FVector::ZeroVector, Point);
			Point->MoveToLongitudeLatitudeHeight(LonLatHeight);
			Point->CustomOnPlaced();
			Polygon->AddPoint(Point);
		}
		Polygon->Close();
		GeoFence->SetPolygon(Polygon);
		GeoFence->SetAttributes(Data.ID, Data.GroupID, Data.Name);
		GeoFence->SetSerializedParams(Data.Params);
		FSPInterestStruct* GroupInterests = Interests.Find(Data.GroupID);
		if (GroupInterests) {
			GroupInterests->GeoFences.Add(Data.ID, GeoFence);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Unable to find group interests for id: %i"), Data.GroupID);
		}
	}

	TArray<int32> missionIDs;
	Missions.GetKeys(missionIDs);
	
	for (int32 key : missionIDs) {
		SpawnMissionLine(key);
	}
}

void USPMissionManager::AddMission(FString name, int32& OutMissionID) {
	int32 MissionID = -1;
	gameStateRef->storageManagerRef->missionAssetsManagerRef->InsertMission(name, MissionID);

	Missions.Add(MissionID, FSPMissionStruct{ .ID = MissionID, .Name = name });
	Interests.Add(MissionID);
	OutMissionID = MissionID;
	OnMissionAdded.Broadcast(MissionID);
	LOG->Info(FString::Format(*FString("Added mission: '{0}'"), { *name }), LOG_ORIGIN);
}

void USPMissionManager::DeleteMission(int32 InMissionID, int32 DumpMissionID, bool DestroyInterests) {
	DespawnMissionLine(InMissionID);
	gameStateRef->storageManagerRef->missionAssetsManagerRef->DeleteMission(InMissionID);

	const FSPInterestStruct* GroupInterests = Interests.Find(InMissionID);
	if (GroupInterests == nullptr) {
		LOG->Warn(FString::Printf(TEXT("Found no interest references for interest group '%i'"), InMissionID), LOG_ORIGIN);
		return;
	}

	if (DestroyInterests) {
		TArray<UInterest*> InterestsToDestroy;
		for (const TPair<int32, UInterest*>& Pair : GroupInterests->Interests) {
			InterestsToDestroy.Add(Pair.Value);
		}

		for (const TPair<int32, USPGeoFence*>& Pair : GroupInterests->GeoFences) {
			InterestsToDestroy.Add(Pair.Value);
		}

		if (GroupInterests->TakeoffPoint) {
			InterestsToDestroy.Add(GroupInterests->TakeoffPoint);
		}

		for (UInterest*& Interest : InterestsToDestroy) {
			IInteractionBoxProvider::Execute_DestroySelf(Interest);
		}
	}
	else if (Missions.Contains(DumpMissionID)) {
		FSPInterestStruct* DumpGroupInterests = Interests.Find(DumpMissionID);
		
		for (const TPair<int32, UInterest*>& Pair : GroupInterests->Interests) {
			int32 ID = Pair.Value->GetID();
			Pair.Value->SetGroupID(DumpMissionID);
			gameStateRef->storageManagerRef->missionAssetsManagerRef->ReassignInterest(ID, DumpMissionID);
			DumpGroupInterests->Interests.Add(Pair);
		}

		for (const TPair<int32, USPGeoFence*>& Pair : GroupInterests->GeoFences) {
			int32 ID = Pair.Value->GetID();
			Pair.Value->SetGroupID(DumpMissionID);
			gameStateRef->storageManagerRef->missionAssetsManagerRef->ReassignInterest(ID, DumpMissionID);
			DumpGroupInterests->GeoFences.Add(Pair);
		}

		if (!DumpGroupInterests->TakeoffPoint) {
			DumpGroupInterests->TakeoffPoint = GroupInterests->TakeoffPoint;
		}
		else if (GroupInterests->TakeoffPoint) {
			IInteractionBoxProvider::Execute_DestroySelf(GroupInterests->TakeoffPoint);
		}

		RefreshMissionLine(DumpMissionID);
		OnMissionUpdated.Broadcast(DumpMissionID);
	}
	Missions.Remove(InMissionID);
	Interests.Remove(InMissionID);
	OnMissionDeleted.Broadcast(InMissionID);
}

void USPMissionManager::RenameMission(int32 MissionID, FString NewName) {
	if (Missions.Contains(MissionID)) {
		Missions[MissionID].Name = NewName;
		gameStateRef->storageManagerRef->missionAssetsManagerRef->RenameMission(MissionID, NewName);

		gameStateRef->RefreshSelected();

		OnMissionUpdated.Broadcast(MissionID);
	}
}

void USPMissionManager::UpdateMissionInterestOrder(int32 MissionID, TArray<int32>& InterestIDs, bool Broadcast) {
	if (Missions.Contains(MissionID)) {
		UpdateMissionOrderNoBroadcast(MissionID, InterestIDs);

		if (Broadcast) {
			OnMissionUpdated.Broadcast(MissionID);
		}
	}
}

void USPMissionManager::UpdateMissionOrderNoBroadcast(int32 MissionID, TArray<int32>& InterestIDs) {
	Missions[MissionID].InterestOrder = InterestIDs;
	gameStateRef->storageManagerRef->missionAssetsManagerRef->UpdateMissionOrder(MissionID, InterestIDs);

	RefreshMissionLine(MissionID);
}

void USPMissionManager::GetMissionNames(TArray<FString>& outKeys) {
	outKeys.Empty();

	for (const TPair<int32, FSPMissionStruct>& Pair : Missions) {
		outKeys.Add(Pair.Value.Name);
	}
}

const TMap<int32, FSPInterestStruct>& USPMissionManager::GetInterests() const {
	return Interests;
}

void USPMissionManager::GetAllInterestsInOrder(int32 MissionID, TArray<UInterest*>& OutInterests) const {
	OutInterests.Empty();

	const FSPInterestStruct* MissionInterests = Interests.Find(MissionID);
	if (MissionInterests) {

		if (MissionInterests->TakeoffPoint) {
			OutInterests.Add(MissionInterests->TakeoffPoint);
		}

		for (const int32& ID : Missions[MissionID].InterestOrder) {
			//LOG->Info(FString::Printf(TEXT("curr ID: %i"), ID));
			if (MissionInterests->Interests.Contains(ID)) {
				UInterest* Interest = MissionInterests->Interests.Find(ID)->Get();
				if (Interest) {
					OutInterests.Add(Interest);
				}
			}
		}

		for (const TPair<int32, USPGeoFence*> Pair : MissionInterests->GeoFences) {
			OutInterests.Add(Pair.Value);
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Returned no interests from GetAllInterestsInOrder: Mission does not exist"));
	}
}

void USPMissionManager::GetInterestsInOrder(int32 MissionID, TArray<UInterest*>& OutInterests) const {
	OutInterests.Empty();

	const FSPInterestStruct* MissionInterests = Interests.Find(MissionID);
	if (MissionInterests) {
		for (const int32& ID : Missions[MissionID].InterestOrder) {
			//LOG->Info(FString::Printf(TEXT("curr ID: %i"), ID));
			if (MissionInterests->Interests.Contains(ID)) {
				UInterest* Interest = MissionInterests->Interests.Find(ID)->Get();
				if (Interest) {
					OutInterests.Add(Interest);
				}
			}
		}
	}
}

void USPMissionManager::SetCurrentPlannedMission(int32 GroupID) {
	if (Missions.Contains(GroupID))
	{
		CurrentPlannedMissionID = GroupID;
		UE_LOG(LogTemp, Log, TEXT("Current Planned Mission set to ID: %d"), CurrentPlannedMissionID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to set CurrentPlannedMissionID to %d"), GroupID);
	}
}

void USPMissionManager::AddInterest(UInterest* Interest, int32 GroupID) {
	if (GroupID == -1) {
		GroupID = CurrentPlannedMissionID;
	}
	else if (!Missions.Contains(GroupID)) {
		UE_LOG(LogTemp, Warning, TEXT("Mission does not exist"))
		return;
	}

	Interest->SetGroupID(GroupID);

	FSPInterestStruct* GroupInterests = Interests.Find(GroupID);
	int32 OutID = -1;
	FString Name;
	Interest->GetName(Name);

	EInterestType InterestType = Interest->GetInterestType();
	FString SerializedParams = Interest->GetSerializedParams();

	if (InterestType == EInterestType::POI) {
		UPOI* POI = Cast<UPOI>(Interest);
		APlaceablePoint* Point;
		POI->GetPoint(Point);
		FVector LonLatHeight;
		Point->GetLongitudeLatitudeHeight(LonLatHeight);
		gameStateRef->storageManagerRef->missionAssetsManagerRef->InsertPOI(
			Name,
			LonLatHeight,
			GroupID,
			SerializedParams,
			OutID
		);
		GroupInterests->Interests.Add(OutID, Interest);
	}
	else if (InterestType == EInterestType::Takeoff) {
		if (GroupInterests->TakeoffPoint) {
			LOG->Error(FString("Cannot place more than one takeoff point per mission"), FString("MissionManager"), true);
			IInteractionBoxProvider::Execute_DestroySelf(Interest);
			return;
		}

		USPTakeoffPoint* TakeoffPoint = Cast<USPTakeoffPoint>(Interest);
		APlaceablePoint* Point;
		TakeoffPoint->GetPoint(Point);
		FVector LonLatHeight;
		Point->GetLongitudeLatitudeHeight(LonLatHeight);
		gameStateRef->storageManagerRef->missionAssetsManagerRef->InsertTakeoffPoint(
			Name,
			LonLatHeight,
			GroupID,
			SerializedParams,
			OutID
		);
		GroupInterests->TakeoffPoint = TakeoffPoint;
	}
	else if (InterestType == EInterestType::AOI) {
		UAreaOfInterest* AOI = Cast<UAreaOfInterest>(Interest);
		APolygon* Polygon;
		AOI->GetPolygon(Polygon);
		TArray<FVector> PointLocations;
		Polygon->GetPointLocations(PointLocations);
		gameStateRef->storageManagerRef->missionAssetsManagerRef->InsertAOI(
			Name,
			PointLocations,
			GroupID,
			SerializedParams,
			OutID
		);
		GroupInterests->Interests.Add(OutID, Interest);
	}
	else if (InterestType == EInterestType::GeoFence) {
		USPGeoFence* GeoFence = Cast<USPGeoFence>(Interest);
		APolygon* Polygon;
		GeoFence->GetPolygon(Polygon);
		TArray<FVector> PointLocations;
		Polygon->GetPointLocations(PointLocations);
		gameStateRef->storageManagerRef->missionAssetsManagerRef->InsertGeoFence(
			Name,
			PointLocations,
			GroupID,
			SerializedParams,
			OutID
		);
		GroupInterests->GeoFences.Add(OutID, GeoFence);
	}

	if (OutID == -1) {
		LOG->Error(FString("Unable to store Interest in database"), LOG_ORIGIN);
	}
	Interest->SetID(OutID);
	RenameInterestNoBroadcast(OutID, GroupID, FString::FromInt(OutID));
	
	if (InterestType == EInterestType::AOI || InterestType == EInterestType::POI) {
		Missions[GroupID].InterestOrder.Add(OutID);
		gameStateRef->storageManagerRef->missionAssetsManagerRef->UpdateMissionOrder(GroupID, Missions[GroupID].InterestOrder);
	}
	
	LOG->Info(FString::Format(*FString("Succesfully plotted Interest with ID: '{0}'"), { OutID }));

	OnInterestAdded.Broadcast(OutID, GroupID);

	gameStateRef->obstacleManagerRef->HandleNewInterest(Interest);
	RefreshMissionLine(GroupID);
}

void USPMissionManager::RemoveInterest(int32 InterestID, int32 GroupID) {
	FSPInterestStruct* GroupInterests = Interests.Find(GroupID);
	if (!GroupInterests) {
		LOG->Error(TEXT("Unable to remove interest: group does not exist"), LOG_ORIGIN);
		return;
	}

	UInterest* Interest = GetInterestFromInterestsStruct(InterestID, *GroupInterests);
	if (!Interest) {
		LOG->Error(TEXT("Unable to remove interest: interest does not exist"), LOG_ORIGIN);
		return;
	}

	EInterestType Type = Interest->GetInterestType();

	if (Type == EInterestType::GeoFence) {
		GroupInterests->GeoFences.Remove(InterestID);
	}
	else if (Type == EInterestType::Takeoff) {
		GroupInterests->TakeoffPoint = nullptr;
	}
	else {
		GroupInterests->Interests.Remove(InterestID);
		Missions[GroupID].InterestOrder.Remove(InterestID);
		gameStateRef->storageManagerRef->missionAssetsManagerRef->UpdateMissionOrder(GroupID, Missions[GroupID].InterestOrder);
	}

	gameStateRef->obstacleManagerRef->HandleDeleteInterest(Interest);
	gameStateRef->storageManagerRef->missionAssetsManagerRef->DeleteInterest(InterestID);

	OnInterestDeleted.Broadcast(InterestID, GroupID);

	RefreshMissionLine(GroupID);
}

void USPMissionManager::ReassignInterest(int32 InterestID, int32 CurrGroupID, int32 NewGroupID) {
	FSPInterestStruct* OldGroupInterests = Interests.Find(CurrGroupID);
	if (!OldGroupInterests) {
		LOG->Error(TEXT("Unable to reassign interest: group does not exist"), LOG_ORIGIN);
		return;
	}

	FSPInterestStruct* NewGroupInterests = Interests.Find(NewGroupID);
	if (!NewGroupInterests) {
		LOG->Error(TEXT("Unable to reassign interest: new group does not exist"), LOG_ORIGIN);
		return;
	}

	UInterest* Interest = GetInterestFromInterestsStruct(InterestID, *OldGroupInterests);
	if (!Interest) {
		LOG->Error(TEXT("Unable to reassign interest: interest does not exist"), LOG_ORIGIN);
		return;
	}

	Interest->SetGroupID(NewGroupID);
	EInterestType Type = Interest->GetInterestType();
	gameStateRef->storageManagerRef->missionAssetsManagerRef->ReassignInterest(InterestID, NewGroupID);

	gameStateRef->RefreshSelected();

	if (Type == EInterestType::GeoFence) {
		OldGroupInterests->GeoFences.Remove(InterestID);
		NewGroupInterests->GeoFences.Add(InterestID, Cast<USPGeoFence>(Interest));
	}
	else if (Type == EInterestType::Takeoff) {
		if (NewGroupInterests->TakeoffPoint) {
			IInteractionBoxProvider::Execute_DestroySelf(NewGroupInterests->TakeoffPoint);
		}
		NewGroupInterests->TakeoffPoint = Cast<USPTakeoffPoint>(Interest);
	}
	else {
		OldGroupInterests->Interests.Remove(InterestID);
		NewGroupInterests->Interests.Add(InterestID, Interest);

		Missions[CurrGroupID].InterestOrder.Remove(InterestID);
		Missions[NewGroupID].InterestOrder.Add(InterestID);

		gameStateRef->storageManagerRef->missionAssetsManagerRef->UpdateMissionOrder(CurrGroupID, Missions[CurrGroupID].InterestOrder);
		gameStateRef->storageManagerRef->missionAssetsManagerRef->UpdateMissionOrder(NewGroupID, Missions[NewGroupID].InterestOrder);
	}

	OnMissionUpdated.Broadcast(CurrGroupID);
	OnMissionUpdated.Broadcast(NewGroupID);

	RefreshMissionLine(CurrGroupID);
	RefreshMissionLine(NewGroupID);
}

void USPMissionManager::ReassignInterests(const TMap<int32, int32>& IDToGroupMap, int32 NewGroupID) {
	FSPInterestStruct* NewGroupInterests = Interests.Find(NewGroupID);
	if (!NewGroupInterests) {
		LOG->Error(TEXT("Unable to reassign interests: new group does not exist"), LOG_ORIGIN);
		return;
	}

	TSet<int32> GroupsToUpdate = { NewGroupID };

	for (const TPair<int32, int32> IDToGroupPair : IDToGroupMap) {
		FSPInterestStruct* OldGroupInterests = Interests.Find(IDToGroupPair.Value);
		if (!OldGroupInterests) {
			LOG->Error(TEXT("Unable to reassign interest: group does not exist"), LOG_ORIGIN);
			continue;
		}

		int32 ID = IDToGroupPair.Key;

		UInterest* Interest = GetInterestFromInterestsStruct(ID, *OldGroupInterests);
		if (!Interest) {
			LOG->Error(TEXT("Unable to reassign interest: interest does not exist"), LOG_ORIGIN);
			continue;
		}

		Interest->SetGroupID(NewGroupID);
		gameStateRef->storageManagerRef->missionAssetsManagerRef->ReassignInterest(ID, NewGroupID);

		EInterestType Type = Interest->GetInterestType();
		if (Type == EInterestType::GeoFence) {
			OldGroupInterests->GeoFences.Remove(ID);
			NewGroupInterests->GeoFences.Add(ID, Cast<USPGeoFence>(Interest));
		}
		else if (Type == EInterestType::Takeoff) {
			if (NewGroupInterests->TakeoffPoint) {
				IInteractionBoxProvider::Execute_DestroySelf(NewGroupInterests->TakeoffPoint);
			}
			NewGroupInterests->TakeoffPoint = Cast<USPTakeoffPoint>(Interest);
		}
		else {
			OldGroupInterests->Interests.Remove(ID);
			NewGroupInterests->Interests.Add(ID, Interest);

			Missions[IDToGroupPair.Value].InterestOrder.Remove(ID);
			Missions[NewGroupID].InterestOrder.Add(ID);
		}

		OldGroupInterests->Interests.Remove(IDToGroupPair.Key);
		NewGroupInterests->Interests.Add(IDToGroupPair.Key, Interest);
		GroupsToUpdate.Add(IDToGroupPair.Value);
	}

	gameStateRef->RefreshSelected();

	for (int32& ID : GroupsToUpdate) {
		gameStateRef->storageManagerRef->missionAssetsManagerRef->UpdateMissionOrder(ID, Missions[ID].InterestOrder);
		RefreshMissionLine(ID);
		OnMissionUpdated.Broadcast(ID);
	}

	RefreshMissionLine(NewGroupID);
}

void USPMissionManager::RenameInterest(int32 InterestID, int32 GroupID, FString NewName) {
	RenameInterestNoBroadcast(InterestID, GroupID, NewName);

	gameStateRef->RefreshSelected();
	OnInterestUpdated.Broadcast(InterestID, GroupID);
}

void USPMissionManager::RenameInterestNoBroadcast(int32 InterestID, int32 GroupID, FString NewName) {
	FSPInterestStruct* GroupInterests = Interests.Find(GroupID);
	if (!GroupInterests) {
		LOG->Error(TEXT("Unable to rename interest: group does not exist"), LOG_ORIGIN);
		return;
	}

	UInterest* Interest = GetInterestFromInterestsStruct(InterestID, *GroupInterests);
	if (!Interest) {
		LOG->Error(TEXT("Unable to rename interest: interest does not exist"), LOG_ORIGIN);
		return;
	}

	Interest->SetName(NewName);
	gameStateRef->storageManagerRef->missionAssetsManagerRef->RenameInterest(InterestID, NewName);
}

void USPMissionManager::UpdateInterestParams(int32 InterestID, int32 MissionID) {
	FSPInterestStruct* GroupInterests = Interests.Find(MissionID);
	if (!GroupInterests) {
		LOG->Error(TEXT("Unable to update interest params: mission does not exist"), LOG_ORIGIN);
		return;
	}

	UInterest* Interest = GetInterestFromInterestsStruct(InterestID, *GroupInterests);
	if (!Interest) {
		LOG->Error(TEXT("Unable to update interest params: interest does not exist"), LOG_ORIGIN);
		return;
	}

	FString Params = Interest->GetSerializedParams();
	gameStateRef->storageManagerRef->missionAssetsManagerRef->UpdateInterestParams(InterestID, Params);
}

void USPMissionManager::MoveInterest(int32 InterestID, int32 GroupID) {
	FSPInterestStruct* GroupInterests = Interests.Find(GroupID);
	if (!GroupInterests) {
		LOG->Error(TEXT("Unable to move interest: group does not exist"), LOG_ORIGIN);
		return;
	}

	UInterest* Interest = GetInterestFromInterestsStruct(InterestID, *GroupInterests);
	if (!Interest) {
		LOG->Error(TEXT("Unable to move interest: interest does not exist"), LOG_ORIGIN);
		return;
	}

	EInterestType InterestType = Interest->GetInterestType();
	if (InterestType == EInterestType::POI) {
		UPOI* POI = Cast<UPOI>(Interest);
		APlaceablePoint* Point;
		POI->GetPoint(Point);
		FVector LonLatHeight;
		Point->GetLongitudeLatitudeHeight(LonLatHeight);
		gameStateRef->storageManagerRef->missionAssetsManagerRef->MovePOI(InterestID, LonLatHeight);
	}
	else if (InterestType == EInterestType::Takeoff) {
		USPTakeoffPoint* Takeoff = Cast<USPTakeoffPoint>(Interest);
		APlaceablePoint* Point;
		Takeoff->GetPoint(Point);
		FVector LonLatHeight;
		Point->GetLongitudeLatitudeHeight(LonLatHeight);
		gameStateRef->storageManagerRef->missionAssetsManagerRef->MoveTakeoffPoint(InterestID, LonLatHeight);
	}
	else if (InterestType == EInterestType::AOI) {
		UAreaOfInterest* AOI = Cast<UAreaOfInterest>(Interest);
		APolygon* Polygon;
		AOI->GetPolygon(Polygon);
		TArray<FVector> PointLocations;
		Polygon->GetPointLocations(PointLocations);
		gameStateRef->storageManagerRef->missionAssetsManagerRef->MoveAOI(InterestID, PointLocations);
	}
	else if (InterestType == EInterestType::GeoFence) {
		USPGeoFence* GeoFence = Cast<USPGeoFence>(Interest);
		APolygon* Polygon;
		GeoFence->GetPolygon(Polygon);
		TArray<FVector> PointLocations;
		Polygon->GetPointLocations(PointLocations);
		gameStateRef->storageManagerRef->missionAssetsManagerRef->MoveGeoFence(InterestID, PointLocations);
	}
	gameStateRef->obstacleManagerRef->HandleMoveInterest(Interest);

	RefreshMissionLine(GroupID);
}

FString USPMissionManager::GetMissionName(int GroupID) const {
	const FSPMissionStruct* Value = Missions.Find(GroupID);
	return Value == nullptr ? "Unknown" : Value->Name;
}

void USPMissionManager::ValidateMissionName(FString InName, bool& OutIsValid) {
	OutIsValid = true;
}

const TMap<int32, FSPMissionStruct>& USPMissionManager::GetMissions() const {
	return Missions;
}

UInterest* USPMissionManager::GetInterest(int32 InterestID, int32 GroupID) const {
	const FSPInterestStruct* GroupInterests = Interests.Find(GroupID);
	if (!GroupInterests) {
		LOG->Error(TEXT("Unable to get interest: group does not exist"), LOG_ORIGIN);
		return nullptr;
	}

	UInterest* Interest = GetInterestFromInterestsStruct(InterestID, *GroupInterests);
	if (!Interest) {
		LOG->Error(TEXT("Unable to get interest: interest does not exist"), LOG_ORIGIN);
		return nullptr;
	}
	return Interest;
}

UInterest* USPMissionManager::GetInterestFromInterestsStruct(int32 InterestID, const FSPInterestStruct& InInterests) const {
	if (InInterests.TakeoffPoint && InInterests.TakeoffPoint->GetID() == InterestID) {
		UE_LOG(LogTemp, Warning, TEXT("Found takeoff point interest"));
		return InInterests.TakeoffPoint;
	}

	if (InInterests.Interests.Contains(InterestID)) {
		return InInterests.Interests.Find(InterestID)->Get();
	}

	if (InInterests.GeoFences.Contains(InterestID)) {
		return InInterests.GeoFences.Find(InterestID)->Get();
	}
	return nullptr;
}

void USPMissionManager::CullRelatedObjects(float MaximumDrawDistance, const FVector& OriginLocationUE, bool FromFlyTo) {
	for (const TPair<int32, FSPInterestStruct>& Pair : Interests) {
		for (const TPair<int32, USPGeoFence*>& GeoFencePair : Pair.Value.GeoFences) {
			APolygon* Polygon;
			USPGeoFence* GeoFence = Cast<USPGeoFence>(GeoFencePair.Value);
			GeoFence->GetPolygon(Polygon);
			FVector LocationToCheck = Polygon->GetCenterpointUE();

			FVector DistanceVector = OriginLocationUE - LocationToCheck;
			bool IsCulled = DistanceVector.Size() > MaximumDrawDistance;
			IInteractionBoxProvider::Execute_ToggleCull(GeoFencePair.Value, IsCulled, FromFlyTo);
		}

		for (const TPair<int32, UInterest*>& InterestPair : Pair.Value.Interests) {
			EInterestType InterestType = InterestPair.Value->GetInterestType();
			FVector LocationToCheck;

			if (InterestType == EInterestType::POI) {
				APlaceablePoint* Point;
				UPOI* POI = Cast<UPOI>(InterestPair.Value);
				POI->GetPoint(Point);
				LocationToCheck = Point->GetActorLocation();
			}
			else if (InterestType == EInterestType::AOI) {
				APolygon* Polygon;
				UAreaOfInterest* AOI = Cast<UAreaOfInterest>(InterestPair.Value);
				AOI->GetPolygon(Polygon);
				LocationToCheck = Polygon->GetCenterpointUE();
			}

			FVector DistanceVector = OriginLocationUE - LocationToCheck;
			bool IsCulled = DistanceVector.Size() > MaximumDrawDistance;
			IInteractionBoxProvider::Execute_ToggleCull(InterestPair.Value, IsCulled, FromFlyTo);
		}

		if (Pair.Value.TakeoffPoint) {
			APlaceablePoint* Point;
			Pair.Value.TakeoffPoint->GetPoint(Point);
			FVector LocationToCheck = Point->GetActorLocation();

			FVector DistanceVector = OriginLocationUE - LocationToCheck;
			bool IsCulled = DistanceVector.Size() > MaximumDrawDistance;
			IInteractionBoxProvider::Execute_ToggleCull(Pair.Value.TakeoffPoint, IsCulled, FromFlyTo);
		}
		
		if (SimWorldOrigin) {
			FVector LocationToCheck = SimWorldOrigin->GetActorLocation();
			FVector DistanceVector = OriginLocationUE - LocationToCheck;
			bool IsCulled = DistanceVector.Size() > MaximumDrawDistance;
			IInteractionBoxProvider::Execute_ToggleCull(SimWorldOrigin, IsCulled, FromFlyTo);
		}
	}
}

void USPMissionManager::StopMission(int32 MissionID) {
	FSPMissionStruct* Mission = Missions.Find(MissionID);
	if (!Mission) {
		if (MissionID == -1) {
			for (const TPair<int32, FSPMissionStruct> Pair : Missions) {
				Mission = Missions.Find(Pair.Key);
				MissionID = Pair.Key;
				break;
			}
			for (const TPair<int32, FString>& Pair : gameStateRef->droneManagerRef->GetSwarms()) {
				Mission->AssignedSwarmIDs.Add(Pair.Key);
				break;
			}
		}
		else {
			LOG->Error(TEXT("Unable to end mission: mission does not exist"), LOG_ORIGIN);
			return;
		}
	}

	if (Mission->AssignedSwarmIDs.IsEmpty()) {
		LOG->Warn(TEXT("Unable to end mission: no swarms assigned"), LOG_ORIGIN);
		return;
	}

	if (!Interests[MissionID].TakeoffPoint) {
		LOG->Error(TEXT("Unable to end mission: no takeoff point"), LOG_ORIGIN, true);
		return;
	}

	FString MissionMsg = UCCSimWebsocketMessenger::MakeMissionUploadMessage(EMissionUploadAction::START, FormatMissionAsJson(MissionID));

	TArray<const FSPSwarmStruct*> Drones = gameStateRef->droneManagerRef->GetSwarmDrones(Mission->AssignedSwarmIDs);
	for (const FSPSwarmStruct* Swarm : Drones) {
		for (const TPair<int32, ABasicDrone*>& Pair : Swarm->Drones) {
			ACCSimDrone* SimDrone = Cast<ACCSimDrone>(Pair.Value);
			if (SimDrone) {
				SimDrone->Return();
			}
		}
	}
}

void USPMissionManager::StartMission(int32 MissionID) {
	FSPMissionStruct* Mission = Missions.Find(MissionID);
	if (!Mission) {
		if (MissionID == -1) {
			for (const TPair<int32, FSPMissionStruct> Pair : Missions) {
				Mission = Missions.Find(Pair.Key);
				MissionID = Pair.Key;
				break;
			}
			for (const TPair<int32, FString>& Pair : gameStateRef->droneManagerRef->GetSwarms()) {
				Mission->AssignedSwarmIDs.Add(Pair.Key);
				break;
			}
		}
		else {
			LOG->Error(TEXT("Unable to start mission: mission does not exist"), LOG_ORIGIN);
			return;
		}
	}

	if (Mission->AssignedSwarmIDs.IsEmpty()) {
		LOG->Warn(TEXT("Unable to start mission: no swarms assigned"), LOG_ORIGIN);
		return;
	}

	if (!Interests[MissionID].TakeoffPoint) {
		LOG->Error(TEXT("Unable to start mission: no takeoff point"), LOG_ORIGIN);
		return;
	}

	FString MissionMsg = UCCSimWebsocketMessenger::MakeMissionUploadMessage(EMissionUploadAction::START, FormatMissionAsJson(MissionID));

	TArray<const FSPSwarmStruct*> Drones = gameStateRef->droneManagerRef->GetSwarmDrones(Mission->AssignedSwarmIDs);
	for (const FSPSwarmStruct* Swarm : Drones) {
		for (const TPair<int32, ABasicDrone*>& Pair : Swarm->Drones) {
			ACCSimDrone* SimDrone = Cast<ACCSimDrone>(Pair.Value);
			if (SimDrone) {
				SimDrone->UploadMission(MissionMsg);
			}
		}
	}
}

bool USPMissionManager::ShouldStartMission(int32 MissionID) {
	FSPMissionStruct* Mission = Missions.Find(MissionID);
	if (!Mission) {
		LOG->Error(TEXT("Unable to start mission: mission does not exist"), LOG_ORIGIN, true);
		return false;
	}

	if (Mission->AssignedSwarmIDs.IsEmpty()) {
		LOG->Error(TEXT("Unable to start mission: no swarms assigned"), LOG_ORIGIN, true);
		return false;
	}

	if (!Interests[MissionID].TakeoffPoint) {
		LOG->Error(TEXT("Unable to start mission: no takeoff point"), LOG_ORIGIN, true);
		return false;
	}

	if (gameStateRef->droneManagerRef->GetNumSwarmDrones(Mission->AssignedSwarmIDs) == 0) {
		LOG->Error(TEXT("Unable to start mission: no drones in assigned swarm"), LOG_ORIGIN, true);
		return false;
	}

	return true;
}

void USPMissionManager::AssignSwarm(int32 SwarmID, int32 MissionID) {
	FSPMissionStruct* Mission = Missions.Find(MissionID);
	if (!Mission) {
		LOG->Error(TEXT("Unable to assign swarm: mission does not exist"), LOG_ORIGIN);
		return;
	}

	Mission->AssignedSwarmIDs.Empty();
	Mission->AssignedSwarmIDs.Add(SwarmID);
}

void USPMissionManager::AddAssignSwarm(int32 SwarmID, int32 MissionID) {
	FSPMissionStruct* Mission = Missions.Find(MissionID);
	if (!Mission) {
		LOG->Error(TEXT("Unable to assign swarm: mission does not exist"), LOG_ORIGIN);
		return;
	}

	if (Mission->AssignedSwarmIDs.Find(SwarmID) == INDEX_NONE) {
		Mission->AssignedSwarmIDs.Add(SwarmID);
	}
}

void USPMissionManager::UnassignSwarm(int32 SwarmID, int32 MissionID) {
	FSPMissionStruct* Mission = Missions.Find(MissionID);
	if (!Mission) {
		LOG->Error(TEXT("Unable to unassign swarm: mission does not exist"), LOG_ORIGIN);
		return;
	}

	Mission->AssignedSwarmIDs.Remove(SwarmID);
}

TSharedPtr<FJsonObject> USPMissionManager::FormatMissionAsJson(int32 MissionID) {
	const FSPInterestStruct* MissionInterests = Interests.Find(MissionID);
	if (MissionInterests) {
		return UMissionSchemaFormatter::FormatMissionAsJson(MissionInterests, Missions[MissionID].InterestOrder);
	}
	else {
		TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
		return RootObject;
	}
}

void USPMissionManager::StartMissionProgressTimer() {
	FTimerManagerTimerParameters TimerParams;
	TimerParams.bLoop = TimerParams.bMaxOncePerFrame = true;
	MissionProgressTimerActive = true;
	GetWorld()->GetTimerManager().SetTimer(MissionProgressTickHandle, this, &USPMissionManager::EmitMissionProgress, MissionProgressTimerTickTime, TimerParams);
}

void USPMissionManager::StopMissionProgressTimer() {
	MissionProgressTimerActive = false;
	GetWorld()->GetTimerManager().ClearTimer(MissionProgressTickHandle);
}

void USPMissionManager::EmitMissionProgress() {
	if (!MissionProgressTimerActive) {
		return;
	}

	TMap<int32, float> ProgressMap;

	for (const TPair<int32, FSPMissionStruct>& Mission : Missions) {
		TArray<const FSPSwarmStruct*> Drones = gameStateRef->droneManagerRef->GetSwarmDrones(Mission.Value.AssignedSwarmIDs);

		float ProgressSum = 0.0f;
		int NumDrones = 0;
		for (const FSPSwarmStruct* Swarm : Drones) {
			for (const TPair<int32, ABasicDrone*>& Pair : Swarm->Drones) {
				ProgressSum += Pair.Value->GetStatus().mission_progress;
				NumDrones++;
			}
		}

		float TotalProgress = NumDrones > 0 ? ProgressSum / NumDrones : 0.0f;
		ProgressMap.Add(Mission.Key, TotalProgress);
	}

	OnMissionProgressUpdated.Broadcast(FSPMissionProgress{ .Progresses=ProgressMap });
}

void USPMissionManager::CompleteMission(int32 SwarmID) {
	for (const TPair<int32, FSPMissionStruct>& Mission : Missions) {
		if (Mission.Value.AssignedSwarmIDs.Contains(SwarmID)) {
			LOG->Info("Mission Finished");
			OnMissionFinished.Broadcast(Mission.Key);
			break;
		}
	}
}

void USPMissionManager::SpawnMissionLine(int32 MissionID) {
	if (!Missions.Contains(MissionID)) {
		return;
	}

	TArray<UInterest*> InOrderInterests;
	GetInterestsInOrder(MissionID, InOrderInterests);
	
	if (InOrderInterests.Num() < 1) {
		return;
	}

	TArray<FVector> LonLatHgts;
	TArray<int32> checkedIDs;

	if (Interests[MissionID].TakeoffPoint) {
		APlaceablePoint* Point;
		Interests[MissionID].TakeoffPoint->GetPoint(Point);
		FVector LonLatHeight;
		Point->GetLongitudeLatitudeHeight(LonLatHeight);

		LonLatHgts.Emplace(LonLatHeight);
	}

	for (int32 i = 0; i < InOrderInterests.Num(); i++) {
		EInterestType InterestType = InOrderInterests[i]->GetInterestType();
		int32 id = InOrderInterests[i]->GetID();

		if (!checkedIDs.Contains(id)) {
			//LOG->Info(FString::Printf(TEXT("currInterest ID: %i"), id));
			checkedIDs.Add(id);

			if (InterestType == EInterestType::POI) {
				UPOI* POI = Cast<UPOI>(InOrderInterests[i]);
				APlaceablePoint* Point;
				POI->GetPoint(Point);
				FVector LonLatHeight;
				Point->GetLongitudeLatitudeHeight(LonLatHeight);

				LonLatHgts.Emplace(LonLatHeight);
			}

			if (InterestType == EInterestType::AOI) {
				UAreaOfInterest* AOI = Cast<UAreaOfInterest>(InOrderInterests[i]);
				TArray<FVector> polygonLonLatHgts;
				polygonLonLatHgts = AOI->GeneratePolygonTransectPoints();

				LonLatHgts.Append(polygonLonLatHgts);
			}
		}
	}
	
	AInterestLines* newMissionLine;
	gameStateRef->GenericSpawnActor<AInterestLines>(InterestLineToSpawn, FVector::ZeroVector, newMissionLine);
	newMissionLine->DrawLinesViaLonLatHgts(LonLatHgts, "mission");
	Missions[MissionID].MissionLine = newMissionLine;
}

void USPMissionManager::DespawnMissionLine(int32 MissionID) {
	if (!Missions.Contains(MissionID)) {
		return;
	}
	if (Missions[MissionID].MissionLine) {
		Missions[MissionID].MissionLine->DestroyInterestLines();
		Missions[MissionID].MissionLine = nullptr;
	}
}

void USPMissionManager::RefreshMissionLine(int32 MissionID) {
	DespawnMissionLine(MissionID);
	SpawnMissionLine(MissionID);
}
