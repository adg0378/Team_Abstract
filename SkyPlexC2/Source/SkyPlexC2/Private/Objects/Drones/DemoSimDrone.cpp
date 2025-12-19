// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Drones/DemoSimDrone.h"
#include "Objects/Drones/DroneTrack.h"


// Sets default values
ADemoSimDrone::ADemoSimDrone() {

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	DroneTrack = nullptr;

	Duration = 0.0;
	StartOffset = 0.0;
	Progress = 0.0;
}

// Called when the game starts or when spawned
void ADemoSimDrone::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ADemoSimDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADemoSimDrone::DestoryAircraft() {
	DroneTrack->DestroyTrack();
	Destroy();
}

void ADemoSimDrone::SetDroneTrack(ADroneTrack* InTrack) {
	DroneTrack = InTrack;
}

ADroneTrack* ADemoSimDrone::GetDroneTrack() {
	return DroneTrack;
}

void ADemoSimDrone::SetDuration(float InDuration) {
	Duration = InDuration;
}

float ADemoSimDrone::GetDuration() {
	return Duration;
}

void ADemoSimDrone::SetStartOffset(float InStartOffset) {
	StartOffset = InStartOffset;
}

float ADemoSimDrone::GetStartOffset() {
	return StartOffset;
}

//Must be a float value Between 1 - 0
void ADemoSimDrone::SetProgress(float InProgress) {
	Progress = InProgress;
}

float ADemoSimDrone::GetProgress() {
	return Progress;
}

void ADemoSimDrone::StartDroneMovement_Implementation() {

}

void ADemoSimDrone::EndDroneMovement_Implementation() {

}

void ADemoSimDrone::RestartDrone_Implementation() {

}

void ADemoSimDrone::PauseDrone_Implementation() {

}

void ADemoSimDrone::ResumeDrone_Implementation() {

}
