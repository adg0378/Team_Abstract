// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Interests/SPTakeoffPoint.h"
#include "Objects/Geometry/PlaceablePoint.h"
#include "Objects/InteractionBoxProvider.h"
#include "State/SPGameState.h"
#include "JsonObjectConverter.h"
#include "State/Missions/SPMissionManager.h"
#include "SPUtility.h"


EInterestType USPTakeoffPoint::GetInterestType() const {
	return EInterestType::Takeoff;
}

void USPTakeoffPoint::SetPoint(APlaceablePoint* InPoint) {
	IInteractionBoxProvider::Execute_SetLinkedProvider(InPoint, TScriptInterface<UInteractionBoxProvider>(this));
	Point = InPoint;
}

void USPTakeoffPoint::SetName(FString InName) {
	Super::SetName(InName);
	IInteractionBoxProvider::Execute_OnInteractionBoxTitleChanged(Point, FText::FromString(InName));
}

void USPTakeoffPoint::GetPoint(APlaceablePoint*& OutPoint) const {
	OutPoint = Point;
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
	ASPGameState* GameState = USPUtility::GetSPGameState(this);
	GameState->MissionManagerRef->RemoveInterest(ID, GroupID);
	Point->Deselect();
	Point->Destroy();
	MarkAsGarbage();
}

void USPTakeoffPoint::ToggleCull_Implementation(bool IsCulled, bool FromFlyTo) {
	IInteractionBoxProvider::Execute_ToggleCull(Point, IsCulled, FromFlyTo);
}

void USPTakeoffPoint::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) {
	Super::GetInteractionBoxKeyVals_Implementation(OutKeyVals);

	OutKeyVals.Add(TEXT("Takeoff Point Parameters"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::TakeoffParams,
		.value = FText::FromString("TBD")
		});
}

FSPTakeoffPointParams USPTakeoffPoint::GetParams() const {
	return Params;
}

FString USPTakeoffPoint::GetSerializedParams() const {
	FString JsonString;

	if (!FJsonObjectConverter::UStructToJsonObjectString<FSPTakeoffPointParams>(Params, JsonString)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to stringify Takeoff Params"));
		return FString("{}");
	}
	return JsonString;
}

void USPTakeoffPoint::SetSerializedParams(const FString& ParamsJson) {
	FSPTakeoffPointParams TempParams;

	if (!FJsonObjectConverter::JsonObjectStringToUStruct<FSPTakeoffPointParams>(ParamsJson, &TempParams, 0, 0)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to parse takeoff params"));
	}

	SetParams(TempParams);
}

void USPTakeoffPoint::SetParams(FSPTakeoffPointParams& InParams, bool UpdateDB) {
	Params = InParams;

	if (UpdateDB) {
		ASPGameState* GameState = USPUtility::GetSPGameState(this);
		GameState->MissionManagerRef->UpdateInterestParams(ID, GroupID);
	}
}
