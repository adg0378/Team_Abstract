// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SPPlaceableInterface.generated.h"

UINTERFACE(MinimalAPI)
class USPPlaceableInterface : public UInterface
{
	GENERATED_BODY()
};

/*
 * This interface assists with propagating placeable events
 *
 * Any class implementing this interface should probably also implement IInteractionBoxProvider
 */
class SKYPLEXC2_API ISPPlaceableInterface
{
	GENERATED_BODY()

public:
	/* This method calls up the tree notifying parents that an object was placed or moved */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ProvideOnPlaced();
};
