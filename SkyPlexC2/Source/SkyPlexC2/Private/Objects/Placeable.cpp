#include "Objects/Placeable.h"
#include "SPUtility.h"
#include "State/SPGameState.h"
#include "Components/BillboardComponent.h"
#include "SPLogger.h"
#include "Player/SPPlayerController.h"

#undef GetObject

TObjectPtr<UMaterialInstance> APlaceable::defaultInvalidMaterial = nullptr;

// Handles drag for multi-selection contexts
static void SetOverlapEventsOnSelected(UObject* worldContextObject, bool& invalidPlacements, bool bind = false) {
	invalidPlacements = false;
	ASPGameState* gameState = USPUtility::GetSPGameState(worldContextObject);

	for (AInteractable* actor : gameState->selectedActors) {

		APlaceable* placeable = Cast<APlaceable>(actor);
		if (placeable) {
			bool invalidPlacement;
			placeable->SetOverlapEvents(bind, invalidPlacement);

			if (invalidPlacement) {
				invalidPlacements = true;
			}
		}
	}
}

static void MoveSelectedToPosBeforeDrag(UObject* worldContextObject) {
	ASPGameState* gameState = USPUtility::GetSPGameState(worldContextObject);
	for (AInteractable* actor : gameState->selectedActors) {

		APlaceable* placeable = Cast<APlaceable>(actor);
		if (placeable) {
			placeable->SetActorLocation(placeable->posBeforeDrag);
		}
	}
}

static void CallProviderOnPlacedOnAllSelected(UObject* worldContextObject) {
	ASPGameState* GameState = USPUtility::GetSPGameState(worldContextObject);
	for (AInteractable* Actor : GameState->selectedActors) {

		APlaceable* Placeable = Cast<APlaceable>(Actor);
		if (Placeable) {
			IPlaceableProvider::Execute_ProvideOnPlaced(Placeable);
		}
	}
}

APlaceable::APlaceable()
	: AStateboundInteractable(),
	isDragging(false),
	dragOffset(FVector()),
	isPlacementValid(true),
	posBeforeDrag(FVector())
{
	if (defaultInvalidMaterial == nullptr) {
		defaultInvalidMaterial = USPUtility::StateToMaterial(this, EState::Error);
	}
	userPlacementEnabled = true;
}

void APlaceable::BeginPlay() {
	Super::BeginPlay();

	if (staticMesh) {
		staticMesh->OnReleased.AddDynamic(this, &APlaceable::PlaceableOnRelease);
	}
}

void APlaceable::StartDrag() {
	CustomBeforeDrag();
	// Setup
	ASPPlayerController* controller = USPUtility::GetSPPlayerController(this);
	isDragging = true;
	controller->DraggedObject = this;
	bool temp;
	SetOverlapEventsOnSelected(this, temp, true);

	// Calculate starting mouse pos
	FHitResult hit;
	if (controller->GetHitResultUnderCursorByChannel(USPUtility::GetLandscapeTraceType(), false, hit)) {
		dragOffset = this->GetActorLocation();
		dragOffset.X -= hit.Location.X;
		dragOffset.Y -= hit.Location.Y;
		dragOffset.Z = 0;
	}

	// Set drag event timer
	FTimerManagerTimerParameters timerParams;
	timerParams.bLoop = timerParams.bMaxOncePerFrame = true;
	GetWorld()->GetTimerManager().SetTimer(dragTimerHandle, this, &APlaceable::TimerEvent, 0.01f, timerParams);
}

void APlaceable::StopDrag() {
	if (isDragging) {
		GetWorld()->GetTimerManager().ClearTimer(dragTimerHandle);
		USPLogger* log = USPUtility::GetLogger(this);
		ASPPlayerController* controller = USPUtility::GetSPPlayerController(this);
		isDragging = false;

		// Cancel the drag if any placement is invalid
		bool invalidPlacements;
		SetOverlapEventsOnSelected(this, invalidPlacements);
		if (invalidPlacements) {
			MoveSelectedToPosBeforeDrag(this);
		}
		else {
			USPUtility::SetActorLocationAboveCesiumTerrain(this, this);
			CallProviderOnPlacedOnAllSelected(this);
		}
		CustomAfterDrag();

		controller->DraggedObject = nullptr;
	}
}

