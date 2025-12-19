// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/StateboundInteractable.h"
#include "SPUtility.h"

AStateboundInteractable::AStateboundInteractable()
	: AInteractable(),
	state(EState::Unassigned),
	id(-1) {

}

void AStateboundInteractable::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& outKeyVals) {
	Super::GetInteractionBoxKeyVals_Implementation(outKeyVals);
	/*outKeyVals.Add(TEXT("State"), FInteractionBoxValue{ .displayType = EKeyValDisplayType::UneditableText, .value = UEnum::GetDisplayValueAsText(state) });*/
}

void AStateboundInteractable::SetState(EState newState) {
	state = newState;
	defaultMaterial = USPUtility::StateToMaterial(this, state);
	if (!isSelected && staticMesh) {
		staticMesh->SetMaterial(0, defaultMaterial);
	}
}

void AStateboundInteractable::GetState(EState& outState) const {
	outState = state;
}

void AStateboundInteractable::GetID(int32& outID) const {
	outID = id;
}

void AStateboundInteractable::SetID(int32 inID) {
	id = inID;
}
