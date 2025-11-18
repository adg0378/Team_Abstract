// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Objects/Interests/SPInterest.h"
#include "SPTakeoffPoint.generated.h"

USTRUCT(BlueprintType)
struct FSPTakeoffPointParams {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Altitude = 200.0f;
};

/**
 * Takeoff Point
 */
UCLASS()
class SKYPLEXC2_API USPTakeoffPoint : public USPInterest
{
	GENERATED_BODY()
	
public:
	FSPTakeoffPointParams Params;

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
