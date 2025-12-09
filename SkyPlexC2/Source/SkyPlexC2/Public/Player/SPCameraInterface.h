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
 * 
 */
class SKYPLEXC2_API ISPCameraInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetZoomPercentage(float& OutZoomPercentage) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetCameraLocation(FVector& OutCameraLocation) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetCameraRotation(FRotator& OutCameraRotation) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetDevicePitch(float& OutDevicePitch) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetDeviceRoll(float& OutDeviceRoll) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetDeviceYaw(float& OutDeviceYaw) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetDeviceName(FString& OutDeviceName) const;


};
