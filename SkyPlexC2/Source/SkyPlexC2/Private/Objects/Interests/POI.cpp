// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Interests/POI.h"
#include "Objects/InteractionBoxProvider.h"
#include "State/SPGameState.h"
#include "JsonObjectConverter.h"
#include "State/Missions/SPMissionManager.h"
#include "SPUtility.h"

EInterestType UPOI::GetInterestType() const {
	return EInterestType::POI;
}

void UPOI::SetPoint(APlaceablePoint* InPoint) {
	InPoint->Execute_SetLinkedProvider(InPoint, TScriptInterface<UInteractionBoxProvider>(this));
	Point = InPoint;
}

void UPOI::SetName(FString InName) {
	Super::SetName(InName);
	IInteractionBoxProvider::Execute_OnInteractionBoxTitleChanged(Point, FText::FromString(InName));
}

void UPOI::GetPoint(APlaceablePoint*& OutPoint) const {
	OutPoint = Point;
}

void UPOI::SetSelected(bool IsSelected) {
	if (IsSelected) {
		Point->Select();
	}
	else {
		Point->Deselect();
	}
}

void UPOI::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) {
	Super::GetInteractionBoxKeyVals_Implementation(OutKeyVals);

	OutKeyVals.Add(TEXT("POI Parameters"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::POIParams,
		.value = FText::FromString("TBD")
	});
}

void UPOI::DestroySelf_Implementation() {
	ASPGameState* GameState = USPUtility::GetSPGameState(this);
	GameState->MissionManagerRef->RemoveInterest(ID, GroupID);
	Point->Deselect();
	Point->Destroy();
	MarkAsGarbage();
}

void UPOI::ToggleCull_Implementation(bool IsCulled, bool FromFlyTo) {
	IInteractionBoxProvider::Execute_ToggleCull(Point, IsCulled, FromFlyTo);
}

FPOIParametersStruct UPOI::GetParams() const {
	return Params;
}

FString UPOI::GetSerializedParams() const {
	FString JsonString;

	if (!FJsonObjectConverter::UStructToJsonObjectString<FPOIParametersStruct>(Params, JsonString)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to stringify POI Params"));
		return FString("{}");
	}
	return JsonString;
}

void UPOI::SetSerializedParams(const FString& ParamsJson) {
	FPOIParametersStruct TempParams;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct<FPOIParametersStruct>(ParamsJson, &TempParams, 0, 0)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to parse POI params"));
	}

	SetParams(TempParams);
}

void UPOI::SetParams(FPOIParametersStruct& InParams, bool UpdateDB) {
	Params = InParams;

	if (UpdateDB) {
		ASPGameState* GameState = USPUtility::GetSPGameState(this);
		GameState->MissionManagerRef->UpdateInterestParams(ID, GroupID);
	}
}