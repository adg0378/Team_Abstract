// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Objects/Interests/SPInterest.h"
#include "SPAOI.generated.h"

USTRUCT(BlueprintType)
struct FAOIParametersStruct {
	GENERATED_BODY()

public:
	float TriggerDistM = 25.0f;
	float TransectsAngle = 0.0f;
	float TurnaroundDistM = 10.0f;
	float SpacingM = 25.0f;
	float Altitude = 200.0f;
	bool ReflyAt90DegOffset = false;
	bool ImagesInTurnarounds = true;
	int EntryPoint = 0;
};

/**
 * Area of interest
 */
UCLASS()
class SKYPLEXC2_API USPAOI : public USPInterest
{
	GENERATED_BODY()
	
public:
	FAOIParametersStruct Params;

	virtual EInterestType GetInterestType() const override;

	virtual void SetSelected(bool IsSelected);

	virtual void SetName(FString InName) override;

	UFUNCTION(BlueprintCallable)
	void SetPolygon(class ASPPolygon* InPolygon);

	UFUNCTION(BlueprintCallable)
	ASPPolygon* GetPolygon() const;

	UFUNCTION(BlueprintCallable)
	TArray<FVector> GeneratePolygonTransectPoints() const;

	virtual void DestroySelf_Implementation();

	virtual void ToggleCull_Implementation(bool IsCulled) override;

private:
	UPROPERTY()
	TObjectPtr<ASPPolygon> Polygon;
};
