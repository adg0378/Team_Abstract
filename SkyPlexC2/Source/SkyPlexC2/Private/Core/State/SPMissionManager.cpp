// Copyright (c) 2025 Synetos Aerospace


#include "Core/State/SPMissionManager.h"
#include "Core/State/SPGameState.h"
#include "Objects/Interests/SPPOI.h"
#include "Objects/Interests/SPAOI.h"
#include "Objects/Interests/SPGeoFence.h"
#include "Objects/Interests/SPTakeoffPoint.h"
#include "Objects/Geo/SPPlaceablePoint.h"
#include "Objects/Geo/SPPolygon.h"
#include "Core/State/SPPlacementManager.h"
#include "Core/State/SPSelectionManager.h"
#include "Core/State/SPDroneManager.h"
#include "Objects/Drones/SPCCSimMessenger.h"
#include "Objects/Drones/SPCCSimDrone.h"
#include "Util/SPMissionFormatter.h"

void USPMissionManager::Setup_Implementation() {
	Super::Setup_Implementation();
	LoadMissions();
	LoadInterests();
	StartMissionProgressTimer();
}

void USPMissionManager::Teardown_Implementation() {
	Super::Teardown_Implementation();
}

void USPMissionManager::PreTeardown() {
	StopMissionProgressTimer();
}

void USPMissionManager::LoadMissions() {
	GameStateRef->DatabaseManager->LoadMissions(Missions);

	for (TPair<int32, FSPMissionStruct>& Pair : Missions) {
		Interests.Add(Pair.Key);
	}
}

void USPMissionManager::LoadInterests() {
	TArray<FPOIDataStruct> POIData;
	TArray<FAOIDataStruct> AOIData;
	TArray<FGeoFenceDataStruct> GeoFenceData;
	TArray<FTakeoffPointDataStruct> TakeoffPointData;
	GameStateRef->DatabaseManager->LoadInterests(POIData, AOIData, GeoFenceData, TakeoffPointData);

	for (FPOIDataStruct Data : POIData) {
		USPPOI* POI = NewObject<USPPOI>(this);
		ASPPlaceablePoint* Point;
		GameStateRef->GenericSpawnActor<ASPPlaceablePoint>(GameStateRef->PlacementManager->POIPointToSpawn, FVector::ZeroVector, Point);
		Point->MoveToLongitudeLatitudeHeight(Data.longLatHeight);
		Point->CustomOnPlaced();
		POI->SetPoint(Point);
		POI->SetAttributes(Data.id, Data.groupID, Data.name);
		FSPInterestStruct* GroupInterests = Interests.Find(Data.groupID);
		GroupInterests->Interests.Add(Data.id, POI);
		//& GroupInterests = Interests.FindOrAdd(Data.groupID);
		//GroupInterests.Interests.Add(Data.id, POI);
	}

	for (FTakeoffPointDataStruct Data : TakeoffPointData) {
		USPTakeoffPoint* TakeoffPoint = NewObject<USPTakeoffPoint>(this);
		ASPPlaceablePoint* Point;
		GameStateRef->GenericSpawnActor<ASPPlaceablePoint>(GameStateRef->PlacementManager->TakeoffPointToSpawn, FVector::ZeroVector, Point);
		Point->MoveToLongitudeLatitudeHeight(Data.LonLatHeight);
		Point->CustomOnPlaced();
		TakeoffPoint->SetPoint(Point);
		TakeoffPoint->SetAttributes(Data.ID, Data.GroupID, Data.Name);
		FSPInterestStruct* GroupInterests = Interests.Find(Data.GroupID);
		GroupInterests->Interests.Add(Data.ID, TakeoffPoint);
	}

	for (FAOIDataStruct Data : AOIData) {
		USPAOI* AOI = NewObject<USPAOI>(this);
		ASPPolygon* Polygon;
		GameStateRef->GenericSpawnActor<ASPPolygon>(GameStateRef->PlacementManager->AOIPolygonToSpawn, FVector::ZeroVector, Polygon);
		for (const FVector& LonLatHeight : Data.longLatHeights) {
			ASPPlaceablePoint* Point;
			GameStateRef->GenericSpawnActor<ASPPlaceablePoint>(GameStateRef->PlacementManager->AOIPointToSpawn, FVector::ZeroVector, Point);
			Point->MoveToLongitudeLatitudeHeight(LonLatHeight);
			Point->CustomOnPlaced();
			Polygon->AddPoint(Point);
		}
		Polygon->Close();
		AOI->SetPolygon(Polygon);
		AOI->SetAttributes(Data.id, Data.groupID, Data.name);
		FSPInterestStruct* GroupInterests = Interests.Find(Data.groupID);
		GroupInterests->Interests.Add(Data.id, AOI);
	}

	for (FGeoFenceDataStruct Data : GeoFenceData) {
		USPGeoFence* GeoFence = NewObject<USPGeoFence>(this);
		ASPPolygon* Polygon;
		GameStateRef->GenericSpawnActor<ASPPolygon>(GameStateRef->PlacementManager->GeoFencePolygonToSpawn, FVector::ZeroVector, Polygon);
		for (const FVector& LonLatHeight : Data.LonLatHeights) {
			ASPPlaceablePoint* Point;
			GameStateRef->GenericSpawnActor<ASPPlaceablePoint>(GameStateRef->PlacementManager->GeoFencePointToSpawn, FVector::ZeroVector, Point);
			Point->MoveToLongitudeLatitudeHeight(LonLatHeight);
			Point->CustomOnPlaced();
			Polygon->AddPoint(Point);
		}
		Polygon->Close();
		GeoFence->SetPolygon(Polygon);
		GeoFence->SetAttributes(Data.ID, Data.GroupID, Data.Name);
		FSPInterestStruct* GroupInterests = Interests.Find(Data.GroupID);
		GroupInterests->Interests.Add(Data.ID, GeoFence);
	}
}

