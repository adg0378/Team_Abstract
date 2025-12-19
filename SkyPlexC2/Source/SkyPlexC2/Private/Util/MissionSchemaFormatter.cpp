// Copyright (c) 2025 Synetos Aerospace


#include "Util/MissionSchemaFormatter.h"
#include "Objects/Interests/Interest.h"
#include "Objects/Interests/SPTakeoffPoint.h"
#include "Objects/Interests/POI.h"
#include "Objects/Interests/AreaOfInterest.h"
#include "Objects/Interests/SPGeoFence.h"
#include "Objects/Geometry/PlaceablePoint.h"
#include "Objects/Geometry/Polygon.h"
#include "Util/CCSimWebsocketMessenger.h"
#include <CesiumGlobeAnchorComponent.h>

TArray<TSharedPtr<FJsonValue>> UMissionSchemaFormatter::MakeCoordinateArrayJson(const TArray<FVector>& Points) {
	TArray<TSharedPtr<FJsonValue>> PolygonPoints;
	for (const FVector& Location : Points) {
		TArray<TSharedPtr<FJsonValue>> PointsArray{
			MakeShared<FJsonValueNumber>(Location.Y),
			MakeShared<FJsonValueNumber>(Location.X),
		};
		PolygonPoints.Add(MakeShared<FJsonValueArray>(PointsArray));
	}
	return PolygonPoints;
}

TSharedPtr<FJsonObject> UMissionSchemaFormatter::MakeGeoFencesJson(const TArray<USPGeoFence*>& GeoFences) {
	TArray<TSharedPtr<FJsonValue>> GeoFenceItems;

	for (const USPGeoFence* GeoFence : GeoFences) {
		TSharedPtr<FJsonObject> GeoFenceItem = MakeShared<FJsonObject>();

		GeoFenceItem->SetBoolField(TEXT("inclusion"), !GeoFence->GetParams().Inclusive);

		APolygon* Polygon;
		GeoFence->GetPolygon(Polygon);

		TArray<FVector> Locations;
		Polygon->GetPointLocations(Locations);

		TArray<TSharedPtr<FJsonValue>> PolygonPoints = MakeCoordinateArrayJson(Locations);

		GeoFenceItem->SetArrayField(TEXT("polygon"), PolygonPoints);
		GeoFenceItems.Add(MakeShared<FJsonValueObject>(GeoFenceItem));
	}

	TSharedPtr<FJsonObject> GeoFenceObject = MakeShared<FJsonObject>();
	GeoFenceObject->SetArrayField(TEXT("polygons"), GeoFenceItems);
	return GeoFenceObject;
}

void UMissionSchemaFormatter::HandleAddTakeoffItem(
	TArray<TSharedPtr<FJsonValue>>& MissionItems,
	int32 JumpID,
	const USPTakeoffPoint* TakeoffPoint,
	TArray<TSharedPtr<FJsonValue>>& PlannedHomePosition
) {
	if (!TakeoffPoint) {
		return;
	}

	TSharedPtr<FJsonObject> TakeoffPointObject = MakeShared<FJsonObject>();
	
	APlaceablePoint* Point;
	TakeoffPoint->GetPoint(Point);
	FVector LonLatHeight = Point->cesiumGlobeAnchor->GetLongitudeLatitudeHeight();

	TakeoffPointObject->SetBoolField(TEXT("autoContinue"), true);
	TakeoffPointObject->SetNumberField(TEXT("command"), static_cast<uint8>(PX4Command::TAKEOFF));
	TakeoffPointObject->SetNumberField(TEXT("doJumpId"), JumpID);
	TakeoffPointObject->SetNumberField(TEXT("frame"), 3);

	TArray<TSharedPtr<FJsonValue>> Params;
	FSPTakeoffPointParams TakeoffParams = TakeoffPoint->GetParams();

	Params.Add(MakeShared<FJsonValueNumber>(0));
	Params.Add(MakeShared<FJsonValueNumber>(0));
	Params.Add(MakeShared<FJsonValueNumber>(0));
	Params.Add(MakeShared<FJsonValueNull>());
	Params.Add(MakeShared<FJsonValueNumber>(LonLatHeight.Y));
	Params.Add(MakeShared<FJsonValueNumber>(LonLatHeight.X));
	Params.Add(MakeShared<FJsonValueNumber>(TakeoffParams.Altitude));

	TakeoffPointObject->SetArrayField(TEXT("params"), Params);
	TakeoffPointObject->SetStringField(TEXT("type"), TEXT("SimpleItem"));

	PlannedHomePosition.Empty();
	PlannedHomePosition.Add(MakeShared<FJsonValueNumber>(LonLatHeight.Y));
	PlannedHomePosition.Add(MakeShared<FJsonValueNumber>(LonLatHeight.X));
	PlannedHomePosition.Add(MakeShared<FJsonValueNumber>(TakeoffParams.Altitude));

	MissionItems.Add(MakeShared<FJsonValueObject>(TakeoffPointObject));

	if (TakeoffPoint->GetParams().InitialSpeedMS > 0.0f) {
		HandleAddSpeedItemObject(MissionItems, JumpID + 1, TakeoffPoint->GetParams().InitialSpeedMS);
	}
}

