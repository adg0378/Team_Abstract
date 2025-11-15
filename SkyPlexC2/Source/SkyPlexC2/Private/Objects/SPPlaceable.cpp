// Copyright (c) 2025 Synetos Aerospace


#include "Objects/SPPlaceable.h"
#include "Core/State/SPGameState.h"
#include "Core/State/SPSelectionManager.h"
#include "Core/Player/SPPlayerController.h"
#include "Util/SPAssetUtility.h"
#include "Core/State/SPPlacementManager.h"

ASPPlaceable::ASPPlaceable()
{
	if (!DefaultInvalidMaterial) {
		static ConstructorHelpers::FObjectFinder<UMaterialInstance> _DefaultInvalidMaterial(TEXT("/Game/Materials/InteractableRed.InteractableRed"));

		if (_DefaultInvalidMaterial.Succeeded()) {
			DefaultInvalidMaterial = _DefaultInvalidMaterial.Object;
		}
	}
	UserPlacementEnabled = true;
}

// Handles drag for multi-selection contexts
static void SetOverlapEventsOnSelected(UObject* WorldContextObject, bool& InvalidPlacements, bool Bind = false) {
	InvalidPlacements = false;
	ASPGameState* GameState = ASPGameState::GetSPGameState(WorldContextObject);

	for (ASPInteractable* Actor : GameState->SelectionManager->GetSelectedActors()) {

		ASPPlaceable* Placeable = Cast<ASPPlaceable>(Actor);
		if (Placeable) {
			bool InvalidPlacement;
			Placeable->SetOverlapEvents(Bind, InvalidPlacement);

			InvalidPlacements |= InvalidPlacement;
		}
	}
}

static void MoveSelectedToPosBeforeDrag(UObject* WorldContextObject) {
	ASPGameState* GameState = ASPGameState::GetSPGameState(WorldContextObject);
	for (ASPInteractable* Actor : GameState->SelectionManager->GetSelectedActors()) {

		ASPPlaceable* Placeable = Cast<ASPPlaceable>(Actor);
		if (Placeable) {
			Placeable->SetToPosBeforeDrag();
		}
	}
}

static void CallProviderOnPlacedOnAllSelected(UObject* WorldContextObject) {
	ASPGameState* GameState = ASPGameState::GetSPGameState(WorldContextObject);
	for (ASPInteractable* Actor : GameState->SelectionManager->GetSelectedActors()) {

		ASPPlaceable* Placeable = Cast<ASPPlaceable>(Actor);
		if (Placeable) {
			ISPPlaceableInterface::Execute_ProvideOnPlaced(Placeable);
		}
	}
}

void ASPPlaceable::SetToPosBeforeDrag() {
	this->SetActorLocation(PosBeforeDrag);
}

void ASPPlaceable::BeginPlay() {
	Super::BeginPlay();

	if (StaticMeshComponent) {
		StaticMeshComponent->OnReleased.AddDynamic(this, &ASPPlaceable::PlaceableOnRelease);
	}
}

bool ASPPlaceable::IsDragging() const {
	return _IsDragging;
}

bool ASPPlaceable::IsPlacementValid() const {
	return _IsPlacementValid;
}

void ASPPlaceable::StartDrag() {
	CustomBeforeDrag();
	// Setup
	ASPPlayerController* Controller = ASPPlayerController::GetSPPlayerController(Cast<UObject>(this));
	_IsDragging = true;
	ASPGameState* GameState = ASPGameState::GetSPGameState(Cast<UObject>(this));
	GameState->PlacementManager->DraggedObject = this;
	bool Temp;
	SetOverlapEventsOnSelected(this, Temp, true);

	// Calculate starting mouse pos
	FHitResult Hit;
	if (Controller->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(USPAssetUtility::LandscapeTraceChannel), false, Hit)) {
		DragOffset = this->GetActorLocation();
		DragOffset.X -= Hit.Location.X;
		DragOffset.Y -= Hit.Location.Y;
		DragOffset.Z = 0;
	}

	// Set drag event timer
	FTimerManagerTimerParameters TimerParams;
	TimerParams.bLoop = TimerParams.bMaxOncePerFrame = true;
	GetWorld()->GetTimerManager().SetTimer(DragTimerHandle, this, &ASPPlaceable::TimerEvent, 0.01f, TimerParams);
}

