// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Interests/Interest.h"
#include "SPUtility.h"
#include "State/SPGameState.h"
#include "State/Missions/SPMissionManager.h"

EInterestType UInterest::GetInterestType() const {
	return EInterestType::Unknown;
}

void UInterest::SetGroupID(int32 InID) {
	GroupID = InID;
}

void UInterest::ReassignGroup(int32 InGroupID) {
	ASPGameState* GameState = USPUtility::GetSPGameState(this);
	GameState->MissionManagerRef->ReassignInterest(ID, GroupID, InGroupID);
}

void UInterest::GetGroupID(int32& OutID) const {
	OutID = GroupID;
}

void UInterest::SetID(int32 InID) {
	ID = InID;
}

int32 UInterest::GetID() const {
	return ID;
}

void UInterest::SetName(FString InName) {
	Name = InName;
}

void UInterest::RenameInterest(FString InName) {
	ASPGameState* GameState = USPUtility::GetSPGameState(this);
	GameState->MissionManagerRef->RenameInterest(ID, GroupID, InName);
}

void UInterest::GetName(FString& OutName) const {
	OutName = Name;
}

void UInterest::ToggleCull_Implementation(bool IsCulled, bool FromFlyTo) {}

void UInterest::SetAttributes(int32 InID, int32 InGroupID, FString InName) {
	ID = InID;
	GroupID = InGroupID;
	SetName(InName);
}

void UInterest::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) {
	ASPGameState* gameState = USPUtility::GetSPGameState(this);
	FString GroupName = gameState->MissionManagerRef->GetMissionName(GroupID);

	OutKeyVals.Add(TEXT("Group"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(GroupName)
	});

	OutKeyVals.Add(TEXT("Delete interest"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::DeleteInterestButton,
		.value = FText::FromString(TEXT("Delete interest")),
	});
}

void UInterest::GetInteractionBoxTitle_Implementation(FText& OutTitle) {
	OutTitle = FText::FromString(Name);
}

void UInterest::ProvideOnPlaced_Implementation() {
	ASPGameState* GameState = USPUtility::GetSPGameState(this);
	GameState->MissionManagerRef->MoveInterest(ID, GroupID);
}

void UInterest::SetSelected(bool IsSelected) {}

FString UInterest::GetSerializedParams() const {
	return FString("{}");
}

void UInterest::SetSerializedParams(const FString& ParamsJson) {}
