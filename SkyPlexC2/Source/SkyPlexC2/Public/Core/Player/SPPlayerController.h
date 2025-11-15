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

	UFUNCTION(BlueprintCallable, Category = "SPPlayerController", meta = (WorldContext = "WorldContextObject"))
	static ASPPlayerController* GetSPPlayerController(const UObject* WorldContextObject);

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

	UFUNCTION(BlueprintCallable)
	void IADoActionStarted();

	UFUNCTION(BlueprintCallable)
	void IADoActionCompleted();

	UFUNCTION(BlueprintCallable)
	void IAMultipleSelect();

	UFUNCTION(BlueprintCallable)
	void IADelete();

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
	UPROPERTY()
	TObjectPtr<class ASPMainCamera> MainCameraRef;

	UPROPERTY()
	TObjectPtr<class ASPGameState> GameStateRef;

	bool FreeCameraPressed = false;
	bool ShowCursorInFirstPersonPressed = false;
};
