// Copyright (c) 2025 Synetos Aerospace

#include "Objects/Interests/AreaOfInterest.h"
#include "SPUtility.h"
#include "State/SPGameState.h"
#include "Objects/Geometry/PlaceablePoint.h"
#include "GeographicUtility.h"
#include "JsonObjectConverter.h"
#include "State/Missions/SPMissionManager.h"
#include "Objects/Geometry/Polygon.h"

EInterestType UAreaOfInterest::GetInterestType() const {
	return EInterestType::AOI;
}

void UAreaOfInterest::SetPolygon(APolygon* InPolygon) {
	IInteractionBoxProvider::Execute_SetLinkedProvider(InPolygon, TScriptInterface<UInteractionBoxProvider>(this));
	Polygon = InPolygon;
}

void UAreaOfInterest::SetName(FString InName) {
	Super::SetName(InName);
	IInteractionBoxProvider::Execute_OnInteractionBoxTitleChanged(Polygon, FText::FromString(InName));
}

void UAreaOfInterest::SetSelected(bool IsSelected) {
	APlaceablePoint* Point;
	Polygon->GetHead(Point);
	if (IsSelected) {
		Point->Select();
	}
	else {
		Point->Deselect();
	}
}

void UAreaOfInterest::GetPolygon(APolygon*& OutPolygon) const {
	OutPolygon = Polygon;
}

void UAreaOfInterest::DestroySelf_Implementation() {
	ASPGameState* GameState = USPUtility::GetSPGameState(this);
	GameState->MissionManagerRef->RemoveInterest(ID, GroupID);

	Polygon->DestroyPolygon();
	MarkAsGarbage();
}

void UAreaOfInterest::ToggleCull_Implementation(bool IsCulled, bool FromFlyTo) {
	IInteractionBoxProvider::Execute_ToggleCull(Polygon, IsCulled, FromFlyTo);
}

TArray<FVector> UAreaOfInterest::GeneratePolygonTransectPoints() const {
	FVector OriginLonLatHeight = Polygon->GetCenterpoint();
	TArray<FVector> LonLatHeights;
	Polygon->GetPointLocations(LonLatHeights);

	ASPGameState* GameState = USPUtility::GetSPGameState(this);
	ACesiumGeoreference* Georeference = GameState->CesiumGeoreference;

	TArray<FVector> LocalPoints = UGeographicUtility::LatLonToLocal(Georeference, OriginLonLatHeight, LonLatHeights);

	if (Params.TransectsAngle != 0.0f) {
		LocalPoints = UGeographicUtility::RotatePoints(LocalPoints, -Params.TransectsAngle);
	}

	double MinY = DBL_MAX;
	double MaxY = DBL_MIN;
	for (const auto& Point : LocalPoints) {
		if (Point.Y < MinY) {
			MinY = Point.Y;
		}
		if (Point.Y > MaxY) {
			MaxY = Point.Y;
		}
	}

	TArray<FVector> LineSegmentPoints;

	double Y = MinY;
	double SpacingM = Params.SpacingM;
	double TurnaroundDistanceM = Params.TurnaroundDistM;
	bool SwapDirection = false;
	while (Y <= MaxY) {
		TArray<FVector2D> Intersections = UGeographicUtility::GetPolygonLineIntersections(LocalPoints, Y);

		int N = Intersections.Num();
		if (N < 2) {
			UE_LOG(LogTemp, Error, TEXT("Found a line segment with less than 2 intersections"));
		}
		else {
			FVector2D P1 = Intersections[0];
			P1.X -= TurnaroundDistanceM;
			FVector2D P2 = Intersections[N - 1];
			P2.X += TurnaroundDistanceM;

			if (SwapDirection) {
				LineSegmentPoints.Add(FVector(P2.X, P2.Y, 0.0f));
				LineSegmentPoints.Add(FVector(P1.X, P1.Y, 0.0f));
			}
			else {
				LineSegmentPoints.Add(FVector(P1.X, P1.Y, 0.0f));
				LineSegmentPoints.Add(FVector(P2.X, P2.Y, 0.0f));
			}
		}
		Y += SpacingM;
		SwapDirection = !SwapDirection;
	}

	if (Params.TransectsAngle != 0.0f) {
		LineSegmentPoints = UGeographicUtility::RotatePoints(LineSegmentPoints, Params.TransectsAngle);
	}

	return UGeographicUtility::LocalToLatLon(Georeference, OriginLonLatHeight, LineSegmentPoints);
}

void UAreaOfInterest::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) {
	Super::GetInteractionBoxKeyVals_Implementation(OutKeyVals);

	OutKeyVals.Add(TEXT("AOI Parameters"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::AOIParams,
		.value = FText::FromString("TBD")
		});
}

FAOIParametersStruct UAreaOfInterest::GetParams() const {
	return Params;
}

FString UAreaOfInterest::GetSerializedParams() const {
	FString JsonString;

	if (!FJsonObjectConverter::UStructToJsonObjectString<FAOIParametersStruct>(Params, JsonString)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to stringify AOI Params"));
		return FString("{}");
	}
	return JsonString;
}

void UAreaOfInterest::SetSerializedParams(const FString& ParamsJson) {
	FAOIParametersStruct TempParams;

	if (!FJsonObjectConverter::JsonObjectStringToUStruct<FAOIParametersStruct>(ParamsJson, &TempParams, 0, 0)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to parse AOI params: json='%s'"), *ParamsJson);
	}

	SetParams(TempParams);
}

void UAreaOfInterest::SetParams(FAOIParametersStruct& InParams, bool UpdateDB) {

	bool Refresh = InParams.SpacingM != Params.SpacingM
		|| InParams.TransectsAngle != Params.TransectsAngle
		|| InParams.TurnaroundDistM != Params.TurnaroundDistM;
	
	Params = InParams;

	if (Refresh || UpdateDB) {
		ASPGameState* GameState = USPUtility::GetSPGameState(this);
		if (Refresh) {
			GameState->MissionManagerRef->RefreshMissionLine(GroupID);
		}
		else {
			GameState->MissionManagerRef->UpdateInterestParams(ID, GroupID);
		}
	}
}