void UMissionSchemaFormatter::HandleAddPOIItem(
	TArray<TSharedPtr<FJsonValue>>& MissionItems,
	int32 JumpID,
	const UPOI* POI
) {
	if (!POI) {
		return;
	}

	APlaceablePoint* Point;
	POI->GetPoint(Point);
	FVector LonLatHeight = Point->cesiumGlobeAnchor->GetLongitudeLatitudeHeight();

	TSharedPtr<FJsonObject> POIObject = MakeShared<FJsonObject>();

	POIObject->SetBoolField(TEXT("autoContinue"), true);
	POIObject->SetNumberField(TEXT("command"), static_cast<uint8>(PX4Command::WAYPOINT));
	POIObject->SetNumberField(TEXT("doJumpId"), JumpID);
	POIObject->SetNumberField(TEXT("frame"), 3);

	TArray<TSharedPtr<FJsonValue>> Params;

	Params.Add(MakeShared<FJsonValueNumber>(POI->GetParams().HoldTimeS));
	Params.Add(MakeShared<FJsonValueNumber>(0));
	Params.Add(MakeShared<FJsonValueNumber>(0));

	if (POI->GetParams().YawDeg == 400.0f) {
		Params.Add(MakeShared<FJsonValueNull>());
	}
	else {
		Params.Add(MakeShared<FJsonValueNumber>(POI->GetParams().YawDeg));
	}

	Params.Add(MakeShared<FJsonValueNumber>(LonLatHeight.Y));
	Params.Add(MakeShared<FJsonValueNumber>(LonLatHeight.X));
	Params.Add(MakeShared<FJsonValueNumber>(POI->GetParams().Altitude));

	POIObject->SetArrayField(TEXT("params"), Params);
	POIObject->SetStringField(TEXT("type"), TEXT("SimpleItem"));
	MissionItems.Add(MakeShared<FJsonValueObject>(POIObject));

	if (POI->GetParams().SpeedMS > 0.0f) {
		HandleAddSpeedItemObject(MissionItems, JumpID + 1, POI->GetParams().SpeedMS);
	}
}

void UMissionSchemaFormatter::HandleAddSimpleWaypointItem(
	TArray<TSharedPtr<FJsonValue>>& MissionItems,
	int32 JumpID,
	const FVector& LonLatAlt
) {
	TSharedPtr<FJsonObject> WaypointObject = MakeShared<FJsonObject>();

	WaypointObject->SetBoolField(TEXT("autoContinue"), true);
	WaypointObject->SetNumberField(TEXT("command"), static_cast<uint8>(PX4Command::WAYPOINT));
	WaypointObject->SetNumberField(TEXT("doJumpId"), JumpID);
	WaypointObject->SetNumberField(TEXT("frame"), 3);

	TArray<TSharedPtr<FJsonValue>> Params;

	Params.Add(MakeShared<FJsonValueNumber>(0));
	Params.Add(MakeShared<FJsonValueNumber>(0));
	Params.Add(MakeShared<FJsonValueNumber>(0));
	Params.Add(MakeShared<FJsonValueNull>());
	Params.Add(MakeShared<FJsonValueNumber>(LonLatAlt.Y));
	Params.Add(MakeShared<FJsonValueNumber>(LonLatAlt.X));
	Params.Add(MakeShared<FJsonValueNumber>(LonLatAlt.Z));

	WaypointObject->SetArrayField(TEXT("params"), Params);
	WaypointObject->SetStringField(TEXT("type"), TEXT("SimpleItem"));
	MissionItems.Add(MakeShared<FJsonValueObject>(WaypointObject));
}

