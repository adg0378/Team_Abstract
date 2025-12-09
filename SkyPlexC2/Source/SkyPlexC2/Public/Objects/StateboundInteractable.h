// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/Interactable.h"
#include "StateboundInteractable.generated.h"

// Forward declarations
enum class EState : uint8;

/**
 * Interactable class that is bound to a mission state and assignment
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API AStateboundInteractable : public AInteractable
{
	GENERATED_BODY()

public:
	AStateboundInteractable();

	virtual void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& outKeyVals) override;

	UFUNCTION(BlueprintCallable)
	void SetState(EState state);

	UFUNCTION(BlueprintCallable)
	void GetState(EState& outState) const;

	UFUNCTION(BlueprintCallable)
	void GetID(int32& outID) const;

	UFUNCTION(BlueprintCallable)
	void SetID(int32 inID);

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	EState state;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	int32 id;
};


