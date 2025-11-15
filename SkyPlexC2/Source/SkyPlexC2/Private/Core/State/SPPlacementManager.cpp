// Copyright (c) 2025 Synetos Aerospace


#include "Core/State/SPPlacementManager.h"
#include "Kismet/GameplayStatics.h"
#include "Core/State/SPSelectionManager.h"
#include "Core/State/SPGameState.h"
#include "Objects/SPInteractable.h"
#include "Objects/SPPlaceable.h"
#include "Objects/Geo/SPPolygon.h"
#include "Objects/Geo/SPPlaceablePoint.h"
#include "Util/SPAssetUtility.h"

USPPlacementManager::USPPlacementManager() {
	POIPointToSpawn = ASPPlaceablePoint::StaticClass();
	TakeoffPointToSpawn = ASPPlaceablePoint::StaticClass();
	AOIPointToSpawn = ASPPlaceablePoint::StaticClass();
	AOIPolygonToSpawn = ASPPolygon::StaticClass();
	GeoFencePointToSpawn = ASPPlaceablePoint::StaticClass();
	GeoFencePolygonToSpawn = ASPPolygon::StaticClass();
}

void USPPlacementManager::PositionActorOnTileset(UObject* WorldContextObject, AActor* Actor, FVector OffsetFromCurrentLocation) {
	UWorld* World = WorldContextObject->GetWorld();
	Actor->SetActorEnableCollision(false);

	FVector startLocation = Actor->GetActorLocation() - OffsetFromCurrentLocation;
	FVector newLocation = startLocation;
	FVector upVector = Actor->GetActorUpVector();
	FHitResult upHitResult, downHitResult;
	bool upHit = World->LineTraceSingleByChannel(upHitResult, startLocation, startLocation + (upVector * TRACE_DISTANCE), USPAssetUtility::LandscapeTraceChannel);
	bool downHit = World->LineTraceSingleByChannel(downHitResult, startLocation, startLocation + (-upVector * TRACE_DISTANCE), USPAssetUtility::LandscapeTraceChannel);
	if (upHit && downHit) {
		//DrawDebugLine(world, newLocation, upHitResult.ImpactPoint, FColor::Red, false, 5.0f, 0, 1.0f);
		//DrawDebugLine(world, newLocation, downHitResult.ImpactPoint, FColor::Red, false, 5.0f, 0, 1.0f);

		// If uphit is closer than downhit, use uphit
		const FVector upDelta = upHitResult.ImpactPoint - startLocation;
		const FVector downDelta = downHitResult.ImpactPoint - startLocation;
		if (upDelta.Size() <= downDelta.Size()) {
			newLocation = startLocation + upVector * FVector::DotProduct(upDelta, upVector);
		}
		else {
			newLocation = startLocation + upVector * FVector::DotProduct(downDelta, upVector);
		}
	}
	else if (upHit) {
		//DrawDebugLine(world, newLocation, upHitResult.ImpactPoint, FColor::Red, false, 5.0f, 0, 2.0f);
		const FVector upDelta = upHitResult.ImpactPoint - startLocation;
		newLocation = startLocation + upVector * FVector::DotProduct(upDelta, upVector);
	}
	else if (downHit) {
		//DrawDebugLine(world, newLocation, downHitResult.ImpactPoint, FColor::Red, false, 5.0f, 0, 2.0f);
		const FVector downDelta = downHitResult.ImpactPoint - startLocation;
		newLocation = startLocation + upVector * FVector::DotProduct(downDelta, upVector);
	}
	Actor->SetActorLocation(newLocation);
	Actor->SetActorEnableCollision(true);
}

void USPPlacementManager::PositionActorOnTilesetFromMousePos(UObject* WorldContextObject, AActor* Actor, FVector OffsetFromMouse, bool MoveAllSelected) {
	UWorld* World = WorldContextObject->GetWorld();

	if (!Actor || !World) {
		return;
	}

	FVector worldLocationStart, worldDirection;
	if (UGameplayStatics::GetPlayerController(World, 0)->DeprojectMousePositionToWorld(worldLocationStart, worldDirection)) {
		FVector end = worldLocationStart + (worldDirection * TRACE_DISTANCE);
		FHitResult hit;
		World->LineTraceSingleByChannel(hit, worldLocationStart, end, USPAssetUtility::LandscapeTraceChannel);
		if (hit.bBlockingHit) {
			ASPGameState* GameState = ASPGameState::GetSPGameState(WorldContextObject);
			if (GameState->SelectionManager->IsMultipleSelected() && MoveAllSelected) {
				FVector movedOffset = Actor->GetActorLocation() - (hit.Location + OffsetFromMouse);
				for (ASPInteractable* a : GameState->SelectionManager->GetSelectedActors()) {
					if (a->UserPlacementEnabled) {
						USPPlacementManager::PositionActorOnTileset(WorldContextObject, a, movedOffset);
					}
				}
			}
			else {
				FVector movedOffset = Actor->GetActorLocation() - (hit.Location + OffsetFromMouse);
				USPPlacementManager::PositionActorOnTileset(WorldContextObject, Actor, movedOffset);
			}
		}
	}
}

