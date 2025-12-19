// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Objects/Interests/Interest.h"
#include "AreaOfInterest.generated.h"

// Forward References
class APolygon;

USTRUCT(BlueprintType)
struct FAOIParametersStruct {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TriggerDistM = 25.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TransectsAngle = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TurnaroundDistM = 10.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SpacingM = 25.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Altitude = 61.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool ReflyAt90DegOffset = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool ImagesInTurnarounds = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int EntryPoint = 0;
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API UAreaOfInterest : public UInterest
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	FAOIParametersStruct GetParams() const;

	virtual FString GetSerializedParams() const;
	virtual void SetSerializedParams(const FString& ParamsJson) override;

	UFUNCTION(BlueprintCallable)
	void SetParams(UPARAM(ref) FAOIParametersStruct& InParams, bool UpdateDB = false);

	virtual EInterestType GetInterestType() const override;

	virtual void SetSelected(bool IsSelected);

	virtual void SetName(FString InName) override;

	UFUNCTION(BlueprintCallable)
	void SetPolygon(APolygon* InPolygon);

	UFUNCTION(BlueprintCallable)
	void GetPolygon(APolygon*& OutPolygon) const;

	UFUNCTION(BlueprintCallable)
	TArray<FVector> GeneratePolygonTransectPoints() const;

	virtual void DestroySelf_Implementation();

	virtual void ToggleCull_Implementation(bool IsCulled, bool FromFlyTo) override;

	virtual void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals);
private:
	UPROPERTY()
	TObjectPtr<APolygon> Polygon;

	FAOIParametersStruct Params;
};