void USPMissionManager::AddMission(FString name, int32& OutMissionID) {
	int32 MissionID = -1;
	GameStateRef->DatabaseManager->InsertMission(name, MissionID);

	Missions.Add(MissionID, FSPMissionStruct{ .ID = MissionID, .Name = name });
	Interests.Add(MissionID);
	OutMissionID = MissionID;
	OnMissionAdded.Broadcast(MissionID);
	LOG->Info(FString::Format(*FString("Added mission: '{0}'"), { *name }), LOG_ORIGIN);
}

void USPMissionManager::DeleteMission(int32 InMissionID, int32 DumpMissionID, bool DestroyInterests) {
	GameStateRef->DatabaseManager->DeleteMission(InMissionID);
	Missions.Remove(InMissionID);

	const FSPInterestStruct* GroupInterests = Interests.Find(InMissionID);
	if (GroupInterests == nullptr) {
		LOG->Warn(FString::Printf(TEXT("Found no interest references for interest group '%i'"), InMissionID), LOG_ORIGIN);
		return;
	}

	if (DestroyInterests) {
		for (const TPair<int32, USPInterest*>& Pair : GroupInterests->Interests) {
			ISPInteractionInterface::Execute_DestroySelf(Pair.Value);
		}
	}
	else if (Missions.Contains(DumpMissionID)) {
		FSPInterestStruct* DumpGroupInterests = Interests.Find(DumpMissionID);

		for (const TPair<int32, USPInterest*>& Pair : GroupInterests->Interests) {
			int32 ID = Pair.Value->GetID();
			Pair.Value->SetGroupID(DumpMissionID);
			GameStateRef->DatabaseManager->ReassignInterest(ID, DumpMissionID);
			DumpGroupInterests->Interests.Add(Pair);
		}
		OnMissionUpdated.Broadcast(DumpMissionID);
	}
	Interests.Remove(InMissionID);
	OnMissionDeleted.Broadcast(InMissionID);
}

void USPMissionManager::RenameMission(int32 MissionID, FString NewName) {
	if (Missions.Contains(MissionID)) {
		Missions[MissionID].Name = NewName;
		GameStateRef->DatabaseManager->RenameMission(MissionID, NewName);

		GameStateRef->SelectionManager->RefreshSelected();

		OnMissionUpdated.Broadcast(MissionID);
	}
}