void USPPlacementManager::StopDraggingObject() {
	if (DraggedObject) {
		DraggedObject->StopDrag();
		DraggedObject = nullptr;
	}
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->CurrentMouseCursor = EMouseCursor::Default;
}

bool USPPlacementManager::IsPlacementModeEnabled() const {
	return PlacementModeEnabled;
}

void USPPlacementManager::SetPlacementModeEnabled(bool IsEnabled) {
	if (IsEnabled == PlacementModeEnabled) {
		return;
	}
	PlacementModeEnabled = IsEnabled;

	if (PlacementModeEnabled && PlaceableActorType) {
		ASPGameState* GameState = ASPGameState::GetSPGameState(this);
		GameState->SelectionManager->DeselectAll();

		if (PlaceableActorType == GeoFencePointToSpawn || PlaceableActorType == AOIPointToSpawn) {
			InitializeInProgressPolygon(GameState);
		}

		ASPPlaceable* Placeable;
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

void USPPlacementManager::SpawnActor() {
	if (!PlaceableActor || !PlaceableActor->IsPlacementValid()) {
		return;
	}

	ASPGameState* GameState = ASPGameState::GetSPGameState(this);
	bool IsPolygonal = PlaceableActorType == GeoFencePointToSpawn || PlaceableActorType == AOIPointToSpawn;

	if (IsPolygonal && InProgressPolygon && InProgressPolygon->IsClosed) {
		InProgressPolygon->Close();
		if (PlaceableActorType == GeoFencePointToSpawn) {
			/*USPGeoFence* GeoFence = NewObject<USPGeoFence>(this);
			GeoFence->SetPolygon(InProgressPolygon);
			GameState->MissionManagerRef->AddInterest(GeoFence);*/
		}
		else {
			/*UAreaOfInterest* AOI = NewObject<UAreaOfInterest>(this);
			AOI->SetPolygon(InProgressPolygon);
			GameState->MissionManagerRef->AddInterest(AOI);*/
		}
		InitializeInProgressPolygon(GameState);

		bool _;
		PlaceableActor->UpdatePlacementState(false, false, _);

		OnPlacementInfoUpdated(nullptr, EControllerMode::Default);
	}
	else {
		const FTransform& Transform = PlaceableActor->GetActorTransform();

		ASPPlaceable* Placeable;
		GameState->GenericSpawnActor<ASPPlaceable>(PlaceableActorType, Transform.GetLocation(), Placeable, Transform.GetScale3D());

		Placeable->CustomOnPlaced();

		bool _;
		Placeable->SetOverlapEvents(false, _);

		ASPPlaceablePoint* Point = Cast<ASPPlaceablePoint>(Placeable);
		if (IsPolygonal) {
			InProgressPolygon->AddPoint(Point);
		}
		else if (PlaceableActorType /*&& PlaceableActorType->IsChildOf(GameState->MissionManagerRef->POIPointToSpawn)*/) {
			/*UPOI* POI = NewObject<UPOI>(this);
			POI->SetPoint(Point);
			GameState->MissionManagerRef->AddInterest(POI);*/
		}
		else {
			/*USPTakeoffPoint* TakeoffPoint = NewObject<USPTakeoffPoint>(this);
			TakeoffPoint->SetPoint(Point);
			GameState->MissionManagerRef->AddInterest(TakeoffPoint);*/

			OnPlacementInfoUpdated(nullptr, EControllerMode::Default);
		}
	}
}

void USPPlacementManager::OnPlacementInfoUpdated(TSubclassOf<ASPPlaceable> InPlaceableActorType, EControllerMode ControllerMode) {
	PlaceableActorType = InPlaceableActorType;

	SetPlacementModeEnabled(false);
	Mode = ControllerMode;

	if (Mode == EControllerMode::Placement) {
		SetPlacementModeEnabled(true);
	}

	else {
		OnPlacementModeExited.Broadcast();
	}
}

void USPPlacementManager::InitializeInProgressPolygon(ASPGameState* GameState) {
	TSubclassOf<ASPPolygon> PolygonToSpawn =
		PlaceableActorType == AOIPointToSpawn ? AOIPolygonToSpawn : GeoFencePolygonToSpawn;

	ASPPolygon* Polygon;
	GameState->GenericSpawnActor(PolygonToSpawn, FVector(0.0f, 0.0f, -100000.0f), Polygon);

	if (!Polygon) {
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize polygon in progress"));
		return;
	}

	InProgressPolygon = Polygon;
}

void USPPlacementManager::ToggleMultipleSelectMode() {
	if (Mode == EControllerMode::Default) {
		Mode = EControllerMode::MultiSelect;
	}
	else if (Mode == EControllerMode::MultiSelect) {
		Mode = EControllerMode::Default;
	}
}

void USPPlacementManager::Tick(float DeltaTime) {
	if (PlacementModeEnabled) {
		PositionActorOnTilesetFromMousePos(this, PlaceableActor);
	}
}

bool USPPlacementManager::IsTickable() const {
	return PlacementModeEnabled;
}

bool USPPlacementManager::IsTickableInEditor() const {
	return false;
}
bool USPPlacementManager::IsTickableWhenPaused() const {
	return false;
}

TStatId USPPlacementManager::GetStatId() const {
	return TStatId();
}