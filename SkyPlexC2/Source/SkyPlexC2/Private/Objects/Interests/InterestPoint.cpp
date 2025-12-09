// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/Interests/InterestPoint.h"
#include "SPUtility.h"
#include "State/SPGameState.h"
#include "State/Missions/SPMissionManager.h"


void AInterestPoint::DestroySelf_Implementation() {
	/*ASPGameState* gameState = USPUtility::GetSPGameState(this);
	gameState->interestManagerRef->DestroyPOI(this);
	this->Destroy();*/
}

void AInterestPoint::SetGroupID(int32 inID) {
	groupID = inID;
}

void AInterestPoint::GetGroupID(int32& outID) const {
	outID = groupID;
}

void AInterestPoint::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& outKeyVals) {
	Super::GetInteractionBoxKeyVals_Implementation(outKeyVals);

	ASPGameState* gameState = USPUtility::GetSPGameState(this);
	FString groupName = gameState->MissionManagerRef->GetMissionName(groupID);

	outKeyVals.Add(TEXT("Group"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(groupName)
		});

	outKeyVals.Add(TEXT("Delete interest"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::DeleteInterestButton,
		.value = FText::FromString(TEXT("Delete interest")),
		});
}