// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Objects/Interactable.h"
#include "SPSimWorldOrigin.generated.h"

/**
 * Represents the origin of the Gazebo simulation world
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API ASPSimWorldOrigin : public AInteractable
{
	GENERATED_BODY()
	
public:
	void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) override;
	void GetInteractionBoxTitle_Implementation(FText& OutTitle) override;
private:
	float PermittedRadiusNM = 2.7f;
};