void UMissionSchemaFormatter::HandleAddSpeedItemObject(TArray<TSharedPtr<FJsonValue>>& MissionItems, int32 JumpID, float SpeedMS) {
	TSharedPtr<FJsonObject> SpeedItemObject = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> SpeedItemParams;

	SpeedItemObject->SetBoolField(TEXT("autoContinue"), true);
	SpeedItemObject->SetNumberField(TEXT("command"), static_cast<uint8>(PX4Command::CHANGE_SPEED));
	SpeedItemObject->SetNumberField(TEXT("doJumpId"), JumpID);
	SpeedItemObject->SetNumberField(TEXT("frame"), 2);

	SpeedItemParams.Add(MakeShared<FJsonValueNumber>(1));
	SpeedItemParams.Add(MakeShared<FJsonValueNumber>(SpeedMS));
	SpeedItemParams.Add(MakeShared<FJsonValueNumber>(-1));
	SpeedItemParams.Add(MakeShared<FJsonValueNumber>(0));
	SpeedItemParams.Add(MakeShared<FJsonValueNumber>(0));
	SpeedItemParams.Add(MakeShared<FJsonValueNumber>(0));
	SpeedItemParams.Add(MakeShared<FJsonValueNumber>(0));

	SpeedItemObject->SetArrayField(TEXT("params"), SpeedItemParams);
	SpeedItemObject->SetStringField(TEXT("type"), TEXT("SimpleItem"));
	MissionItems.Add(MakeShared<FJsonValueObject>(SpeedItemObject));
}

void UMissionSchemaFormatter::HandleAddReturnToLaunchObject(TArray<TSharedPtr<FJsonValue>>& MissionItems, int32 JumpID) {
	TSharedPtr<FJsonObject> ReturnToLaunchObject = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> ReturnToLaunchParams{
		MakeShared<FJsonValueNumber>(0),
		MakeShared<FJsonValueNumber>(0),
		MakeShared<FJsonValueNumber>(0),
		MakeShared<FJsonValueNumber>(0),
		MakeShared<FJsonValueNumber>(0),
		MakeShared<FJsonValueNumber>(0),
		MakeShared<FJsonValueNumber>(0),
	};

	ReturnToLaunchObject->SetBoolField(TEXT("autoContinue"), true);
	ReturnToLaunchObject->SetNumberField(TEXT("command"), static_cast<uint8>(PX4Command::RETURN_TO_LAUNCH));
	ReturnToLaunchObject->SetNumberField(TEXT("doJumpId"), JumpID);
	ReturnToLaunchObject->SetNumberField(TEXT("frame"), 2);
	ReturnToLaunchObject->SetArrayField(TEXT("params"), ReturnToLaunchParams);
	ReturnToLaunchObject->SetStringField(TEXT("type"), TEXT("SimpleItem"));
	MissionItems.Add(MakeShared<FJsonValueObject>(ReturnToLaunchObject));
}

TSharedPtr<FJsonObject> UMissionSchemaFormatter::MakeTransectStyleComplexItem(int32 JumpID, const UAreaOfInterest* AOI, int32& NestedItemJumpIDOffset) {
	TSharedPtr<FJsonObject> TransectItem = MakeShared<FJsonObject>();

	if (!AOI) {
		return TransectItem;
	}

	FAOIParametersStruct AOIParams = AOI->GetParams();

	TransectItem->SetBoolField(TEXT("cameraTriggerInTurnAround"), AOIParams.ImagesInTurnarounds);
	TransectItem->SetBoolField(TEXT("hoverAndCapture"), false);

	TArray<FVector> TransectPoints = AOI->GeneratePolygonTransectPoints();
	
	TArray<TSharedPtr<FJsonValue>> TransectMissionItems;
	float Alt = AOIParams.Altitude;

	for (const auto& Point : TransectPoints) {
		FVector LonLatAlt = FVector(Point.X, Point.Y, Alt);
		HandleAddSimpleWaypointItem(TransectMissionItems, JumpID + TransectMissionItems.Num(), LonLatAlt);
	}

	NestedItemJumpIDOffset += TransectPoints.Num() - 1;
	
	TransectItem->SetArrayField(TEXT("items"), TransectMissionItems);
	TransectItem->SetBoolField(TEXT("refly90Degrees"), AOIParams.ReflyAt90DegOffset);
	TransectItem->SetNumberField(TEXT("turnAroundDistance"), AOIParams.TurnaroundDistM);

	TArray<TSharedPtr<FJsonValue>> VisualTransectPoints = MakeCoordinateArrayJson(TransectPoints);
	TransectItem->SetArrayField(TEXT("visualTransectPoints"), VisualTransectPoints);

	return TransectItem;
}

