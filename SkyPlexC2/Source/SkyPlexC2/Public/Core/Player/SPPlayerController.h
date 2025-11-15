// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SPPlayerController.generated.h"

/**
 * Base Player Controller
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API ASPPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASPPlayerController();

	UFUNCTION(BlueprintCallable)
	void ChangeCameraType(uint8 CameraTypeIndex);

protected:
	UFUNCTION(BlueprintCallable)
	void IACameraPitch(float ActionValue);

	UFUNCTION(BlueprintCallable)
	void IAFreeCamera();

	UFUNCTION(BlueprintCallable)
	void IAShowCursorInFirstPerson();

	UFUNCTION(BlueprintCallable)
	void IAMouseX(float ActionValue);

	UFUNCTION(BlueprintCallable)
	void IAMouseY(float ActionValue);

	UFUNCTION(BlueprintCallable)
	void IACameraZoomIn();

	UFUNCTION(BlueprintCallable)
	void IACameraZoomOut();

	UFUNCTION(BlueprintCallable)
	void IACameraOrbit(float ActionValue);

	UFUNCTION(BlueprintCallable)
	void IAMovement(FVector ActionValue);

	// TODO: add tick, do action, selection, multiselection, deselect, deletion when placement is added

	/* First person */
	UFUNCTION(BlueprintCallable)
	void IASwapCamera1();

	/* Top down */
	UFUNCTION(BlueprintCallable)
	void IASwapCamera2();

	/* Isometric */
	UFUNCTION(BlueprintCallable)
	void IASwapCamera3();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetupEnhancedInput();
	void SetupEnhancedInput_Implementation();

private:
	TObjectPtr<class ASPMainCamera> MainCameraRef;

	bool FreeCameraPressed = false;
	bool ShowCursorInFirstPersonPressed = false;
};
