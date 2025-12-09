// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Objects/InteractionBoxProvider.h"
#include "Objects/PlaceableProvider.h"
#include "State/Missions/InterestsUtil.h"
#include "Interest.generated.h"

UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API UInterest : public UObject, public IInteractionBoxProvider, public IPlaceableProvider
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
	void GetGroupID(int32& OutID) const;

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
	void GetName(FString& OutName) const;

	UFUNCTION(BlueprintCallable)
	void SetAttributes(int32 InID, int32 InGroupID, FString InName);

	virtual FString GetSerializedParams() const;

	virtual void SetSerializedParams(const FString& ParamsJson);

	/** Used by interest panel to programatically select the interest on the map */
	UFUNCTION(BlueprintCallable)
	virtual void SetSelected(bool IsSelected);

	virtual void ToggleCull_Implementation(bool IsCulled, bool FromFlyTo);
	virtual void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals);
	virtual void GetInteractionBoxTitle_Implementation(FText& OutTitle);
	virtual void ProvideOnPlaced_Implementation();

protected:
	int32 GroupID = -1;

	int32 ID = -1;

	FString Name = "0";
};
