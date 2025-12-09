// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Objects/Interests/Interest.h"
#include "SPGeoFence.generated.h"

// Forward References
class APolygon;

USTRUCT(BlueprintType)
struct FGeoFenceParametersStruct {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Inclusive = true;
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API USPGeoFence : public UInterest
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	FGeoFenceParametersStruct GetParams() const;

	virtual FString GetSerializedParams() const;
	virtual void SetSerializedParams(const FString& ParamsJson) override;

	UFUNCTION(BlueprintCallable)
	void SetParams(UPARAM(ref) FGeoFenceParametersStruct& InParams, bool UpdateDB = false);

	virtual EInterestType GetInterestType() const override;

	virtual void SetSelected(bool IsSelected);

	virtual void SetName(FString InName) override;

	UFUNCTION(BlueprintCallable)
	void SetPolygon(APolygon* InPolygon);

	UFUNCTION(BlueprintCallable)
	void GetPolygon(APolygon*& OutPolygon) const;

	virtual void DestroySelf_Implementation();
	virtual void ToggleCull_Implementation(bool IsCulled, bool FromFlyTo) override;
	virtual void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals);
private:
	UPROPERTY()
	TObjectPtr<APolygon> Polygon;
	
	FGeoFenceParametersStruct Params;
};
