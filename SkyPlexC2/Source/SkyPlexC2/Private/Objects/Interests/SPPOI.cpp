// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Interests/SPPOI.h"
#include "Objects/Geo/SPPlaceablePoint.h"
#include "Core/State/SPGameState.h"
#include "Core/State/SPMissionManager.h"

EInterestType USPPOI::GetInterestType() const {
	return EInterestType::POI;
}

void USPPOI::SetPoint(ASPPlaceablePoint* InPoint) {
	InPoint->Execute_SetLinkedProvider(InPoint, TScriptInterface<USPInteractionInterface>(this));
	Point = InPoint;
}

void USPPOI::SetName(FString InName) {
	Super::SetName(InName);
	ISPInteractionInterface::Execute_OnInteractionBoxTitleChanged(Point, FText::FromString(InName));
}

ASPPlaceablePoint* USPPOI::GetPoint() const {
	return Point;
}

void USPPOI::SetSelected(bool IsSelected) {
	if (IsSelected) {
		Point->Select();
	}
	else {
		Point->Deselect();
	}
}

void USPPOI::DestroySelf_Implementation() {
	ASPGameState* GameState = ASPGameState::GetSPGameState(this);
	GameState->MissionManager->RemoveInterest(ID, GroupID);
	Point->Deselect();
	Point->Destroy();
	MarkAsGarbage();
}

void USPPOI::ToggleCull_Implementation(bool IsCulled) {
	ISPInteractionInterface::Execute_ToggleCull(Point, IsCulled);
}