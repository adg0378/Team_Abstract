// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "State/SPManager.h"
#include "FlyToLocationManager.generated.h"

// Forward declarations
struct FGoogleAPIAddressPredictionResult;
struct FGooglePlacesResult;

/** Stores location name and longLatHeight */
USTRUCT(BlueprintType)
struct FFlyToLocationData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector longLatHeight;

	bool operator==(const FFlyToLocationData& other) const
	{
		return name == other.name && longLatHeight == other.longLatHeight;
	}
};

USTRUCT(BlueprintType)
struct FFlyToLocations {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FFlyToLocationData> locations;
};

// Update UI when a location is added
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlyToLocationAddedDelegate, const FFlyToLocationData&, LocationData);

// Broadcast when address prediction info is recieved
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlyToAddressPredictionRecievedDelegate, const FGoogleAPIAddressPredictionResult&, AddressPredictions);

// Broadcast when place detail info is recieved
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlyToPlaceDetailsRecievedDelegate, const FGooglePlacesResult&, PlaceDetails);

/**
 * Stores saved locations for camera fly to
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API UFlyToLocationManager : public USPManager
{
	GENERATED_BODY()

public:
	UFlyToLocationManager();

	void Setup_Implementation(bool& outSuccess) override;

	UPROPERTY(BlueprintAssignable)
	FFlyToLocationAddedDelegate OnLocationAdded;

	UPROPERTY(BlueprintAssignable)
	FFlyToAddressPredictionRecievedDelegate OnAddressPredictionRecieved;

	UPROPERTY(BlueprintAssignable)
	FFlyToPlaceDetailsRecievedDelegate OnPlaceDetailsRecieved;

	UFUNCTION(BlueprintCallable)
	void GetLocations(TArray<FFlyToLocationData>& outLocations) const;

	UFUNCTION(BlueprintCallable)
	void DeleteLocation(FFlyToLocationData location);

	UFUNCTION(BlueprintCallable)
	void AddLocationFromCoords(FVector longLatHeight);

	UFUNCTION(BlueprintCallable)
	void RenameLocation(FFlyToLocationData location, FString newName, FFlyToLocationData& outLocation);

	UFUNCTION(BlueprintCallable)
	void UpdateLocationOfLocation(FFlyToLocationData location, FVector newLongLatHeight, FFlyToLocationData& outLocation);

	/** Search addresses. Bind to OnAddressPredictionsRecieved to get results */
	UFUNCTION(BlueprintCallable)
	void GetAddressPredictions(FString AddressSearchString);

	/** Finalizes address search and returns place details. Bind to OnPlaceDetailsRecieved to get results */
	UFUNCTION(BlueprintCallable)
	void GetPlaceDetails(FString PlaceID);

	/** Terminates the address prediction session. Not necessary, but highly recommended */
	UFUNCTION(BlueprintCallable)
	void EndAddressPredictionSession();

private:
	void WriteLocations();

	void LoadLocations();

	void AddLocation(FFlyToLocationData location);

	TArray<FFlyToLocationData> locations;

	uint16 LatestAddressPredictionRequestID = 0;
};
