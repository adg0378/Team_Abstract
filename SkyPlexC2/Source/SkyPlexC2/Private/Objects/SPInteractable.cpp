// Copyright (c) 2025 Synetos Aerospace


#include "Objects/SPInteractable.h"
#include "Core/State/SPGameState.h"
#include "Core/Player/SPPlayerController.h"
#include "Core/State/SPSelectionManager.h"
#include "Core/State/SPPlacementManager.h"
#include "CesiumGlobeAnchorComponent.h"

ASPInteractable::ASPInteractable() {
	if (!DefaultMaterial) {
		static ConstructorHelpers::FObjectFinder<UMaterialInstance> _DefaultMaterial(TEXT("/Game/Materials/InteractableBlue.InteractableBlue"));

		if (_DefaultMaterial.Succeeded()) {
			DefaultMaterial = _DefaultMaterial.Object;
		}
	}

	if (!SelectedMaterial) {
		static ConstructorHelpers::FObjectFinder<UMaterialInstance> _SelectedMaterial(TEXT("/Game/Materials/InteractableBlue.InteractableBlue"));

		if (_SelectedMaterial.Succeeded()) {
			SelectedMaterial = _SelectedMaterial.Object;
		}
	}

	if (!StaticMesh) {
		static ConstructorHelpers::FObjectFinder<UStaticMesh> _StaticMesh(TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube"));

		if (_StaticMesh.Succeeded()) {
			StaticMesh = _StaticMesh.Object;
		}
	}

	PrimaryActorTick.bCanEverTick = false;
	RootComponent = AActor::CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	if (!StaticMeshComponent) {
		StaticMeshComponent = AActor::CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	}

	StaticMeshComponent->SetupAttachment(RootComponent);

	GlobeAnchorComponent = AActor::CreateDefaultSubobject<UCesiumGlobeAnchorComponent>(TEXT("CesiumGlobeAnchor"));
	GlobeAnchorComponent->bAutoActivate = true;

	// Ensure the actor is moveable
	if (GetRootComponent()->Mobility == EComponentMobility::Static)
	{
		GetRootComponent()->SetMobility(EComponentMobility::Movable);
	}
}

// Called when the game starts or when spawned
void ASPInteractable::BeginPlay()
{
	AActor::BeginPlay();

	if (StaticMeshComponent) {
		StaticMeshComponent->SetGenerateOverlapEvents(true);
		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		StaticMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

		// collision preset from within the unreal engine editor
		StaticMeshComponent->SetCollisionProfileName(TEXT("SPInteractable"));

		StaticMeshComponent->OnBeginCursorOver.AddDynamic(this, &ASPInteractable::OnHover);
		StaticMeshComponent->OnEndCursorOver.AddDynamic(this, &ASPInteractable::OnHoverEnd);
		StaticMeshComponent->OnClicked.AddDynamic(this, &ASPInteractable::OnClick);
	}
}

void ASPInteractable::StartDrag() {

}

void ASPInteractable::Select() {
	ASPInteractable::_IsSelected = true;

	// Set material
	if (StaticMeshComponent) {
		if (SelectedMaterial) {
			StaticMeshComponent->SetMaterial(0, SelectedMaterial);
		}
		StaticMeshComponent->SetRenderCustomDepth(true);
	}

	// Add to selected list
	ASPGameState::GetSPGameState(Cast<UObject>(this))->SelectionManager->AddSelected(this);
}

void ASPInteractable::Deselect(bool DeleteFromGameStateSelected, bool Unhighlight) {
	if (ASPInteractable::_IsSelected) {
		ASPInteractable::_IsSelected = false;

		// Set material
		if (StaticMeshComponent) {
			if (DefaultMaterial) {
				StaticMeshComponent->SetMaterial(0, DefaultMaterial);
			}

			if (Unhighlight) {
				StaticMeshComponent->SetRenderCustomDepth(false);
			}
		}

		if (DeleteFromGameStateSelected) {
			// Deselect
			ASPGameState::GetSPGameState(Cast<UObject>(this))->SelectionManager->RemoveSelected(this);
		}
	}
}

void ASPInteractable::OnHover(UPrimitiveComponent* touchedComponent) {
	USPPlacementManager* PM = ASPGameState::GetSPGameState(Cast<UObject>(this))->PlacementManager;

	if (!PM->DraggedObject && PM->Mode != EControllerMode::Placement) {
		StaticMeshComponent->SetRenderCustomDepth(true);
	}
}

void ASPInteractable::OnHoverEnd(UPrimitiveComponent* TouchedComponent) {
	if (!ASPInteractable::_IsSelected) {
		StaticMeshComponent->SetRenderCustomDepth(false);
	}
}

void ASPInteractable::OnClick(UPrimitiveComponent* touchedComponent, FKey buttonPressed) {
	if (ASPInteractable::_IsSelected) {
		if (UserPlacementEnabled) {
			StartDrag();
		}
		else {
			Deselect(true, false);
		}
	}
	else {
		ASPPlayerController* Controller = ASPPlayerController::GetSPPlayerController(Cast<UObject>(this));
		if (ASPGameState::GetSPGameState(Cast<UObject>(this))->PlacementManager->Mode != EControllerMode::Placement) {
			Select();
			if (UserPlacementEnabled) {
				Controller->CurrentMouseCursor = EMouseCursor::Type::CardinalCross;
			}
		}
	}
}

void ASPInteractable::DestroySelf_Implementation() {
	if (ASPInteractable::_IsSelected) {
		ASPInteractable::Deselect();
	}
	UObject* ProviderObj = LinkedProvider.GetObject();
	if (ProviderObj && ProviderObj->GetClass()->ImplementsInterface(USPInteractionInterface::StaticClass())) {
		ISPInteractionInterface::Execute_DestroySelf(LinkedProvider.GetObject());
	}
	else {
		if (ProviderObj) {
			UE_LOG(LogTemp, Warning, TEXT("Linked provider does not implement interface: destroying"));
		}
		Destroy();
	}
}

void ASPInteractable::GetInteractionBoxTitle_Implementation(FText& OutTitle) {
	UObject* ProviderObj = LinkedProvider.GetObject();
	if (ProviderObj && ProviderObj->GetClass()->ImplementsInterface(USPInteractionInterface::StaticClass())) {
		ISPInteractionInterface::Execute_GetInteractionBoxTitle(LinkedProvider.GetObject(), OutTitle);
	}
	else {
		if (ProviderObj) {
			UE_LOG(LogTemp, Warning, TEXT("Linked provider does not implement interface: cannot get title"))
		}
		OutTitle = FText::FromString("");
	}
}

void ASPInteractable::OnInteractionBoxTitleChanged_Implementation(const FText& NewTitle) {}

void ASPInteractable::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) {
	OutKeyVals = TMap<FString, FInteractionBoxValue>();
	if (LinkedProvider.GetObject() != nullptr) {
		ISPInteractionInterface::Execute_GetInteractionBoxKeyVals(LinkedProvider.GetObject(), OutKeyVals);
	}
}

void ASPInteractable::SetLinkedProvider_Implementation(const TScriptInterface<USPInteractionInterface>& InProvider) {
	LinkedProvider = InProvider;
}

TScriptInterface<USPInteractionInterface> ASPInteractable::GetLinkedProvider_Implementation() const {
	return LinkedProvider;
}

FVector ASPInteractable::GetLongitudeLatitudeHeight() const {
	return this->GlobeAnchorComponent->GetLongitudeLatitudeHeight();
}

void ASPInteractable::MoveToLongitudeLatitudeHeight(FVector InLonLatHeight) const {
	this->GlobeAnchorComponent->MoveToLongitudeLatitudeHeight(InLonLatHeight);
}

void ASPInteractable::ToggleCull_Implementation(bool IsCulled) {

}

bool ASPInteractable::IsSelected() const {
	return _IsSelected;
}
