// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SPCameraInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USPCameraInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * If an actor is ever treated as a camera from the player's POV, it should implement this interface
 */
class SKYPLEXC2_API ISPCameraInterface : public IInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetZoomPercentage(float& OutZoomPercentage) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetCameraLocation(FVector& OutCameraLocation) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetCameraRotation(FRotator& OutCameraRotation) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetCameraTypeFromIndex(uint8 Index);
};
