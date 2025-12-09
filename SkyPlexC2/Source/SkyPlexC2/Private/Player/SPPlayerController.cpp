// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SPPlayerController.h"
#include "SPUtility.h"
#include "State/SPGameState.h"
#include "Objects/Geometry/Polygon.h"
#include "Objects/Interests/SPGeoFence.h"
#include "Objects/Interests/AreaOfInterest.h"
#include "Objects/Interests/POI.h"
#include "Player/SPMainHUD.h"
#include "Objects/Interests/SPTakeoffPoint.h"
#include "State/Missions/SPMissionManager.h"

void ASPPlayerController::BeginPlay() {
	Super::BeginPlay();

	AGameStateBase* GS = GetWorld()->GetGameState();
	if (GS) {
		ASPGameState* GameState = Cast<ASPGameState>(GS);
		if (GameState->IsFullyInitialized()) {
			InitializeHUD();
		}
		else {
			GameState->OnGameStateFullyInitialized.AddDynamic(this, &ASPPlayerController::InitializeHUD);
		}
	}
}

void ASPPlayerController::InitializeHUD() {
	ASPMainHUD* HUD = Cast<ASPMainHUD>(GetHUD());
	HUD->SpawnHUD();
}

void ASPPlayerController::InitializeInProgressPolygon(ASPGameState* GameState) {
	TSubclassOf<APolygon> PolygonToSpawn =
		PlaceableActorType == AOIPinType
		? GameState->MissionManagerRef->AOIPolygonToSpawn
		: GameState->MissionManagerRef->GeoFencePolygonToSpawn;

	APolygon* Polygon;
	GameState->GenericSpawnActor(PolygonToSpawn, FVector(0.0f, 0.0f, -100000.0f), Polygon);

	if (!Polygon) {
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize polygon in progress"))
			return;
	}

	InProgressPolygon = Polygon;
}

void ASPPlayerController::SetPlacementModeEnabled(bool IsEnabled) {
	if (IsEnabled == PlacementModeEnabled) {
		return;
	}
	PlacementModeEnabled = IsEnabled;

	if (PlacementModeEnabled && PlaceableActorType) {
		ASPGameState* GameState = USPUtility::GetSPGameState(this);
		GameState->DeselectAll();

		if (PlaceableActorType == GeoFencePinType || PlaceableActorType == AOIPinType) {
			InitializeInProgressPolygon(GameState);
		}
		
		APlaceable* Placeable;
		GameState->GenericSpawnActor(PlaceableActorType, FVector(0.0f, 0.0f, -100000.0f), Placeable);

		if (!Placeable) {
			UE_LOG(LogTemp, Error, TEXT("Failed to initialize placeable to place"))
			return;
		}

		Placeable->CustomOnStartPlacing();
		PlaceableActor = Placeable;

		bool _;
		PlaceableActor->SetOverlapEvents(true, _);
	}
	else {
		if (PlaceableActor) {
			PlaceableActor->Destroy();
			PlaceableActor = nullptr;
		}

		if (InProgressPolygon) {
			InProgressPolygon->DestroyPolygon();
			InProgressPolygon = nullptr;
		}
	}
}

void ASPPlayerController::OnPlacementInfoUpdated(TSubclassOf<APlaceable> InPlaceableActorType, EControllerMode ControllerMode) {
	PlaceableActorType = InPlaceableActorType;

	SetPlacementModeEnabled(false);
	mode = ControllerMode;

	if (mode != EControllerMode::Default) {
		SetPlacementModeEnabled(true);
	}

	else {
		OnPlacementModeExited.Broadcast();
	}
}


void ASPPlayerController::SpawnActor() {
	if (!PlaceableActor || !PlaceableActor->isPlacementValid) {
		return;
	}

	ASPGameState* GameState = USPUtility::GetSPGameState(this);
	bool IsPolygonal = PlaceableActorType == GeoFencePinType || PlaceableActorType == AOIPinType;

	if (IsPolygonal && InProgressPolygon && InProgressPolygon->IsClosed) {
		InProgressPolygon->Close();
		if (PlaceableActorType == GeoFencePinType) {
			USPGeoFence* GeoFence = NewObject<USPGeoFence>(this);
			GeoFence->SetPolygon(InProgressPolygon);
			GameState->MissionManagerRef->AddInterest(GeoFence);
		}
		else {
			UAreaOfInterest* AOI = NewObject<UAreaOfInterest>(this);
			AOI->SetPolygon(InProgressPolygon);
			GameState->MissionManagerRef->AddInterest(AOI);
		}
		InitializeInProgressPolygon(GameState);

		bool _;
		PlaceableActor->UpdatePlacementState(false, false, _);

		OnPlacementInfoUpdated(nullptr, EControllerMode::Default);
	}
	else {
		const FTransform& Transform = PlaceableActor->GetActorTransform();

		APlaceable* Placeable;
		GameState->GenericSpawnActor<APlaceable>(PlaceableActorType, Transform.GetLocation(), Placeable, Transform.GetScale3D());

		Placeable->CustomOnPlaced();

		bool _;
		Placeable->SetOverlapEvents(false, _);

		APlaceablePoint* Point = Cast<APlaceablePoint>(Placeable);
		if (IsPolygonal) {
			InProgressPolygon->AddPoint(Point);
		}
		else if (PlaceableActorType && PlaceableActorType->IsChildOf(GameState->MissionManagerRef->POIPointToSpawn)) {
			UPOI* POI = NewObject<UPOI>(this);
			POI->SetPoint(Point);
			GameState->MissionManagerRef->AddInterest(POI);
		}
		else {
			USPTakeoffPoint* TakeoffPoint = NewObject<USPTakeoffPoint>(this);
			TakeoffPoint->SetPoint(Point);
			GameState->MissionManagerRef->AddInterest(TakeoffPoint);
			
			OnPlacementInfoUpdated(nullptr, EControllerMode::Default);
		}
	}
}