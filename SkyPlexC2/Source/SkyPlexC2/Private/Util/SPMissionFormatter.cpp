// Copyright (c) 2025 Synetos Aerospace


#include "Util/SPMissionFormatter.h"
#include "Objects/Geo/SPPolygon.h"
#include "Objects/Geo/SPPlaceablePoint.h"
#include "Objects/Interests/SPAOI.h"
#include "Objects/Interests/SPPOI.h"
#include "Objects/Interests/SPGeoFence.h"
#include "Objects/Interests/SPTakeoffPoint.h"
#include "Objects/Drones/SPCCSimMessenger.h"
#include "Core/State/SPMissionManager.h"


TArray<TSharedPtr<FJsonValue>> USPMissionFormatter::MakeCoordinateArrayJson(const TArray<FVector>& Points) {
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

TSharedPtr<FJsonObject> USPMissionFormatter::MakeGeoFencesJson(const TArray<USPGeoFence*>& GeoFences) {
	TArray<TSharedPtr<FJsonValue>> GeoFenceItems;

	for (const USPGeoFence* GeoFence : GeoFences) {
		TSharedPtr<FJsonObject> GeoFenceItem = MakeShared<FJsonObject>();

		GeoFenceItem->SetBoolField(TEXT("inclusion"), false);

		ASPPolygon* Polygon = GeoFence->GetPolygon();

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

void USPMissionFormatter::HandleAddTakeoffItem(
	TArray<TSharedPtr<FJsonValue>>& MissionItems,
	int32 JumpID,
	const USPTakeoffPoint* TakeoffPoint,
	TArray<TSharedPtr<FJsonValue>>& PlannedHomePosition
) {
	if (!TakeoffPoint) {
		return;
	}

	TSharedPtr<FJsonObject> TakeoffPointObject = MakeShared<FJsonObject>();

	ASPPlaceablePoint* Point = TakeoffPoint->GetPoint();
	FVector LonLatHeight = Point->GetLongitudeLatitudeHeight();

	TakeoffPointObject->SetBoolField(TEXT("autoContinue"), true);
	TakeoffPointObject->SetNumberField(TEXT("command"), static_cast<uint8>(PX4Command::TAKEOFF));
	TakeoffPointObject->SetNumberField(TEXT("doJumpId"), JumpID);
	TakeoffPointObject->SetNumberField(TEXT("frame"), 3);

	TArray<TSharedPtr<FJsonValue>> Params;

	Params.Add(MakeShared<FJsonValueNumber>(0));
	Params.Add(MakeShared<FJsonValueNumber>(0));
	Params.Add(MakeShared<FJsonValueNumber>(0));
	Params.Add(MakeShared<FJsonValueNull>());
	Params.Add(MakeShared<FJsonValueNumber>(LonLatHeight.Y));
	Params.Add(MakeShared<FJsonValueNumber>(LonLatHeight.X));
	Params.Add(MakeShared<FJsonValueNumber>(TakeoffPoint->Params.Altitude));

	TakeoffPointObject->SetArrayField(TEXT("params"), Params);
	TakeoffPointObject->SetStringField(TEXT("type"), TEXT("SimpleItem"));

	PlannedHomePosition.Empty();
	PlannedHomePosition.Add(MakeShared<FJsonValueNumber>(LonLatHeight.Y));
	PlannedHomePosition.Add(MakeShared<FJsonValueNumber>(LonLatHeight.X));
	PlannedHomePosition.Add(MakeShared<FJsonValueNumber>(TakeoffPoint->Params.Altitude));

	MissionItems.Add(MakeShared<FJsonValueObject>(TakeoffPointObject));
}

void USPMissionFormatter::HandleAddPOIItem(
	TArray<TSharedPtr<FJsonValue>>& MissionItems,
	int32 JumpID,
	const USPPOI* POI
) {
	if (!POI) {
		return;
	}

	ASPPlaceablePoint* Point = POI->GetPoint();
	FVector LonLatHeight = Point->GetLongitudeLatitudeHeight();

	TSharedPtr<FJsonObject> POIObject = MakeShared<FJsonObject>();

	POIObject->SetBoolField(TEXT("autoContinue"), true);
	POIObject->SetNumberField(TEXT("command"), static_cast<uint8>(PX4Command::WAYPOINT));
	POIObject->SetNumberField(TEXT("doJumpId"), JumpID);
	POIObject->SetNumberField(TEXT("frame"), 3);

	TArray<TSharedPtr<FJsonValue>> Params;

	Params.Add(MakeShared<FJsonValueNumber>(POI->Params.HoldTimeS));
	Params.Add(MakeShared<FJsonValueNumber>(0));
	Params.Add(MakeShared<FJsonValueNumber>(0));

	if (POI->Params.YawDeg == 400.0f) {
		Params.Add(MakeShared<FJsonValueNull>());
	}
	else {
		Params.Add(MakeShared<FJsonValueNumber>(POI->Params.YawDeg));
	}

	Params.Add(MakeShared<FJsonValueNumber>(LonLatHeight.Y));
	Params.Add(MakeShared<FJsonValueNumber>(LonLatHeight.X));
	Params.Add(MakeShared<FJsonValueNumber>(POI->Params.Altitude));

	POIObject->SetArrayField(TEXT("params"), Params);
	POIObject->SetStringField(TEXT("type"), TEXT("SimpleItem"));
	MissionItems.Add(MakeShared<FJsonValueObject>(POIObject));

	if (POI->Params.SpeedMS > 0.0f) {
		HandleAddSpeedItemObject(MissionItems, JumpID + 1, POI->Params.SpeedMS);
	}
}

void USPMissionFormatter::HandleAddSimpleWaypointItem(
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

void USPMissionFormatter::HandleAddSpeedItemObject(TArray<TSharedPtr<FJsonValue>>& MissionItems, int32 JumpID, float SpeedMS) {
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

void USPMissionFormatter::HandleAddReturnToLaunchObject(TArray<TSharedPtr<FJsonValue>>& MissionItems, int32 JumpID) {
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

TSharedPtr<FJsonObject> USPMissionFormatter::MakeTransectStyleComplexItem(int32 JumpID, const USPAOI* AOI, int32& NestedItemJumpIDOffset) {
	TSharedPtr<FJsonObject> TransectItem = MakeShared<FJsonObject>();

	if (!AOI) {
		return TransectItem;
	}

	TransectItem->SetBoolField(TEXT("cameraTriggerInTurnAround"), AOI->Params.ImagesInTurnarounds);
	TransectItem->SetBoolField(TEXT("hoverAndCapture"), false);

	TArray<FVector> TransectPoints = AOI->GeneratePolygonTransectPoints();

	TArray<TSharedPtr<FJsonValue>> TransectMissionItems;
	float Alt = AOI->Params.Altitude;

	for (const auto& Point : TransectPoints) {
		FVector LonLatAlt = FVector(Point.X, Point.Y, Alt);
		HandleAddSimpleWaypointItem(TransectMissionItems, JumpID + TransectMissionItems.Num(), LonLatAlt);
	}

	NestedItemJumpIDOffset += TransectPoints.Num() - 1;

	TransectItem->SetArrayField(TEXT("items"), TransectMissionItems);
	TransectItem->SetBoolField(TEXT("refly90Degrees"), AOI->Params.ReflyAt90DegOffset);
	TransectItem->SetNumberField(TEXT("turnAroundDistance"), AOI->Params.TurnaroundDistM);

	TArray<TSharedPtr<FJsonValue>> VisualTransectPoints = MakeCoordinateArrayJson(TransectPoints);
	TransectItem->SetArrayField(TEXT("visualTransectPoints"), VisualTransectPoints);

	return TransectItem;
}

void USPMissionFormatter::HandleAddSurveyItem(
	TArray<TSharedPtr<FJsonValue>>& MissionItems,
	int32 JumpID,
	const USPAOI* AOI,
	int32& NestedItemJumpIDOffset
) {
	TSharedPtr<FJsonObject> SurveyObject = MakeShared<FJsonObject>();

	if (!AOI) {
		return;
	}

	SurveyObject->SetObjectField(TEXT("transectStyleComplexItem"), MakeTransectStyleComplexItem(JumpID, AOI, NestedItemJumpIDOffset));
	SurveyObject->SetNumberField(TEXT("angle"), AOI->Params.TransectsAngle);
	SurveyObject->SetStringField(TEXT("complexItemType"), TEXT("survey"));
	SurveyObject->SetNumberField(TEXT("entryLocation"), AOI->Params.EntryPoint);
	SurveyObject->SetBoolField(TEXT("flyAlternateTransects"), false);

	ASPPolygon* Polygon = AOI->GetPolygon();

	TArray<FVector> Locations;
	Polygon->GetPointLocations(Locations);

	TArray<TSharedPtr<FJsonValue>> PolygonPoints = MakeCoordinateArrayJson(Locations);

	SurveyObject->SetArrayField(TEXT("polygon"), PolygonPoints);
	SurveyObject->SetStringField(TEXT("type"), TEXT("ComplexItem"));
	MissionItems.Add(MakeShared<FJsonValueObject>(SurveyObject));
}

TSharedPtr<FJsonObject> USPMissionFormatter::MakeMissionJson(const USPTakeoffPoint* Takeoff, const TArray<const USPInterest*>& OrderedInterests) {
	TArray<TSharedPtr<FJsonValue>> MissionItems;
	TArray<TSharedPtr<FJsonValue>> PlannedHomePosition;

	HandleAddTakeoffItem(MissionItems, MissionItems.Num() + 1, Cast<USPTakeoffPoint>(Takeoff), PlannedHomePosition);

	int32 NestedItemJumpIDOffset = 0;
	for (int i = 0; i < OrderedInterests.Num(); i++) {
		const USPInterest* Interest = OrderedInterests[i];
		EInterestType Type = Interest->GetInterestType();

		switch (Type) {
		case EInterestType::POI:
			HandleAddPOIItem(MissionItems, MissionItems.Num() + 1 + NestedItemJumpIDOffset, Cast<USPPOI>(Interest));
			break;
		case EInterestType::AOI:
			HandleAddSurveyItem(MissionItems, MissionItems.Num() + 1 + NestedItemJumpIDOffset, Cast<USPAOI>(Interest), NestedItemJumpIDOffset);
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

TSharedPtr<FJsonObject> USPMissionFormatter::MakeRallyPointsJson() {
	TArray<TSharedPtr<FJsonValue>> RallyPoints;
	TSharedPtr<FJsonObject> RallyPointsObject = MakeShared<FJsonObject>();
	RallyPointsObject->SetArrayField(TEXT("points"), RallyPoints);
	return RallyPointsObject;
}

TSharedPtr<FJsonObject> USPMissionFormatter::FormatMissionAsJson(const FSPInterestStruct* MissionInterests, const TArray<int32>& InterestOrder) {
	TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();

	if (!MissionInterests) {
		return RootObject;
	}

	USPTakeoffPoint* Takeoff = nullptr;
	TArray<const USPInterest*> OrderedInterests;
	TArray<USPGeoFence*> GeoFences;

	for (const int32& ID : InterestOrder) {
		USPInterest* Interest = MissionInterests->Interests.Find(ID)->Get();
		if (Interest) {
			EInterestType Type = Interest->GetInterestType();
			if (Type == EInterestType::POI || Type == EInterestType::AOI) {
				OrderedInterests.Add(Interest);
			}
			else if (Type == EInterestType::GeoFence) {
				USPGeoFence* Fence = Cast<USPGeoFence>(Interest);
				if (Fence) {
					GeoFences.Add(Fence);
				}
			}
			else if (Type == EInterestType::Takeoff) {
				Takeoff = Cast<USPTakeoffPoint>(Interest);
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
