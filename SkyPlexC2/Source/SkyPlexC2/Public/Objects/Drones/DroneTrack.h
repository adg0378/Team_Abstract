// Copyright (c) 2025 Synetos Aerospace

#pragma once

// Add import paths. Make sure they go above the PlaneTrack.generated.h line
#include "Components/SplineComponent.h"
#include "CesiumGeoreference.h"
#include "Engine/DataTable.h"
#include <CesiumGeospatial/Cartographic.h>
#include <CesiumGeospatial/Ellipsoid.h>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DroneTrack.generated.h"

USTRUCT(BlueprintType)
struct FAircraftRawData : public FTableRowBase {
	GENERATED_USTRUCT_BODY()

public:
	FAircraftRawData()
		: Longitude(0.0)
		, Latitude(0.0)
		, Height(0.0)
	{}

	UPROPERTY(EditAnywhere, Category = "DroneTrack")
	double Longitude;
	UPROPERTY(EditAnywhere, Category = "DroneTrack")
	double Latitude;
	UPROPERTY(EditAnywhere, Category = "DroneTrack")
	double Height;
};

UCLASS()
class SKYPLEXC2_API ADroneTrack : public AActor
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Sets default values for this actor's properties
	ADroneTrack();

	UFUNCTION(BlueprintCallable, Category = "DroneTrack")
	void DestroyTrack();
	// Spline variable to represent the plane track
	UPROPERTY(BlueprintReadOnly, Category = "DroneTrack")
	USplineComponent* SplineTrack;

	// Cesium class that contains useful functions for coordinate conversion
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DroneTrack")
	ACesiumGeoreference* CesiumGeoreference;

	// An Unreal Engine data table to store the raw flight data
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DroneTrack")
	UDataTable* AircraftsRawDataTable;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Function to parse the data table and create the spline track
	UFUNCTION(BlueprintCallable, Category = "DroneTrack")
	void LoadSplineTrackPoints();

	UFUNCTION(BlueprintCallable, Category = "DroneTrack")
	void LoadTrackByCSVFile(FString FilePath);

	UFUNCTION(BlueprintCallable, Category = "DroneTrack")
	void ClearSplineTrackPoints();

	UFUNCTION(BlueprintCallable, Category = "DroneTrack")
	int32 GetPointCount();

private:
	int32 PointCount;

};
