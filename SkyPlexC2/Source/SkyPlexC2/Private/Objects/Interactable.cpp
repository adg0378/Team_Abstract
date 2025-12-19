#include "Objects/Interactable.h"
#include "SPUtility.h"
#include "State/SPGameState.h"
#include "SPLogger.h"
#include "Player/SPPlayerController.h"
#include <CesiumGlobeAnchorComponent.h>
#include "Components/StaticMeshComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/SceneComponent.h"

#undef GetObject

AInteractable::AInteractable()
	: userPlacementEnabled(false),
	isSelected(false),
	defaultMaterial(USPUtility::StateToMaterial(Cast<AActor>(this), EState::Unassigned)),
	selectedMaterial(USPUtility::StateToMaterial(Cast<AActor>(this), EState::Selected))
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = AActor::CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	if (!staticMesh) {
		staticMesh = AActor::CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	}

	staticMesh->SetupAttachment(RootComponent);

	cesiumGlobeAnchor = AActor::CreateDefaultSubobject<UCesiumGlobeAnchorComponent>(TEXT("CesiumGlobeAnchor"));

	// Ensure the actor is moveable
	if (GetRootComponent()->Mobility == EComponentMobility::Static)
	{
		GetRootComponent()->SetMobility(EComponentMobility::Movable);
	}

	// Add cesium globe anchor component
	if (cesiumGlobeAnchor)
	{
		//cesiumGlobeAnchor->RegisterComponent();
		//this->AddInstanceComponent(cesiumGlobeAnchor);
		cesiumGlobeAnchor->Activate();
	}
}

// Called when the game starts or when spawned
void AInteractable::BeginPlay()
{
	AActor::BeginPlay();

	if (staticMesh) {
		staticMesh->SetGenerateOverlapEvents(true);
		staticMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		staticMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

		// IMPORTANT: collision preset from within the unreal engine editor
		staticMesh->SetCollisionProfileName(TEXT("InterestPoint"));

		staticMesh->OnBeginCursorOver.AddDynamic(this, &AInteractable::OnHover);
		staticMesh->OnEndCursorOver.AddDynamic(this, &AInteractable::OnHoverEnd);
		staticMesh->OnClicked.AddDynamic(this, &AInteractable::OnClick);
	}
}

void AInteractable::StartDrag() {
	
}

void AInteractable::Select() {
	AInteractable::isSelected = true;

	// Set material
	if (staticMesh) {
		if (selectedMaterial) {
			staticMesh->SetMaterial(0, selectedMaterial);
		}
		staticMesh->SetRenderCustomDepth(true);
	}

	// Add to selected list
	ASPGameState* gameState = USPUtility::GetSPGameState(Cast<UObject>(this));
	gameState->AddSelected(this);
}

void AInteractable::Deselect(bool deleteFromGameStateSelected, bool unhighlight) {
	if (AInteractable::isSelected) {
		AInteractable::isSelected = false;

		// Set material
		if (staticMesh) {
			if (defaultMaterial) {
				staticMesh->SetMaterial(0, defaultMaterial);
			}

			if (unhighlight) {
				staticMesh->SetRenderCustomDepth(false);
			}
		}

		if (deleteFromGameStateSelected) {
			// Deselect
			ASPGameState* gameState = USPUtility::GetSPGameState(Cast<UObject>(this));
			gameState->RemoveSelected(this);
		}
	}
}

void AInteractable::OnHover(UPrimitiveComponent* touchedComponent) {
	ASPPlayerController* controller = USPUtility::GetSPPlayerController(Cast<UObject>(this));

	if (!controller->DraggedObject && controller->mode != EControllerMode::Placement) {
		staticMesh->SetRenderCustomDepth(true);
	}
}

void AInteractable::OnHoverEnd(UPrimitiveComponent* TouchedComponent) {
	if (!AInteractable::isSelected) {
		staticMesh->SetRenderCustomDepth(false);
	}
}

void AInteractable::OnClick(UPrimitiveComponent* touchedComponent, FKey buttonPressed) {
	if (AInteractable::isSelected) {
		if (userPlacementEnabled) {
			StartDrag();
		}
		else {
			Deselect(true, false);
		}
	}
	else {
		ASPPlayerController* controller = USPUtility::GetSPPlayerController(Cast<UObject>(this));
		if (controller->mode != EControllerMode::Placement) {
			Select();
			if (userPlacementEnabled) {
				controller->CurrentMouseCursor = EMouseCursor::Type::CardinalCross;
			}
		}
	}
}

void AInteractable::DestroySelf_Implementation() {
	if (AInteractable::isSelected) {
		AInteractable::Deselect();
	}
	UObject* ProviderObj = LinkedProvider.GetObject();
	if (ProviderObj && ProviderObj->GetClass()->ImplementsInterface(UInteractionBoxProvider::StaticClass())) {
		IInteractionBoxProvider::Execute_DestroySelf(LinkedProvider.GetObject());
	}
	else {
		if (ProviderObj) {
			UE_LOG(LogTemp, Warning, TEXT("LINKED PROVIDER DOES NOT IMPLEMENT INTERFACE"))
		}
		Destroy();
	}
}

void AInteractable::GetInteractionBoxTitle_Implementation(FText& OutTitle) {
	UObject* ProviderObj = LinkedProvider.GetObject();
	if (ProviderObj && ProviderObj->GetClass()->ImplementsInterface(UInteractionBoxProvider::StaticClass())) {
		IInteractionBoxProvider::Execute_GetInteractionBoxTitle(LinkedProvider.GetObject(), OutTitle);
	}
	else {
		if (ProviderObj) {
			UE_LOG(LogTemp, Warning, TEXT("LINKED PROVIDER DOES NOT IMPLEMENT INTERFACE"))
		}
		OutTitle = FText::FromString("");
	}
}

void AInteractable::OnInteractionBoxTitleChanged_Implementation(const FText &NewTitle) {}

void AInteractable::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) {
	OutKeyVals = TMap<FString, FInteractionBoxValue>();
	if (LinkedProvider.GetObject() != nullptr) {
		IInteractionBoxProvider::Execute_GetInteractionBoxKeyVals(LinkedProvider.GetObject(), OutKeyVals);
	}
}

void AInteractable::SetLinkedProvider_Implementation(const TScriptInterface<UInteractionBoxProvider>& InProvider) {
	LinkedProvider = InProvider;
}

void AInteractable::GetLinkedProvider_Implementation(TScriptInterface<UInteractionBoxProvider>& OutProvider) const {
	OutProvider = LinkedProvider;
}

void AInteractable::GetLongitudeLatitudeHeight(FVector& OutLonLatHeight) const {
	OutLonLatHeight = this->cesiumGlobeAnchor->GetLongitudeLatitudeHeight();
}

void AInteractable::MoveToLongitudeLatitudeHeight(FVector InLonLatHeight) const {
	this->cesiumGlobeAnchor->MoveToLongitudeLatitudeHeight(InLonLatHeight);
}

void AInteractable::ToggleCull_Implementation(bool IsCulled, bool FromFlyTo) {

}
