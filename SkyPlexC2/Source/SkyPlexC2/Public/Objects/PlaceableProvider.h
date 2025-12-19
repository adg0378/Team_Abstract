// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlaceableProvider.generated.h"

UINTERFACE(MinimalAPI)
class UPlaceableProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * This interface assists with propagating placeable events
 * 
 * Any class implementing this interface should also implement IInteractionBoxProvider
 */
class SKYPLEXC2_API IPlaceableProvider
{
	GENERATED_BODY()

public:
	/** This method calls up the tree notifying parents that an object was placed / moved */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ProvideOnPlaced();
};
