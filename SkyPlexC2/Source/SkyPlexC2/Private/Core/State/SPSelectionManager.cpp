// Copyright (c) 2025 Synetos Aerospace


#include "Core/State/SPSelectionManager.h"
#include "Objects/SPInteractable.h"
#include "Core/State/SPGameState.h"
#include "Core/State/SPPlacementManager.h"
#include "Util/SPAssetUtility.h"

void USPSelectionManager::AddSelected(ASPInteractable* Actor) {
	if (ASPGameState::GetSPGameState(this)->PlacementManager->Mode != EControllerMode::MultiSelect) {
		DeselectAll();
	}
	int Index = SelectedActors.Add(Actor);

	RefreshSelected();
}

void USPSelectionManager::RemoveSelected(ASPInteractable* Actor) {
	SelectedActors.Remove(Actor);

	RefreshSelected();
}

void USPSelectionManager::RefreshSelected() {
	if (SelectedActors.IsEmpty()) {
		DeleteInteractionBox();
	}
	else {
		FText Title = CompileInteractionBoxTitle();
		TMap<FString, FInteractionBoxValue> KeyVals;
		CompileInteractionBoxKeyVals(KeyVals);
		UpdateInteractionBox(Title, KeyVals);
	}
}

void USPSelectionManager::UpdateInteractionBox_Implementation(const FText& Title, const TMap<FString, FInteractionBoxValue>& KeyVals) {

}

void USPSelectionManager::DeleteInteractionBox_Implementation() {

}

void USPSelectionManager::DeselectAll() {
	for (ASPInteractable* Actor : SelectedActors) {
		Actor->Deselect(false);
	}
	SelectedActors.Empty();

	DeleteInteractionBox();
}

bool USPSelectionManager::IsSelected() const {
	return !SelectedActors.IsEmpty();
}

bool USPSelectionManager::IsMultipleSelected() const {
	return SelectedActors.Num() > 1;
}

void USPSelectionManager::DeleteSelected() {
	TArray<ASPInteractable*> SelectedCopy = SelectedActors;
	for (ASPInteractable* Actor : SelectedCopy) {
		ISPInteractionInterface::Execute_DestroySelf(Actor);
	}

	DeleteInteractionBox();
}

void USPSelectionManager::CompileInteractionBoxKeyVals(TMap<FString, FInteractionBoxValue>& OutKeyVals) {
	OutKeyVals.Empty();

	if (SelectedActors.Num() == 1) {
		ISPInteractionInterface::Execute_GetInteractionBoxKeyVals(SelectedActors[0], OutKeyVals);
		return;
	}

	bool InitializedKeyVals = false;
	for (ASPInteractable* actor : SelectedActors) {
		TMap<FString, FInteractionBoxValue> actorKeyVals;
		ISPInteractionInterface::Execute_GetInteractionBoxKeyVals(actor, actorKeyVals);

		if (!InitializedKeyVals) {
			OutKeyVals = actorKeyVals;
			InitializedKeyVals = true;
			continue;
		}

		TArray<FString> keys;
		OutKeyVals.GetKeys(keys);

		for (const FString& key : keys) {
			if (!actorKeyVals.Contains(key) || !(*actorKeyVals.Find(key) == *OutKeyVals.Find(key))) {
				OutKeyVals.Remove(key);
			}
		}
	}
}

FText USPSelectionManager::CompileInteractionBoxTitle() {
	if (SelectedActors.Num() == 1) {
		FText Title;
		ISPInteractionInterface::Execute_GetInteractionBoxTitle(SelectedActors[0], Title);
		return Title;
	}

	return FText::FromString(TEXT("Multiple Selected"));
}

const TArray<TObjectPtr<ASPInteractable>>& USPSelectionManager::GetSelectedActors() const {
	return SelectedActors;
}

void USPSelectionManager::CheckMouseLocationForDeselect(FVector& WorldLocation, FVector& WorldDirection) {
	if (!ASPGameState::GetSPGameState(this)->PlacementManager->DraggedObject && IsSelected()) {
		float TraceDistance = 5000000.0f;
		FHitResult Hit;
		FVector EndLocation = WorldDirection * TraceDistance + WorldLocation;
		if (!GetWorld()->LineTraceSingleByChannel(Hit, WorldLocation, EndLocation, ECC_Visibility) || !Cast<ASPInteractable>(Hit.GetActor())) {
			DeselectAll();
		}
	}
}