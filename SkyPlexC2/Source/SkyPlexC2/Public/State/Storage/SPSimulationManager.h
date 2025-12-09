// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "State/SPManager.h"
#include "Objects/SimulationData.h"
#include "SPSimulationManager.generated.h"

// Forward declarations
class ASPGameState;
class ADroneTrack;
class ADemoSimDrone;
/**
 * Manages Switching, loading and saving of Drone Simulations
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API USPSimulationManager : public USPManager
{
	GENERATED_BODY()
	
public:
	USPSimulationManager();

	void Setup_Implementation(bool& outSuccess) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Simulation")
	void StartDemo();
	void StartDemo_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void AddDrone(ADroneTrack* Track);

	// Averages the swarm's progress and returns that value as a float.
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	float GetSimProgress();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Simulation")
	void SendSimProgressToDispatch();
	void SendSimProgressToDispatch_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Simulation")
	void StartSimulation();
	void StartSimulation_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void LoadSimulation(FString Title);

	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void UnloadSimulation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Simulation")
	void EndSimulation();
	void EndSimulation_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Simulation")
	void RestartSimulation();
	void RestartSimulation_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Simulation")
	void PauseSimulation();
	void PauseSimulation_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Simulation")
	void ResumeSimulation();
	void ResumeSimulation_Implementation();

	// Sets how long the simulation should last (in seconds)
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void SetSimulationDuration(float Seconds);

	// Modifies the overall progress of the Simulation, Percentage value 
	// must be a float value Between 1 - 0
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void ChangeSimulationProgress(float Percentage);

	// Creates a new Simulation
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void CreateSimulation(FString Title);

	// Returns array of all Simulations' FSimulationData Struct
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	TArray<FSimulationData> GetAllSimulationsData();

	// Returns only one Simulation's FSimulationData Struct, by matching title of Simulation
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	FSimulationData GetSimulationDataByName(FString Name);

	// Returns array of all current drones being used in Simulation
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	TArray<ADemoSimDrone*> GetDrones();

	UFUNCTION(BlueprintCallable, Category = "Simulation")
	int32 GetCamIndex(); 

	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void SetCamIndex(int32 InCamIndex);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Simulation")
	void SwapToIsoCam();
	void SwapToIsoCam_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Simulation")
	void SwapToFrontCam();
	void SwapToFrontCam_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Simulation")
	void SwapToEYECam();
	void SwapToEYECam_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void CycleCurrentDroneCam();

	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void CycleCurrentDroneCamReverse();

	UFUNCTION(BlueprintCallable, Category = "Simulation")
	ADemoSimDrone* GetCurrentActivateCamDrone();

	UPROPERTY(EditAnywhere, Category = "Simulation")
	TArray<ADemoSimDrone*> Drones;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Simulation")
	TSubclassOf<ADemoSimDrone> DronesToSpawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Simulation")
	TSubclassOf<ADroneTrack> TracksToSpawn;

	UPROPERTY(EditAnywhere, Category = "Simulation")
	TArray<FSimulationData> SimsData;

private:

	int32 HighestPointCount; // Stores the Drone Track with the largest number of points

	int32 CameraIndex; // Stores the current Drone index that has a active camera for the Manager

	FString autoAssignmentGroup;

	const FString LOG_ORIGIN = TEXT("SimulationManager");
};
