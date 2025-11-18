// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Interests/SPTakeoffPoint.h"
#include "Objects/Geo/SPPlaceablePoint.h"
#include "Core/State/SPGameState.h"
#include "Core/State/SPMissionManager.h"

EInterestType USPTakeoffPoint::GetInterestType() const {
	return EInterestType::Takeoff;
}

void USPTakeoffPoint::SetPoint(ASPPlaceablePoint* InPoint) {
	ISPInteractionInterface::Execute_SetLinkedProvider(InPoint, TScriptInterface<USPInteractionInterface>(this));
	Point = InPoint;
}

void USPTakeoffPoint::SetName(FString InName) {
	Super::SetName(InName);
	ISPInteractionInterface::Execute_OnInteractionBoxTitleChanged(Point, FText::FromString(InName));
}

ASPPlaceablePoint* USPTakeoffPoint::GetPoint() const {
	return Point;
}

void USPTakeoffPoint::SetSelected(bool IsSelected) {
	if (IsSelected) {
		Point->Select();
	}
	else {
		Point->Deselect();
	}
}

void USPTakeoffPoint::DestroySelf_Implementation() {
	ASPGameState* GameState = ASPGameState::GetSPGameState(this);
	GameState->MissionManager->RemoveInterest(ID, GroupID);
	Point->Deselect();
	Point->Destroy();
	MarkAsGarbage();
}

void USPTakeoffPoint::ToggleCull_Implementation(bool IsCulled) {
	ISPInteractionInterface::Execute_ToggleCull(Point, IsCulled);
}