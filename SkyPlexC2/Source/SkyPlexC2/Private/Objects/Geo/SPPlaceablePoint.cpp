// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Geo/SPPlaceablePoint.h"

void ASPPlaceablePoint::ToggleClickMeMode_Implementation(bool IsClickMe) {
	InClickMeMode = IsClickMe;
}

void ASPPlaceablePoint::OnClick(UPrimitiveComponent* touchedComponent, FKey buttonPressed) {
	Super::OnClick(touchedComponent, buttonPressed);
	if (InClickMeMode) {
		OnClickMeClick.Broadcast();
	}
}