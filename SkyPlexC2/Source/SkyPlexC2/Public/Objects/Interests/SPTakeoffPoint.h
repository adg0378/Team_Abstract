// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Objects/Interests/Interest.h"
#include "SPTakeoffPoint.generated.h"

// forward declarations
class APlaceablePoint;

USTRUCT(BlueprintType)
struct FSPTakeoffPointParams {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Altitude = 61.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float InitialSpeedMS = 5.0f;
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API USPTakeoffPoint : public UInterest
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintCallable)
	FSPTakeoffPointParams GetParams() const;

	virtual FString GetSerializedParams() const;
	virtual void SetSerializedParams(const FString& ParamsJson) override;

	UFUNCTION(BlueprintCallable)
	void SetParams(UPARAM(ref) FSPTakeoffPointParams& InParams, bool UpdateDB = false);

	virtual EInterestType GetInterestType() const override;

	virtual void SetSelected(bool IsSelected);

	virtual void SetName(FString InName) override;

	UFUNCTION(BlueprintCallable)
	void SetPoint(APlaceablePoint* InPoint);

	UFUNCTION(BlueprintCallable)
	void GetPoint(APlaceablePoint*& OutPoint) const;

	virtual void ToggleCull_Implementation(bool IsCulled, bool FromFlyTo) override;

	virtual void DestroySelf_Implementation();

	virtual void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals);
private:
	UPROPERTY()
	TObjectPtr<APlaceablePoint> Point;

	FSPTakeoffPointParams Params;
};
