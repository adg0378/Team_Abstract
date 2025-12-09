// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Objects/Interactable.h"
#include "State/Obstacles/ObstaclesUtil.h"
#include "FAADOFObstacle.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SKYPLEXC2_API AFAADOFObstacle : public AInteractable
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void GetData(FFAAObjectDataStruct& OutData);

	UFUNCTION(BlueprintCallable)
	void SetData(FFAAObjectDataStruct InData);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnSetData();
	void OnSetData_Implementation();

	void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) override;
	void GetInteractionBoxTitle_Implementation(FText& OutTitle) override;

	UPROPERTY(BlueprintReadWrite)
	float HeightAboveGroundLevelMeters = 10;

	// Stores initial height to prevent conflict with actor location during animations
	UPROPERTY(BlueprintReadWrite)
	float InitialHeight = 0.0;

	FFAAObjectDataStruct Data;
};
