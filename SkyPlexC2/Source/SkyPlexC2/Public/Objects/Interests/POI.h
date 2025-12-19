// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Objects/Interests/Interest.h"
#include "Objects/Geometry/PlaceablePoint.h"
#include "POI.generated.h"

USTRUCT(BlueprintType)
struct FPOIParametersStruct {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float HoldTimeS = 0.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float YawDeg = 400.0f; // 400.0f triggers NULL to be passed

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SpeedMS = -1.0f; // anything <= 0 is interpreted as no speed change required

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Altitude = 61.0f;
};

UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API UPOI : public UInterest
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	FPOIParametersStruct GetParams() const;

	virtual FString GetSerializedParams() const;
	virtual void SetSerializedParams(const FString& ParamsJson) override;
	
	UFUNCTION(BlueprintCallable)
	void SetParams(UPARAM(ref) FPOIParametersStruct& InParams, bool UpdateDB = false);

	virtual EInterestType GetInterestType() const override;

	virtual void SetSelected(bool IsSelected);

	virtual void SetName(FString InName) override;

	UFUNCTION(BlueprintCallable)
	void SetPoint(APlaceablePoint* InPoint);

	UFUNCTION(BlueprintCallable)
	void GetPoint(APlaceablePoint*& OutPoint) const;

	virtual void ToggleCull_Implementation(bool IsCulled, bool FromFlyTo) override;
	virtual void DestroySelf_Implementation();
	virtual void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) override;
private:
	UPROPERTY()
	TObjectPtr<APlaceablePoint> Point;

	FPOIParametersStruct Params;
};
