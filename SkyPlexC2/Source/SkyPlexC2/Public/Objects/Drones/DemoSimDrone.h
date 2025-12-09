// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "BasicDrone.h"
#include "GameFramework/Actor.h"
#include "DemoSimDrone.generated.h"

// Forward declarations
class ADroneTrack;

/**
 * Class for spawning and handling each indivial drone and their "track" in a simulation
 */
UCLASS()
class SKYPLEXC2_API ADemoSimDrone : public ABasicDrone
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADemoSimDrone();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void DestoryAircraft();

	UFUNCTION(BlueprintCallable)
	void SetDroneTrack(ADroneTrack* InTrack);

	UFUNCTION(BlueprintCallable)
	ADroneTrack* GetDroneTrack();

	UFUNCTION(BlueprintCallable)
	void SetDuration(float InDuration);

	UFUNCTION(BlueprintCallable)
	float GetDuration();

	UFUNCTION(BlueprintCallable)
	void SetStartOffset(float InStartOffset);

	UFUNCTION(BlueprintCallable)
	float GetStartOffset();

	//Must be a float value Between 1 - 0
	UFUNCTION(BlueprintCallable)
	void SetProgress(float InProgress);

	UFUNCTION(BlueprintCallable)
	float GetProgress();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void StartDroneMovement();
	void StartDroneMovement_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EndDroneMovement();
	void EndDroneMovement_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RestartDrone();
	void RestartDrone_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PauseDrone();
	void PauseDrone_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ResumeDrone();
	void ResumeDrone_Implementation();

	UPROPERTY(EditAnywhere)
	ADroneTrack* DroneTrack;

	UPROPERTY(EditAnywhere)
	float Duration;

	UPROPERTY(EditAnywhere)
	float StartOffset;

	UPROPERTY(EditAnywhere)
	float Progress;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