void ASPPlaceable::StopDrag() {
	if (_IsDragging) {
		GetWorld()->GetTimerManager().ClearTimer(DragTimerHandle);
		ASPPlayerController* Controller = ASPPlayerController::GetSPPlayerController(Cast<UObject>(this));
		_IsDragging = false;

		// Cancel the drag if any placement is invalid
		bool invalidPlacements;
		SetOverlapEventsOnSelected(this, invalidPlacements);
		if (invalidPlacements) {
			MoveSelectedToPosBeforeDrag(this);
		}
		else {
			USPPlacementManager::PositionActorOnTileset(this, this);
			CallProviderOnPlacedOnAllSelected(this);
		}
		CustomAfterDrag();

		ASPGameState* GameState = ASPGameState::GetSPGameState(Cast<UObject>(this));
		GameState->PlacementManager->DraggedObject = nullptr;
	}
}

void ASPPlaceable::TimerEvent() {
	USPPlacementManager::PositionActorOnTilesetFromMousePos(this, this, DragOffset, true);
}

void ASPPlaceable::OnHover(UPrimitiveComponent* touchedComponent) {
	Super::OnHover(touchedComponent);
	if (_IsSelected) {
		GetWorld()->GetFirstPlayerController()->CurrentMouseCursor = EMouseCursor::Type::CardinalCross;
	}
}

void ASPPlaceable::OnHoverEnd(UPrimitiveComponent* touchedComponent) {
	Super::OnHoverEnd(touchedComponent);
	if (_IsSelected && !_IsDragging) {
		GetWorld()->GetFirstPlayerController()->CurrentMouseCursor = EMouseCursor::Type::Default;
	}
}

void ASPPlaceable::PlaceableOnRelease(UPrimitiveComponent* touchedComponent, FKey buttonReleased) {
	StopDrag();
}

void ASPPlaceable::SetOverlapEvents(bool Bind, bool& InvalidPlacementOnSet) {
	if (Bind) {
		PosBeforeDrag = this->GetActorLocation();
		StaticMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ASPPlaceable::PlaceableOnBeginOverlap);
		StaticMeshComponent->OnComponentEndOverlap.AddDynamic(this, &ASPPlaceable::PlaceableOnEndOverlap);
		UpdatePlacementState(false, false, InvalidPlacementOnSet);
	}
	else {
		StaticMeshComponent->OnComponentBeginOverlap.RemoveDynamic(this, &ASPPlaceable::PlaceableOnBeginOverlap);
		StaticMeshComponent->OnComponentEndOverlap.RemoveDynamic(this, &ASPPlaceable::PlaceableOnEndOverlap);
		UpdatePlacementState(true, false, InvalidPlacementOnSet);
	}
}

void ASPPlaceable::PlaceableOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	bool Temp;
	UpdatePlacementState(false, false, Temp);
}

void ASPPlaceable::PlaceableOnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	bool Temp;
	UpdatePlacementState(false, true, Temp);
}

void ASPPlaceable::UpdatePlacementState(bool ForceValid, bool OnEndOverlap, bool& InvalidPlacement) {
	if (OnEndOverlap) {
		CustomOnEndOverlap();
	}

	TArray<AActor*> OverlappingActors;
	this->GetOverlappingActors(OverlappingActors, ASPInteractable::StaticClass());

	if (OverlappingActors.Num() == 0 || ForceValid) {
		InvalidPlacement = false;
		_IsPlacementValid = true;
		StaticMeshComponent->SetMaterial(0, _IsSelected ? SelectedMaterial : DefaultMaterial);
	}
	else {
		InvalidPlacement = true;
		_IsPlacementValid = false;

		if (!_IsDragging) {
			bool preventDefault;
			CustomOverlapHandler(OverlappingActors, preventDefault);

			if (preventDefault) {
				return;
			}
		}
		StaticMeshComponent->SetMaterial(0, DefaultInvalidMaterial);
	}
}

void ASPPlaceable::CustomOverlapHandler_Implementation(UPARAM(ref) TArray<AActor*>& overlappingActors, bool& preventDefault) {
	preventDefault = false;
}

void ASPPlaceable::CustomOnEndOverlap_Implementation() {

}

void ASPPlaceable::CustomBeforeDrag_Implementation() {}

void ASPPlaceable::CustomAfterDrag_Implementation() {}

void ASPPlaceable::CustomOnStartPlacing_Implementation() {};

void ASPPlaceable::CustomOnPlaced_Implementation() {};

void ASPPlaceable::ProvideOnPlaced_Implementation() {
	UObject* ProviderObj = LinkedProvider.GetObject();
	if (ProviderObj && ProviderObj->GetClass()->ImplementsInterface(USPPlaceableInterface::StaticClass())) {
		ISPPlaceableInterface::Execute_ProvideOnPlaced(LinkedProvider.GetObject());
	}
	else {
		if (ProviderObj) {
			UE_LOG(LogTemp, Warning, TEXT("Linked provider does not implement PlaceableProvider"))
		}
	}
}
