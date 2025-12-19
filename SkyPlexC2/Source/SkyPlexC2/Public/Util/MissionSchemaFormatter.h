// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MissionSchemaFormatter.generated.h"

// forward declarations
struct FSPInterestStruct;
class UInterest;
class USPGeoFence;
class USPTakeoffPoint;
class UPOI;
class UAreaOfInterest;

/**
 * 
 */
UCLASS()
class SKYPLEXC2_API UMissionSchemaFormatter : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static TSharedPtr<FJsonObject> FormatMissionAsJson(const FSPInterestStruct* MissionInterests, const TArray<int32>& InterestOrder);

private:
	static TSharedPtr<FJsonObject> MakeGeoFencesJson(const TArray<USPGeoFence*>& GeoFences);
	static TSharedPtr<FJsonObject> MakeMissionJson(const USPTakeoffPoint* Takeoff, const TArray<const UInterest*>& OrderedInterests);
	static TSharedPtr<FJsonObject> MakeRallyPointsJson();
	static TArray<TSharedPtr<FJsonValue>> MakeCoordinateArrayJson(const TArray<FVector>& Points);

	static void HandleAddTakeoffItem(
		TArray<TSharedPtr<FJsonValue>>& MissionItems,
		int32 JumpID,
		const USPTakeoffPoint* TakeoffPoint,
		TArray<TSharedPtr<FJsonValue>>& PlannedHomePosition
	);
	static void HandleAddPOIItem(
		TArray<TSharedPtr<FJsonValue>>& MissionItems,
		int32 JumpID,
		const UPOI* POI
	);
	static void HandleAddSimpleWaypointItem(
		TArray<TSharedPtr<FJsonValue>>& MissionItems,
		int32 JumpID,
		const FVector& LonLatAlt
	);
	static void HandleAddSurveyItem(
		TArray<TSharedPtr<FJsonValue>>& MissionItems,
		int32 JumpID,
		const UAreaOfInterest* AOI,
		int32& NestedItemJumpIDOffset
	);
	static TSharedPtr<FJsonObject> MakeTransectStyleComplexItem(int32 JumpID, const UAreaOfInterest* AOI, int32& NestedItemJumpIDOffset);

	static void HandleAddSpeedItemObject(TArray<TSharedPtr<FJsonValue>>& MissionItems, int32 JumpID, float SpeedMS);
	static void HandleAddReturnToLaunchObject(TArray<TSharedPtr<FJsonValue>>& MissionItems, int32 JumpID);
};
