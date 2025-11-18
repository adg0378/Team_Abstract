// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Objects/SPInteractionInterface.h"
#include "Objects/SPPlaceableInterface.h"
#include "SPInterest.generated.h"

UENUM(BlueprintType)
enum class EInterestType : uint8
{
	POI UMETA(DisplayName = "POI"),
	AOI UMETA(DisplayName = "AOI"),
	GeoFence UMETA(DisplayName = "GeoFence"),
	Takeoff UMETA(DisplayName = "TakeoffPoint"),
	Unknown UMETA(DisplayName = "Unknown")
};

/**
 * Base interest class
 */
UCLASS()
class SKYPLEXC2_API USPInterest : public UObject, public ISPInteractionInterface, public ISPPlaceableInterface
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	virtual EInterestType GetInterestType() const;

	/** Does not reassign in database */
	UFUNCTION(BlueprintCallable)
	void SetGroupID(int32 InID);

	UFUNCTION(BlueprintCallable)
	void ReassignGroup(int32 GroupID);

	UFUNCTION(BlueprintCallable)
	int32 GetGroupID() const;

	UFUNCTION(BlueprintCallable)
	void SetID(int32 InID);

	UFUNCTION(BlueprintCallable)
	int32 GetID() const;

	/** Does not rename in database. Use RenameInterest instead */
	UFUNCTION(BlueprintCallable)
	virtual void SetName(FString InName);

	UFUNCTION(BlueprintCallable)
	void RenameInterest(FString InName);

	UFUNCTION(BlueprintCallable)
	FString GetName() const;

	UFUNCTION(BlueprintCallable)
	void SetAttributes(int32 InID, int32 InGroupID, FString InName);

	/** Used by interest panel to programatically select the interest on the map */
	UFUNCTION(BlueprintCallable)
	virtual void SetSelected(bool IsSelected);

	virtual void ToggleCull_Implementation(bool IsCulled);
	virtual void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals);
	virtual void GetInteractionBoxTitle_Implementation(FText& OutTitle);
	virtual void ProvideOnPlaced_Implementation();

protected:
	int32 GroupID = -1;

	int32 ID = -1;

	FString Name = "0";
};
