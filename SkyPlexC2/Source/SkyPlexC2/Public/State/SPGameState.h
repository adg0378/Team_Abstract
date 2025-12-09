// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#define NOMINMAX
#include "CoreMinimal.h"
#include "SPUtility.h"
#include "SPLogger.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "CesiumGeospatial/Ellipsoid.h"
#include "GeographicUtility.h"
#include "CesiumGeospatial/Cartographic.h"
#include "Delegates/DelegateCombinations.h"
#include <CesiumGeoreference.h>
#include <Cesium3DTileset.h>
#include <CesiumGlobeAnchorComponent.h>
#include "SPGameState.generated.h"

// Forward declare to avoid circular dependencies
class USPStorageManager;
class USPDroneManager;
class USPMissionManager;
class USPObstacleManager;
class USPSimulationManager;
class AInteractable;
class UPreferencesManager;
class USPWorldManager;
class USPAuthManager;
struct FInteractionBoxValue;

using FSPSampleHeightCallback = TFunction<void(const TArray<FVector>& SampledPositions)>;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStateFullyInitialized);

/**
 * CPP game state class for SkyPlex to store managers and selections
 */
UCLASS(BlueprintType, Blueprintable)
class SKYPLEXC2_API ASPGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnGameStateFullyInitialized OnGameStateFullyInitialized;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State Managers")
	TSubclassOf<USPLogger> LoggerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State Managers")
	TSubclassOf<USPStorageManager> StorageManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State Managers")
	TSubclassOf<USPDroneManager> DroneManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State Managers")
	TSubclassOf<USPMissionManager> MissionManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State Managers")
	TSubclassOf<USPObstacleManager> ObstacleManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State Managers")
	TSubclassOf<USPSimulationManager> SimulationManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State Managers")
	TSubclassOf<UPreferencesManager> PreferencesManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State Managers")
	TSubclassOf<USPWorldManager> WorldManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State Managers")
	TSubclassOf<USPAuthManager> AuthManagerToSpawn;

	template <typename T>
	void GenericSpawnActor(TSubclassOf<T> ActorType, FVector SpawnLocation, T*& OutActor, FVector SpawnScale = FVector(1.0f, 1.0f, 1.0f), FRotator SpawnRotation = FRotator::ZeroRotator) {
		UWorld* World = GetWorld();
		FString LogOrigin = TEXT("SpawnActor");

		if (!World) {
			loggerRef->Error(TEXT("Failed to spawn: No world context"), LogOrigin);
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		FTransform SpawnTransform(SpawnRotation, SpawnLocation, SpawnScale);
		T* SpawnedActor = World->SpawnActor<T>(ActorType, SpawnTransform, SpawnParams);
		OutActor = SpawnedActor;
	};

	template <typename T>
	void GenericSpawnActorAtLonLatHeight(TSubclassOf<T> ActorType, FVector LonLatHeight, T*& OutActor, FVector SpawnScale = FVector(1.0f, 1.0f, 1.0f), FRotator SpawnRotation = FRotator::ZeroRotator) {
		UWorld* World = GetWorld();
		FString LogOrigin = TEXT("SpawnActorAtLonLatHeight");

		if (!World) {
			loggerRef->Error(TEXT("Failed to spawn: No world context"), LogOrigin);
			return;
		}

		ACesiumGeoreference* Georeference = ACesiumGeoreference::GetDefaultGeoreference(World);
		if (!Georeference) {
			loggerRef->Error(TEXT("Failed to spawn: No georeference"), LogOrigin);
			return;
		}

		FVector UnrealPos = Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(LonLatHeight);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		FTransform SpawnTransform(SpawnRotation, UnrealPos, SpawnScale);

		T* SpawnedActor = World->SpawnActor<T>(ActorType, SpawnTransform, SpawnParams);
		OutActor = SpawnedActor;
	};

	/** Samples heights at cesium terrain. If there was an error sampling a height, the height of the returned lonlatheight result will be -9999 */
	void SampleHeights(const TArray<FVector>& LonLatHeightArray, FSPSampleHeightCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "State")
	void AddSelected(AInteractable* actor);

	UFUNCTION(BlueprintCallable, Category = "State")
	void RemoveSelected(AInteractable* actor);

	UFUNCTION(BlueprintCallable, Category = "State")
	void RefreshSelected();

	UFUNCTION(BlueprintCallable, Category = "State")
	void DeselectAll();

	UFUNCTION(BlueprintCallable, Category = "State")
	void IsSelected(bool& isSelected) const;

	UFUNCTION(BlueprintCallable, Category = "State")
	void IsMultipleSelected(bool& isMultipleSelected) const;

	UFUNCTION(BlueprintCallable, Category = "State")
	void DeleteSelected();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateInteractionBox(const FText& title, const TMap<FString, FInteractionBoxValue>& keyVals);
	virtual void UpdateInteractionBox_Implementation(const FText& title, const TMap<FString, FInteractionBoxValue>& keyVals);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DeleteInteractionBox();
	virtual void DeleteInteractionBox_Implementation();

	UFUNCTION(BlueprintCallable)
	void CompileInteractionBoxKeyVals(TMap<FString, FInteractionBoxValue>& outKeyVals);

	UFUNCTION(BlueprintCallable)
	void CompileInteractionBoxTitle(FText& outTitle);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SwapActivePlayerCamera(AActor* InCamera);
	void SwapActivePlayerCamera_Implementation(AActor* InCamera);

	UFUNCTION(BlueprintCallable)
	void GetSelectedTopLevelProviders(TArray<UObject*>& Providers) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPLogger> loggerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPStorageManager> storageManagerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPDroneManager> droneManagerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPMissionManager> MissionManagerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPObstacleManager> obstacleManagerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPSimulationManager> simulationManagerRef;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "State")
	TArray<TObjectPtr<AInteractable>> selectedActors;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	TObjectPtr<UPreferencesManager> PreferencesManager;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	TObjectPtr<USPWorldManager> WorldManagerRef;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "State")
	TObjectPtr<USPAuthManager> AuthManagerRef;

	// Non-manager refs that are accessed frequently
	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<ACesium3DTileset> CesiumTileset;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<ACesiumGeoreference> CesiumGeoreference;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<UWorld> SPWorld;

	UFUNCTION(BlueprintCallable)
	bool IsFullyInitialized() const;

protected:
	virtual void BeginPlay() override;

	bool FullyInitialized = false;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
