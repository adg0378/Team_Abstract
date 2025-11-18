// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Interests/SPInterest.h"
#include "Core/State/SPGameState.h"
#include "Core/State/SPMissionManager.h"

EInterestType USPInterest::GetInterestType() const {
	return EInterestType::Unknown;
}

void USPInterest::SetGroupID(int32 InID) {
	GroupID = InID;
}

void USPInterest::ReassignGroup(int32 InGroupID) {
	ASPGameState* GameState = ASPGameState::GetSPGameState(this);
	GameState->MissionManager->ReassignInterest(ID, GroupID, InGroupID);
}

int32 USPInterest::GetGroupID() const {
	return GroupID;
}

void USPInterest::SetID(int32 InID) {
	ID = InID;
}

int32 USPInterest::GetID() const {
	return ID;
}

void USPInterest::SetName(FString InName) {
	Name = InName;
}

void USPInterest::RenameInterest(FString InName) {
	ASPGameState* GameState = ASPGameState::GetSPGameState(this);
	GameState->MissionManager->RenameInterest(ID, GroupID, InName);
}

FString USPInterest::GetName() const {
	return Name;
}

void USPInterest::ToggleCull_Implementation(bool IsCulled) {}

void USPInterest::SetAttributes(int32 InID, int32 InGroupID, FString InName) {
	ID = InID;
	GroupID = InGroupID;
	SetName(InName);
}

void USPInterest::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) {
	ASPGameState* gameState = ASPGameState::GetSPGameState(this);
	FString GroupName = gameState->MissionManager->GetMissionName(GroupID);

	OutKeyVals.Add(TEXT("Group"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(GroupName)
		});

	OutKeyVals.Add(TEXT("Delete interest"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::DeleteInterestButton,
		.value = FText::FromString(TEXT("Delete interest")),
		});
}

void USPInterest::GetInteractionBoxTitle_Implementation(FText& OutTitle) {
	OutTitle = FText::FromString(Name);
}

void USPInterest::ProvideOnPlaced_Implementation() {
	ASPGameState* GameState = ASPGameState::GetSPGameState(this);
	GameState->MissionManager->MoveInterest(ID, GroupID);
}

void USPInterest::SetSelected(bool IsSelected) {

}