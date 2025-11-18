// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SPMissionFormatter.generated.h"

/**
 * Serializes and parses missions and their interests as json
 */
UCLASS()
class SKYPLEXC2_API USPMissionFormatter : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static TSharedPtr<FJsonObject> FormatMissionAsJson(const struct FSPInterestStruct* MissionInterests, const TArray<int32>& InterestOrder);

private:
	static TSharedPtr<FJsonObject> MakeGeoFencesJson(const TArray<class USPGeoFence*>& GeoFences);
	static TSharedPtr<FJsonObject> MakeMissionJson(const class USPTakeoffPoint* Takeoff, const TArray<const class USPInterest*>& OrderedInterests);
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
		const class USPPOI* POI
	);
	static void HandleAddSimpleWaypointItem(
		TArray<TSharedPtr<FJsonValue>>& MissionItems,
		int32 JumpID,
		const FVector& LonLatAlt
	);
	static void HandleAddSurveyItem(
		TArray<TSharedPtr<FJsonValue>>& MissionItems,
		int32 JumpID,
		const class USPAOI* AOI,
		int32& NestedItemJumpIDOffset
	);
	static TSharedPtr<FJsonObject> MakeTransectStyleComplexItem(int32 JumpID, const class USPAOI* AOI, int32& NestedItemJumpIDOffset);

	static void HandleAddSpeedItemObject(TArray<TSharedPtr<FJsonValue>>& MissionItems, int32 JumpID, float SpeedMS);
	static void HandleAddReturnToLaunchObject(TArray<TSharedPtr<FJsonValue>>& MissionItems, int32 JumpID);
};