void UMissionSchemaFormatter::HandleAddSurveyItem(
	TArray<TSharedPtr<FJsonValue>>& MissionItems,
	int32 JumpID,
	const UAreaOfInterest* AOI,
	int32& NestedItemJumpIDOffset
) {
	TSharedPtr<FJsonObject> SurveyObject = MakeShared<FJsonObject>();

	if (!AOI) {
		return;
	}

	FAOIParametersStruct AOIParams = AOI->GetParams();

	SurveyObject->SetObjectField(TEXT("transectStyleComplexItem"), MakeTransectStyleComplexItem(JumpID, AOI, NestedItemJumpIDOffset));
	SurveyObject->SetNumberField(TEXT("angle"), AOIParams.TransectsAngle);
	SurveyObject->SetStringField(TEXT("complexItemType"), TEXT("survey"));
	SurveyObject->SetNumberField(TEXT("entryLocation"), AOIParams.EntryPoint);
	SurveyObject->SetBoolField(TEXT("flyAlternateTransects"), false);

	APolygon* Polygon;
	AOI->GetPolygon(Polygon);

	TArray<FVector> Locations;
	Polygon->GetPointLocations(Locations);

	TArray<TSharedPtr<FJsonValue>> PolygonPoints = MakeCoordinateArrayJson(Locations);

	SurveyObject->SetArrayField(TEXT("polygon"), PolygonPoints);
	SurveyObject->SetStringField(TEXT("type"), TEXT("ComplexItem"));
	MissionItems.Add(MakeShared<FJsonValueObject>(SurveyObject));
}

TSharedPtr<FJsonObject> UMissionSchemaFormatter::MakeMissionJson(const USPTakeoffPoint* Takeoff, const TArray<const UInterest*>& OrderedInterests) {
	TArray<TSharedPtr<FJsonValue>> MissionItems;
	TArray<TSharedPtr<FJsonValue>> PlannedHomePosition;

	HandleAddTakeoffItem(MissionItems, MissionItems.Num() + 1, Cast<USPTakeoffPoint>(Takeoff), PlannedHomePosition);

	int32 NestedItemJumpIDOffset = 0;
	for (int i = 0; i < OrderedInterests.Num(); i++) {
		const UInterest* Interest = OrderedInterests[i];
		EInterestType Type = Interest->GetInterestType();

		switch (Type) {
			case EInterestType::POI:
				HandleAddPOIItem(MissionItems, MissionItems.Num() + 1 + NestedItemJumpIDOffset, Cast<UPOI>(Interest));
				break;
			case EInterestType::AOI:
				HandleAddSurveyItem(MissionItems, MissionItems.Num() + 1 + NestedItemJumpIDOffset, Cast<UAreaOfInterest>(Interest), NestedItemJumpIDOffset);
				break;
			default:
				break;
		}
	}

	HandleAddReturnToLaunchObject(MissionItems, MissionItems.Num() + 1 + NestedItemJumpIDOffset);

	TSharedPtr<FJsonObject> MissionObject = MakeShared<FJsonObject>();
	MissionObject->SetArrayField(TEXT("items"), MissionItems);
	MissionObject->SetArrayField(TEXT("plannedHomePosition"), PlannedHomePosition);

	return MissionObject;
}

TSharedPtr<FJsonObject> UMissionSchemaFormatter::MakeRallyPointsJson() {
	TArray<TSharedPtr<FJsonValue>> RallyPoints;
	TSharedPtr<FJsonObject> RallyPointsObject = MakeShared<FJsonObject>();
	RallyPointsObject->SetArrayField(TEXT("points"), RallyPoints);
	return RallyPointsObject;
}

TSharedPtr<FJsonObject> UMissionSchemaFormatter::FormatMissionAsJson(const FSPInterestStruct* MissionInterests, const TArray<int32>& InterestOrder) {
	TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();

	if (!MissionInterests) {
		return RootObject;
	}

	USPTakeoffPoint* Takeoff = nullptr;
	TArray<const UInterest*> OrderedInterests;
	TArray<USPGeoFence*> GeoFences;

	Takeoff = MissionInterests->TakeoffPoint;

	for (const TPair<int32, USPGeoFence*> GeoFencePair : MissionInterests->GeoFences) {
		GeoFences.Add(GeoFencePair.Value);
	}

	for (const int32& ID : InterestOrder) {
		UInterest* Interest = MissionInterests->Interests.Find(ID)->Get();
		if (Interest) {
			EInterestType Type = Interest->GetInterestType();
			if (Type == EInterestType::POI || Type == EInterestType::AOI) {
				OrderedInterests.Add(Interest);
			}
		}
	}

	TSharedPtr<FJsonObject> GeoFenceObject = MakeGeoFencesJson(GeoFences);
	TSharedPtr<FJsonObject> MissionObject = MakeMissionJson(Takeoff, OrderedInterests);
	TSharedPtr<FJsonObject> RallyPointsObject = MakeRallyPointsJson();

	RootObject->SetObjectField(TEXT("geoFence"), GeoFenceObject);
	RootObject->SetObjectField(TEXT("mission"), MissionObject);
	RootObject->SetObjectField(TEXT("rallyPoints"), RallyPointsObject);

	return RootObject;
}
