// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Objects/Interests/SPInterest.h"
#include "SPPOI.generated.h"

USTRUCT(BlueprintType)
struct FPOIParametersStruct {
	GENERATED_BODY()

public:
	float HoldTimeS = 0.0;
	float YawDeg = 400.0f; // 400.0f triggers NULL to be passed
	float SpeedMS = -1.0f; // anything <= 0 is interpreted as no speed change required
	float Altitude = 200.0f;
};

/**
 * Point of Interest
 */
UCLASS(Blueprintable)
class SKYPLEXC2_API USPPOI : public USPInterest
{
	GENERATED_BODY()
	
public:
	FPOIParametersStruct Params;

	virtual EInterestType GetInterestType() const override;

	virtual void SetSelected(bool IsSelected);

	virtual void SetName(FString InName) override;

	UFUNCTION(BlueprintCallable)
	void SetPoint(class ASPPlaceablePoint* InPoint);

	UFUNCTION(BlueprintCallable)
	ASPPlaceablePoint* GetPoint() const;

	virtual void ToggleCull_Implementation(bool IsCulled) override;

	virtual void DestroySelf_Implementation();
private:
	UPROPERTY()
	TObjectPtr<ASPPlaceablePoint> Point;
};
