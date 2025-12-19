// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Objects/Placeable.h"
#include "SPPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlacementModeExitedSignature);

// forward declarations
class APolygon;
class ASPGameState;

UENUM(BlueprintType)
enum class EControllerMode : uint8
{
	Default UMETA(DisplayName = "Default"),
	MultiSelect UMETA(DisplayName = "MultiSelect"),
	Placement UMETA(DisplayName = "Placement"),
	Drag UMETA(DisplayName = "Drag"),
};


UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API ASPPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	EControllerMode mode;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlaceable> DraggedObject;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APolygon> InProgressPolygon;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlaceable> PlaceableActor;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<APlaceable> PlaceableActorType;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APlaceable> AOIPinType;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APlaceable> GeoFencePinType;

	UPROPERTY(BlueprintAssignable, Category = "Placement")
	FPlacementModeExitedSignature OnPlacementModeExited;

	UPROPERTY(BlueprintReadWrite)
	bool PlacementModeEnabled = false;

	UFUNCTION(BlueprintCallable)
	void OnPlacementInfoUpdated(TSubclassOf<APlaceable> InPlaceableActorType, EControllerMode ControllerMode);

	UFUNCTION(BlueprintCallable)
	void SetPlacementModeEnabled(bool IsEnabled);

	UFUNCTION(BlueprintCallable)
	void SpawnActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void InitializeHUD();

private:

	void InitializeInProgressPolygon(ASPGameState* GameState);
};
