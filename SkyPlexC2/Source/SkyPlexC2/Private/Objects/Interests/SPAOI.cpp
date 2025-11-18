// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Interests/SPAOI.h"
#include "Objects/Geo/SPPolygon.h"
#include "Objects/Geo/SPPlaceablePoint.h"
#include "Core/State/SPGameState.h"
#include "Core/State/SPMissionManager.h"
#include "CesiumGeoreference.h"
#include "Util/SPGeoUtility.h"

EInterestType USPAOI::GetInterestType() const {
	return EInterestType::AOI;
}

void USPAOI::SetPolygon(ASPPolygon* InPolygon) {
	ISPInteractionInterface::Execute_SetLinkedProvider(InPolygon, TScriptInterface<USPInteractionInterface>(this));
	Polygon = InPolygon;
}

void USPAOI::SetName(FString InName) {
	Super::SetName(InName);
	ISPInteractionInterface::Execute_OnInteractionBoxTitleChanged(Polygon, FText::FromString(InName));
}

void USPAOI::SetSelected(bool IsSelected) {
	ASPPlaceablePoint* Point = Polygon->GetHead();
	if (IsSelected) {
		Point->Select();
	}
	else {
		Point->Deselect();
	}
}

ASPPolygon* USPAOI::GetPolygon() const {
	return Polygon;
}

void USPAOI::DestroySelf_Implementation() {
	ASPGameState* GameState = ASPGameState::GetSPGameState(this);
	GameState->MissionManager->RemoveInterest(ID, GroupID);

	Polygon->DestroyPolygon();
	MarkAsGarbage();
}

void USPAOI::ToggleCull_Implementation(bool IsCulled) {
	ISPInteractionInterface::Execute_ToggleCull(Polygon, IsCulled);
}

TArray<FVector> USPAOI::GeneratePolygonTransectPoints() const {
	FVector OriginLonLatHeight = Polygon->GetCenterpoint();
	TArray<FVector> LonLatHeights;
	Polygon->GetPointLocations(LonLatHeights);

	ASPGameState* GameState = ASPGameState::GetSPGameState(this);
	ACesiumGeoreference* Georeference = GameState->CesiumGeoreference;

	TArray<FVector> LocalPoints = USPGeoUtility::LatLonToLocal(Georeference, OriginLonLatHeight, LonLatHeights);

	if (Params.TransectsAngle != 0.0f) {
		LocalPoints = USPGeoUtility::RotatePoints(LocalPoints, -Params.TransectsAngle);
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
		TArray<FVector2D> Intersections = USPGeoUtility::GetPolygonLineIntersections(LocalPoints, Y);

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
		LineSegmentPoints = USPGeoUtility::RotatePoints(LineSegmentPoints, Params.TransectsAngle);
	}

	return USPGeoUtility::LocalToLatLon(Georeference, OriginLonLatHeight, LineSegmentPoints);
}
