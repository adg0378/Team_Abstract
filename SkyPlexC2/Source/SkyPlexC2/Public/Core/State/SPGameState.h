// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CesiumGeoreference.h"
#include "Core/SPLogger.h"
#include "SPGameState.generated.h"

using FSPSampleHeightCallback = TFunction<void(const TArray<FVector>& SampledPositions)>;

/**
 * Stores managers, plugins, and other commonly accessed references
 */
UCLASS()
class SKYPLEXC2_API ASPGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	// Object types to spawn to support blueprint defined managers
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<USPLogger> LoggerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<class USPPreferences> PreferencesManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<class USPAuthManager> AuthManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<class USPWorldManager> WorldManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<class USPDatabaseManager> DatabaseManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<class USPFlyToManager> FlyToManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<class USPDroneManager> DroneManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<class USPMissionManager> MissionManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<class USPObstacleManager> ObstacleManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<class USPSimulationManager> SimulationManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<class USPSelectionManager> SelectionManagerToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<class USPPlacementManager> PlacementManagerToSpawn;

	// Manager references and other common objects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPLogger> Logger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPPreferences> Preferences;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPAuthManager> AuthManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPWorldManager> WorldManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPDatabaseManager> DatabaseManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPFlyToManager> FlyToManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPDroneManager> DroneManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPMissionManager> MissionManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPObstacleManager> ObstacleManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPSimulationManager> SimulationManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPSelectionManager> SelectionManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<USPPlacementManager> PlacementManager;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class ACesium3DTileset> CesiumTileset;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ACesiumGeoreference> CesiumGeoreference;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UWorld> SPWorld;

	ASPGameState();

	UFUNCTION(BlueprintCallable, Category = "SPGameState", meta = (WorldContext = "WorldContextObject"))
	static ASPGameState* GetSPGameState(const UObject* WorldContextObject);

	template <typename T>
	void GenericSpawnActor(TSubclassOf<T> ActorType, FVector SpawnLocation, T*& OutActor, FVector SpawnScale = FVector(1.0f, 1.0f, 1.0f), FRotator SpawnRotation = FRotator::ZeroRotator) {
		UWorld* World = GetWorld();
		FString LogOrigin = TEXT("SpawnActor");

		if (!World) {
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn: No world context"));
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
			Logger->Error(TEXT("Failed to spawn: No world context"), LogOrigin);
			return;
		}

		ACesiumGeoreference* Georeference = ACesiumGeoreference::GetDefaultGeoreference(World);
		if (!Georeference) {
			Logger->Error(TEXT("Failed to spawn: No georeference"), LogOrigin);
			return;
		}

		FVector UnrealPos = Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(LonLatHeight);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		FTransform SpawnTransform(SpawnRotation, UnrealPos, SpawnScale);

		T* SpawnedActor = World->SpawnActor<T>(ActorType, SpawnTransform, SpawnParams);
		OutActor = SpawnedActor;
	};

	void SampleHeights(const TArray<FVector>& LonLatHeightArray, FSPSampleHeightCallback Callback);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
