// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Geometry/PlaceablePoint.h"

void APlaceablePoint::ToggleClickMeMode_Implementation(bool IsClickMe) {
	InClickMeMode = IsClickMe;
}

void APlaceablePoint::OnClick(UPrimitiveComponent* touchedComponent, FKey buttonPressed) {
	Super::OnClick(touchedComponent, buttonPressed);
	if (InClickMeMode) {
		OnClickMeClick.Broadcast();
	}
}