void APlaceable::TimerEvent() {
	USPUtility::MoveActorToMousePosition(this, this, dragOffset, true);
}

void APlaceable::OnHover(UPrimitiveComponent* touchedComponent) {
	Super::OnHover(touchedComponent);
	if (isSelected) {
		GetWorld()->GetFirstPlayerController()->CurrentMouseCursor = EMouseCursor::Type::CardinalCross;
	}
}

void APlaceable::OnHoverEnd(UPrimitiveComponent* touchedComponent) {
	Super::OnHoverEnd(touchedComponent);
	if (isSelected && !isDragging) {
		GetWorld()->GetFirstPlayerController()->CurrentMouseCursor = EMouseCursor::Type::Default;
	}
}

void APlaceable::PlaceableOnRelease(UPrimitiveComponent* touchedComponent, FKey buttonReleased) {
	StopDrag();
}

void APlaceable::SetOverlapEvents(bool bind, bool& invalidPlacementOnSet) {
	if (bind) {
		posBeforeDrag = this->GetActorLocation();
		staticMesh->OnComponentBeginOverlap.AddDynamic(this, &APlaceable::PlaceableOnBeginOverlap);
		staticMesh->OnComponentEndOverlap.AddDynamic(this, &APlaceable::PlaceableOnEndOverlap);
		UpdatePlacementState(false, false, invalidPlacementOnSet);
	}
	else {
		staticMesh->OnComponentBeginOverlap.RemoveDynamic(this, &APlaceable::PlaceableOnBeginOverlap);
		staticMesh->OnComponentEndOverlap.RemoveDynamic(this, &APlaceable::PlaceableOnEndOverlap);
		UpdatePlacementState(true, false, invalidPlacementOnSet);
	}
}

void APlaceable::PlaceableOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	bool temp;
	UpdatePlacementState(false, false, temp);
}

void APlaceable::PlaceableOnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	bool temp;
	UpdatePlacementState(false, true, temp);
}

void APlaceable::UpdatePlacementState(bool forceValid, bool onEndOverlap, bool& invalidPlacement) {
	if (onEndOverlap) {
		CustomOnEndOverlap();
	}

	TArray<AActor*> overlappingActors;
	this->GetOverlappingActors(overlappingActors, AInteractable::StaticClass());

	if (overlappingActors.Num() == 0 || forceValid) {
		invalidPlacement = false;
		isPlacementValid = true;
		staticMesh->SetMaterial(0, isSelected ? selectedMaterial : defaultMaterial);
	}
	else {
		invalidPlacement = true;
		isPlacementValid = false;

		if (!isDragging) {
			bool preventDefault;
			CustomOverlapHandler(overlappingActors, preventDefault);

			if (preventDefault) {
				return;
			}
		}
		staticMesh->SetMaterial(0, defaultInvalidMaterial);
	}
}

void APlaceable::CustomOverlapHandler_Implementation(UPARAM(ref) TArray<AActor*>& overlappingActors, bool& preventDefault) {
	preventDefault = false;
}

void APlaceable::CustomOnEndOverlap_Implementation() {

}

void APlaceable::CustomBeforeDrag_Implementation() {}

void APlaceable::CustomAfterDrag_Implementation() {}

void APlaceable::CustomOnStartPlacing_Implementation() {};

void APlaceable::CustomOnPlaced_Implementation() {};

void APlaceable::ProvideOnPlaced_Implementation() {
	UObject* ProviderObj = LinkedProvider.GetObject();
	if (ProviderObj && ProviderObj->GetClass()->ImplementsInterface(UPlaceableProvider::StaticClass())) {
		IPlaceableProvider::Execute_ProvideOnPlaced(LinkedProvider.GetObject());
	}
	else {
		if (ProviderObj) {
			UE_LOG(LogTemp, Warning, TEXT("Linked provider does not implement PlaceableProvider"))
		}
	}
}
