// Copyright (c) 2025 Synetos Aerospace


#include "State/Storage/SPSimulationManager.h"
#include "Objects/Drones/DemoSimDrone.h"
#include "Objects/SimulationData.h"
#include "State/SPGameState.h"
#include "Objects/Drones/DroneTrack.h"

const FString UNASSIGNED = TEXT("Default");

USPSimulationManager::USPSimulationManager()
	: autoAssignmentGroup(UNASSIGNED) {

	HighestPointCount = 0;
	CameraIndex = 0;
}

void USPSimulationManager::Setup_Implementation(bool& outSuccess) {
	Super::Setup_Implementation(outSuccess);
}

void USPSimulationManager::StartDemo_Implementation() {

}

void USPSimulationManager::AddDrone(ADroneTrack* Track) {
	if (!DronesToSpawn) {
		UE_LOG(LogTemp, Warning, TEXT("Must define DronesToSpawn for SimulationManager"))
			return;
	}

	ADemoSimDrone* Drone;
	gameStateRef->GenericSpawnActor<ADemoSimDrone>(DronesToSpawn, FVector::ZeroVector, Drone);

	Drone->SetDroneTrack(Track);
	Drones.Emplace(Drone);
}

// Averages the swarm's progress and returns that value as a float.
float USPSimulationManager::GetSimProgress() {
	float OverallProgress = 0.0;
	if (Drones.Num() <= 0) {
		return OverallProgress;
	}

	for (auto& Drone : Drones) {
		OverallProgress += Drone->GetProgress();
	}

	OverallProgress = OverallProgress / (float)Drones.Num();
	return OverallProgress;
}

void USPSimulationManager::SendSimProgressToDispatch_Implementation() {
	
}

void USPSimulationManager::StartSimulation_Implementation() {

}

void USPSimulationManager::LoadSimulation(FString Title) {
	ADroneTrack* Track;

	if (!TracksToSpawn) {
		UE_LOG(LogTemp, Warning, TEXT("Must define TracksToSpawn for SimulationManager"))
			return;
	}
	gameStateRef->GenericSpawnActor<ADroneTrack>(TracksToSpawn, FVector::ZeroVector, Track);
	FString TrackFilePath = FPaths::ProjectContentDir() / TEXT("Test/SFO_CPH_flightData.csv");

	Track->LoadTrackByCSVFile(TrackFilePath);
	Track->ClearSplineTrackPoints();
	Track->LoadSplineTrackPoints();
	int currentCount = Track->GetPointCount();
	
	if (currentCount > HighestPointCount) {
		HighestPointCount = Track->GetPointCount();
	}

	AddDrone(Track);

	int32 index = 0;
	for (auto& Data : SimsData) {
		if (Data.SimulationTitle.Equals(Title, ESearchCase::IgnoreCase)) {
			SimsData[index].DroneCount = Drones.Num();
		}
		index++;
	}
}

void USPSimulationManager::UnloadSimulation() {
	for (auto& Drone : Drones) {
		Drone->DestoryAircraft();
	}
	Drones.Empty();
}

void USPSimulationManager::EndSimulation_Implementation() {

}

void USPSimulationManager::RestartSimulation_Implementation() {

}

void USPSimulationManager::PauseSimulation_Implementation() {

}

void USPSimulationManager::ResumeSimulation_Implementation() {

}

// Sets how long the simulation should last (in seconds)
void USPSimulationManager::SetSimulationDuration(float Seconds) {
	float PointRatio = 0.0;

	for (auto& Drone : Drones) {
		// Calculates the PointRatio for each drone, based on how many total points a drone has in its
		// DroneTrack, compare to the highest overall point count in all of the swarm.
		PointRatio = (float)Drone->GetDroneTrack()->GetPointCount() / (float)HighestPointCount;

		// Sets the duration of each individual drone based on their own ratio and the highest
		// overall point count in the swarm.
		Drone->SetDuration(Seconds * PointRatio);
	}
}

// Modifies the overall progress of the Simulation, Percentage value 
// must be a float value Between 1 - 0
void USPSimulationManager::ChangeSimulationProgress(float Percentage) {
	float PointNumber = 0.0;

	for (auto& Drone : Drones) {
		// Calculates which point each individual Drone should be at depending on the Percentage given, and
		// their own PointRatio.
		PointNumber = ((float)Drone->GetDroneTrack()->GetPointCount() * Percentage)
			/ ((float)Drone->GetDroneTrack()->GetPointCount() / (float)HighestPointCount);

		// Sets each individual drone's progress based on what Point number they should be at and
		// their point count.
		Drone->SetProgress(PointNumber / (float)Drone->GetDroneTrack()->GetPointCount());
	}
}

// Creates a new Simulation
void USPSimulationManager::CreateSimulation(FString Title) {
	FSimulationData SimDetails;

	SimDetails.SimulationTitle = Title;
	SimsData.Emplace(SimDetails);
}

// Returns array of all Simulations' FSimulationData Struct
TArray<FSimulationData> USPSimulationManager::GetAllSimulationsData() {
	return SimsData;
}

// Returns only one Simulation's FSimulationData Struct, by matching title of Simulation
FSimulationData USPSimulationManager::GetSimulationDataByName(FString Name) {
	int32 index = 0;
	for (auto& Data : SimsData) {
		if (Data.SimulationTitle.Equals(Name, ESearchCase::IgnoreCase)) {
			return SimsData[index];
		}
		index++;
	}

	FSimulationData NoDataFound;
	NoDataFound.SimulationTitle = "Invalid";
	NoDataFound.DroneCount = -1;

	return NoDataFound;
}

// Returns array of all current drones being used in Simulation
TArray<ADemoSimDrone*> USPSimulationManager::GetDrones() {
	return Drones;
}

int32 USPSimulationManager::GetCamIndex() {
	return CameraIndex;
}

void USPSimulationManager::SetCamIndex(int32 InCamIndex) {
	CameraIndex = InCamIndex;
}

void USPSimulationManager::SwapToIsoCam_Implementation() {

}

void USPSimulationManager::SwapToFrontCam_Implementation() {

}

void USPSimulationManager::SwapToEYECam_Implementation() {

}

void USPSimulationManager::CycleCurrentDroneCam() {
	SwapToIsoCam();

	if (Drones.Num() < 2) {
		CameraIndex = 0;
	}
	else {
		CameraIndex = (CameraIndex + 1) % Drones.Num();
	}
}

void USPSimulationManager::CycleCurrentDroneCamReverse() {
	SwapToIsoCam();

	if (Drones.Num() < 2) {
		CameraIndex = 0;
	}
	else {
		CameraIndex = CameraIndex--;
		if (CameraIndex < 0 ) {
			CameraIndex = Drones.Num() - 1;
		}

	}
}

ADemoSimDrone* USPSimulationManager::GetCurrentActivateCamDrone() {
	if (Drones.IsEmpty()){
		return nullptr;
	}
	return Drones[CameraIndex];
}
