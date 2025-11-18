// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Interests/SPGeoFence.h"
#include "Objects/Geo/SPPolygon.h"
#include "Objects/Geo/SPPlaceablePoint.h"
#include "Core/State/SPGameState.h"
#include "Core/State/SPMissionManager.h"

EInterestType USPGeoFence::GetInterestType() const {
	return EInterestType::GeoFence;
}

void USPGeoFence::SetPolygon(ASPPolygon* InPolygon) {
	ISPInteractionInterface::Execute_SetLinkedProvider(InPolygon, TScriptInterface<USPInteractionInterface>(this));
	Polygon = InPolygon;
}

void USPGeoFence::SetName(FString InName) {
	Super::SetName(InName);
	ISPInteractionInterface::Execute_OnInteractionBoxTitleChanged(Polygon, FText::FromString(InName));
}

void USPGeoFence::SetSelected(bool IsSelected) {
	ASPPlaceablePoint* Point = Polygon->GetHead();
	if (IsSelected) {
		Point->Select();
	}
	else {
		Point->Deselect();
	}
}

ASPPolygon* USPGeoFence::GetPolygon() const {
	return Polygon;
}

void USPGeoFence::DestroySelf_Implementation() {
	ASPGameState* GameState = ASPGameState::GetSPGameState(this);
	GameState->MissionManager->RemoveInterest(ID, GroupID);

	Polygon->DestroyPolygon();
	MarkAsGarbage();
}

void USPGeoFence::ToggleCull_Implementation(bool IsCulled) {
	ISPInteractionInterface::Execute_ToggleCull(Polygon, IsCulled);
}