void USPMissionManager::UpdateMissionInteresttOrder(int32 MissionID, TArray<int32> InterestIDs) {
	if (Missions.Contains(MissionID)) {
		Missions[MissionID].InterestOrder = InterestIDs;
		GameStateRef->DatabaseManager->UpdateMissionOrder(MissionID, InterestIDs);

		OnMissionUpdated.Broadcast(MissionID);
	}
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

TArray<USPInterest*> USPMissionManager::GetInterestsInOrder(int32 MissionID) const {
	TArray<USPInterest*> OrderedInterests;

	const FSPInterestStruct* MissionInterests = Interests.Find(MissionID);
	if (MissionInterests) {
		for (const int32& ID : Missions[MissionID].InterestOrder) {
			USPInterest* Interest = MissionInterests->Interests.Find(ID)->Get();
			if (Interest) {
				OrderedInterests.Add(Interest);
			}
		}
	}
	return OrderedInterests;
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

void USPMissionManager::AddInterest(USPInterest* Interest, int32 GroupID) {
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
	FString Name = Interest->GetName();

	EInterestType InterestType = Interest->GetInterestType();
	if (InterestType == EInterestType::POI) {
		USPPOI* POI = Cast<USPPOI>(Interest);
		ASPPlaceablePoint* Point = POI->GetPoint();
		FVector LonLatHeight = Point->GetLongitudeLatitudeHeight();
		GameStateRef->DatabaseManager->InsertPOI(
			Name,
			LonLatHeight,
			GroupID,
			OutID
		);
	}
	else if (InterestType == EInterestType::Takeoff) {
		USPTakeoffPoint* TakeoffPoint = Cast<USPTakeoffPoint>(Interest);
		ASPPlaceablePoint* Point = TakeoffPoint->GetPoint();
		FVector LonLatHeight = Point->GetLongitudeLatitudeHeight();
		GameStateRef->DatabaseManager->InsertTakeoffPoint(
			Name,
			LonLatHeight,
			GroupID,
			OutID
		);
	}
	else if (InterestType == EInterestType::AOI) {
		USPAOI* AOI = Cast<USPAOI>(Interest);
		ASPPolygon* Polygon = AOI->GetPolygon();
		TArray<FVector> PointLocations;
		Polygon->GetPointLocations(PointLocations);
		GameStateRef->DatabaseManager->InsertAOI(
			Name,
			PointLocations,
			GroupID,
			OutID
		);
	}
	else if (InterestType == EInterestType::GeoFence) {
		USPGeoFence* GeoFence = Cast<USPGeoFence>(Interest);
		ASPPolygon* Polygon = GeoFence->GetPolygon();
		TArray<FVector> PointLocations;
		Polygon->GetPointLocations(PointLocations);
		GameStateRef->DatabaseManager->InsertGeoFence(
			Name,
			PointLocations,
			GroupID,
			OutID
		);
	}

	if (OutID == -1) {
		LOG->Error(FString("Unable to store Interest in database"), LOG_ORIGIN);
	}
	Interest->SetID(OutID);
	GroupInterests->Interests.Add(OutID, Interest);
	RenameInterestNoBroadcast(OutID, GroupID, FString::FromInt(OutID));

	Missions[GroupID].InterestOrder.Add(OutID);
	GameStateRef->DatabaseManager->UpdateMissionOrder(GroupID, Missions[GroupID].InterestOrder);

	LOG->Info(FString::Format(*FString("Succesfully plotted Interest with ID: '{0}'"), { OutID }));

	OnInterestAdded.Broadcast(OutID, GroupID);

	/*GameStateRef->obstacleManagerRef->HandleNewInterest(Interest);*/
}

void USPMissionManager::RemoveInterest(int32 InterestID, int32 GroupID) {
	FSPInterestStruct* GroupInterests = Interests.Find(GroupID);
	if (!GroupInterests) {
		LOG->Error(TEXT("Unable to remove interest: group does not exist"), LOG_ORIGIN);
		return;
	}

	USPInterest* Interest = GroupInterests->Interests.Find(InterestID)->Get();
	if (!Interest) {
		LOG->Error(TEXT("Unable to remove interest: interest does not exist"), LOG_ORIGIN);
		return;
	}

	GroupInterests->Interests.Remove(InterestID);

	/*GameStateRef->obstacleManagerRef->HandleDeleteInterest(Interest);*/
	GameStateRef->DatabaseManager->DeleteInterest(InterestID);

	OnInterestDeleted.Broadcast(InterestID, GroupID);
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

	USPInterest* Interest = OldGroupInterests->Interests.Find(InterestID)->Get();
	if (!Interest) {
		LOG->Error(TEXT("Unable to reassign interest: interest does not exist"), LOG_ORIGIN);
		return;
	}

	Interest->SetGroupID(NewGroupID);
	GameStateRef->DatabaseManager->ReassignInterest(InterestID, NewGroupID);

	GameStateRef->SelectionManager->RefreshSelected();

	OldGroupInterests->Interests.Remove(InterestID);
	NewGroupInterests->Interests.Add(InterestID, Interest);

	OnMissionUpdated.Broadcast(CurrGroupID);
	OnMissionUpdated.Broadcast(NewGroupID);
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

		USPInterest* Interest = OldGroupInterests->Interests.Find(IDToGroupPair.Key)->Get();
		if (!Interest) {
			LOG->Error(TEXT("Unable to reassign interest: interest does not exist"), LOG_ORIGIN);
			continue;
		}

		Interest->SetGroupID(NewGroupID);
		GameStateRef->DatabaseManager->ReassignInterest(IDToGroupPair.Key, NewGroupID);

		OldGroupInterests->Interests.Remove(IDToGroupPair.Key);
		NewGroupInterests->Interests.Add(IDToGroupPair.Key, Interest);
		GroupsToUpdate.Add(IDToGroupPair.Value);
	}

	GameStateRef->SelectionManager->RefreshSelected();

	for (int32& ID : GroupsToUpdate) {
		OnMissionUpdated.Broadcast(ID);
	}
}

void USPMissionManager::RenameInterest(int32 InterestID, int32 GroupID, FString NewName) {
	RenameInterestNoBroadcast(InterestID, GroupID, NewName);

	GameStateRef->SelectionManager->RefreshSelected();
	OnInterestUpdated.Broadcast(InterestID, GroupID);
}

void USPMissionManager::RenameInterestNoBroadcast(int32 InterestID, int32 GroupID, FString NewName) {
	FSPInterestStruct* GroupInterests = Interests.Find(GroupID);
	if (!GroupInterests) {
		LOG->Error(TEXT("Unable to rename interest: group does not exist"), LOG_ORIGIN);
		return;
	}

	USPInterest* Interest = GroupInterests->Interests.Find(InterestID)->Get();
	if (!Interest) {
		LOG->Error(TEXT("Unable to rename interest: interest does not exist"), LOG_ORIGIN);
		return;
	}

	Interest->SetName(NewName);
	GameStateRef->DatabaseManager->RenameInterest(InterestID, NewName);
}

void USPMissionManager::MoveInterest(int32 InterestID, int32 GroupID) {
	FSPInterestStruct* GroupInterests = Interests.Find(GroupID);
	if (!GroupInterests) {
		LOG->Error(TEXT("Unable to move interest: group does not exist"), LOG_ORIGIN);
		return;
	}

	USPInterest* Interest = GroupInterests->Interests.Find(InterestID)->Get();
	if (!Interest) {
		LOG->Error(TEXT("Unable to move interest: interest does not exist"), LOG_ORIGIN);
		return;
	}

	EInterestType InterestType = Interest->GetInterestType();
	if (InterestType == EInterestType::POI) {
		USPPOI* POI = Cast<USPPOI>(Interest);
		ASPPlaceablePoint* Point = POI->GetPoint();
		FVector LonLatHeight = Point->GetLongitudeLatitudeHeight();
		GameStateRef->DatabaseManager->MovePOI(InterestID, LonLatHeight);
	}
	else if (InterestType == EInterestType::Takeoff) {
		USPTakeoffPoint* Takeoff = Cast<USPTakeoffPoint>(Interest);
		ASPPlaceablePoint* Point = Takeoff->GetPoint();
		FVector LonLatHeight = Point->GetLongitudeLatitudeHeight();
		GameStateRef->DatabaseManager->MovePOI(InterestID, LonLatHeight);
	}
	else if (InterestType == EInterestType::AOI) {
		USPAOI* AOI = Cast<USPAOI>(Interest);
		ASPPolygon* Polygon = AOI->GetPolygon();
		TArray<FVector> PointLocations;
		Polygon->GetPointLocations(PointLocations);
		GameStateRef->DatabaseManager->MoveAOI(InterestID, PointLocations);
	}
	else if (InterestType == EInterestType::GeoFence) {
		USPGeoFence* GeoFence = Cast<USPGeoFence>(Interest);
		ASPPolygon* Polygon = GeoFence->GetPolygon();
		TArray<FVector> PointLocations;
		Polygon->GetPointLocations(PointLocations);
		GameStateRef->DatabaseManager->MoveGeoFence(InterestID, PointLocations);
	}
	/*GameStateRef->obstacleManagerRef->HandleMoveInterest(Interest);*/
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

USPInterest* USPMissionManager::GetInterest(int32 InterestID, int32 GroupID) const {
	const FSPInterestStruct* GroupInterests = Interests.Find(GroupID);
	if (!GroupInterests) {
		LOG->Error(TEXT("Unable to get interest: group does not exist"), LOG_ORIGIN);
		return nullptr;
	}

	USPInterest* Interest = GroupInterests->Interests.Find(InterestID)->Get();
	if (!Interest) {
		LOG->Error(TEXT("Unable to get interest: interest does not exist"), LOG_ORIGIN);
		return nullptr;
	}
	return Interest;
}

void USPMissionManager::CullRelatedObjects(float MaximumDrawDistance, const FVector& OriginLocationUE) {
	for (const TPair<int32, FSPInterestStruct>& Pair : Interests) {
		for (const TPair<int32, USPInterest*>& InterestPair : Pair.Value.Interests) {
			EInterestType InterestType = InterestPair.Value->GetInterestType();
			FVector LocationToCheck;

			if (InterestType == EInterestType::POI || InterestType == EInterestType::Takeoff) {
				ASPPlaceablePoint* Point;

				if (InterestType == EInterestType::POI) {
					USPPOI* POI = Cast<USPPOI>(InterestPair.Value);
					Point = POI->GetPoint();
				}
				else {
					USPTakeoffPoint* Takeoff = Cast<USPTakeoffPoint>(InterestPair.Value);
					Point = Takeoff->GetPoint();
				}
				LocationToCheck = Point->GetActorLocation();
			}
			else if (InterestType == EInterestType::AOI || InterestType == EInterestType::GeoFence) {
				ASPPolygon* Polygon;
				if (InterestType == EInterestType::AOI) {
					USPAOI* AOI = Cast<USPAOI>(InterestPair.Value);
					Polygon = AOI->GetPolygon();
				}
				else {
					USPGeoFence* GeoFence = Cast<USPGeoFence>(InterestPair.Value);
					Polygon = GeoFence->GetPolygon();
				}
				LocationToCheck = Polygon->GetCenterpointUE();
			}

			FVector DistanceVector = OriginLocationUE - LocationToCheck;
			bool IsCulled = DistanceVector.Size() > MaximumDrawDistance;
			ISPInteractionInterface::Execute_ToggleCull(InterestPair.Value, IsCulled);
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
			for (const TPair<int32, FString>& Pair : GameStateRef->DroneManager->GetSwarms()) {
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

	FString MissionMsg = USPCCSimMessenger::MakeMissionUploadMessage(EMissionUploadAction::START, FormatMissionAsJson(MissionID));

	TArray<const FSPSwarmStruct*> Drones = GameStateRef->DroneManager->GetSwarmDrones(Mission->AssignedSwarmIDs);
	for (const FSPSwarmStruct* Swarm : Drones) {
		for (const TPair<int32, ASPBaseDrone*>& Pair : Swarm->Drones) {
			ASPCCSimDrone* SimDrone = Cast<ASPCCSimDrone>(Pair.Value);
			if (SimDrone) {
				SimDrone->UploadMission(MissionMsg);
			}
		}
	}
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
		return USPMissionFormatter::FormatMissionAsJson(MissionInterests, Missions[MissionID].InterestOrder);
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
		TArray<const FSPSwarmStruct*> Drones = GameStateRef->DroneManager->GetSwarmDrones(Mission.Value.AssignedSwarmIDs);

		float ProgressSum = 0.0f;
		int NumDrones = 0;
		for (const FSPSwarmStruct* Swarm : Drones) {
			for (const TPair<int32, ASPBaseDrone*>& Pair : Swarm->Drones) {
				ProgressSum += Pair.Value->GetStatus().mission_progress;
				NumDrones++;
			}
		}

		float TotalProgress = NumDrones > 0 ? ProgressSum / NumDrones : 0.0f;
		LOG->Info(FString::Printf(TEXT("Mission Progress: %f"), TotalProgress));
		ProgressMap.Add(Mission.Key, TotalProgress);
	}

	OnMissionProgressUpdated.Broadcast(FSPMissionProgress{ .Progresses = ProgressMap });
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