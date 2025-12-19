// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Placeable.h"
#include "InterestPoint.generated.h"

/**
 * Not implemented in c++ yet and currently inherited by BP_InterestPoint
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API AInterestPoint : public APlaceable
{
	GENERATED_BODY()

public:
	void DestroySelf_Implementation() override;

	UFUNCTION(BlueprintCallable)
	void SetGroupID(int32 inID);

	UFUNCTION(BlueprintCallable)
	void GetGroupID(int32& outID) const;

	virtual void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& outKeyVals) override;
	
private:
	int32 groupID = -1;
};
