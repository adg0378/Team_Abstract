// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Interests/SPGeoFence.h"
#include "SPUtility.h"
#include "State/SPGameState.h"
#include "JsonObjectConverter.h"
#include "State/Missions/SPMissionManager.h"
#include "Objects/Geometry/PlaceablePoint.h"
#include "Objects/Geometry/Polygon.h"

EInterestType USPGeoFence::GetInterestType() const {
	return EInterestType::GeoFence;
}

void USPGeoFence::SetPolygon(APolygon* InPolygon) {
	IInteractionBoxProvider::Execute_SetLinkedProvider(InPolygon, TScriptInterface<UInteractionBoxProvider>(this));
	Polygon = InPolygon;
}

void USPGeoFence::SetName(FString InName) {
	Super::SetName(InName);
	IInteractionBoxProvider::Execute_OnInteractionBoxTitleChanged(Polygon, FText::FromString(InName));
}

void USPGeoFence::SetSelected(bool IsSelected) {
	APlaceablePoint* Point;
	Polygon->GetHead(Point);
	if (IsSelected) {
		Point->Select();
	}
	else {
		Point->Deselect();
	}
}

void USPGeoFence::GetPolygon(APolygon*& OutPolygon) const {
	OutPolygon = Polygon;
}

void USPGeoFence::DestroySelf_Implementation() {
	ASPGameState* GameState = USPUtility::GetSPGameState(this);
	GameState->MissionManagerRef->RemoveInterest(ID, GroupID);

	Polygon->DestroyPolygon();
	MarkAsGarbage();
}

void USPGeoFence::ToggleCull_Implementation(bool IsCulled, bool FromFlyTo) {
	IInteractionBoxProvider::Execute_ToggleCull(Polygon, IsCulled, FromFlyTo);
}

void USPGeoFence::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) {
	Super::GetInteractionBoxKeyVals_Implementation(OutKeyVals);

	OutKeyVals.Add(TEXT("GeoFence Parameters"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::GeoFenceParams,
		.value = FText::FromString("TBD")
		});
}

FGeoFenceParametersStruct USPGeoFence::GetParams() const {
	return Params;
}

FString USPGeoFence::GetSerializedParams() const {
	FString JsonString;

	if (!FJsonObjectConverter::UStructToJsonObjectString<FGeoFenceParametersStruct>(Params, JsonString)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to stringify GeoFence Params"));
		return FString("{}");
	}
	return JsonString;
}

void USPGeoFence::SetSerializedParams(const FString& ParamsJson) {
	FGeoFenceParametersStruct TempParams;

	if (!FJsonObjectConverter::JsonObjectStringToUStruct<FGeoFenceParametersStruct>(ParamsJson, &TempParams, 0, 0)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to parse geofence params"));
	}

	SetParams(TempParams);
}

void USPGeoFence::SetParams(UPARAM(ref) FGeoFenceParametersStruct& InParams, bool UpdateDB) {
	Params = InParams;
	Polygon->SetInclusive(Params.Inclusive);

	if (UpdateDB) {
		ASPGameState* GameState = USPUtility::GetSPGameState(this);
		GameState->MissionManagerRef->UpdateInterestParams(ID, GroupID);
	}
}
