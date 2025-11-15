// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Delegates/DelegateCombinations.h"
#include "SPPreferences.generated.h"

USTRUCT(BlueprintType)
struct FWorldPreferencesStruct {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SolarTime = 14.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Day = 21;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Month = 9;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Year = 2025;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TilesetLevelOfDetail = 32.0;

	bool operator!=(const FWorldPreferencesStruct& Other) const {
		return (
			SolarTime != Other.SolarTime ||
			Day != Other.Day ||
			Month != Other.Month ||
			Year != Other.Year
			);
	}
};

USTRUCT(BlueprintType)
struct FObstaclesPreferencesStruct {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool ShowFAAObstacles = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool ShowADSB = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool RenderGroundedADSB = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool InterpolateADSBLocations = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int ADSBFlightTrailLength = 8;
};

USTRUCT(BlueprintType)
struct FCameraPreferencesStruct {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CameraFOVDeg = 90.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool InstantFlyTo = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Sensitivity = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool ShowCameraAnchorObject = false;
};

USTRUCT(BlueprintType)
struct FDronePreferencesStruct {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int DroneFlightTrailLength = -1;

	bool operator!=(const FDronePreferencesStruct& Other) const {
		return DroneFlightTrailLength != Other.DroneFlightTrailLength;
	}
};

USTRUCT(BlueprintType)
struct FCCSimPreferencesStruct {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Enabled = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Host = "127.0.0.1";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Port = 8765;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool ShowAltitudeReferenceLineTraces = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AltitudeRefreshDistanceM = 10.0;

	bool operator!=(const FCCSimPreferencesStruct& Other) const {
		return (
			Enabled != Other.Enabled ||
			Host != Other.Host ||
			Port != Other.Port ||
			ShowAltitudeReferenceLineTraces != Other.ShowAltitudeReferenceLineTraces ||
			AltitudeRefreshDistanceM != AltitudeRefreshDistanceM
			);
	}
};

USTRUCT(BlueprintType)
struct FSPPreferencesStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FObstaclesPreferencesStruct ObstaclesPreferences;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCameraPreferencesStruct CameraPreferences;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FWorldPreferencesStruct WorldPreferences;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FDronePreferencesStruct DronePreferences;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCCSimPreferencesStruct CCSimPreferences;
};

// Broadcasted when preferences are applied
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPreferencesAppliedDelegate, FSPPreferencesStruct, PrevPreferences, FSPPreferencesStruct, NewPreferences);

/**
 * Stores preferences and manages changes
 * Should be initialized before other state managers so they can subscribe to preference updates
 */
UCLASS()
class SKYPLEXC2_API USPPreferences : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void Setup();

	UFUNCTION(BlueprintCallable)
	void Teardown();

	UPROPERTY(BlueprintAssignable, Category = "Preferences")
	FPreferencesAppliedDelegate OnPreferencesUpdated;

	/** Gets a copy of current preferences */
	UFUNCTION(BlueprintCallable)
	FSPPreferencesStruct GetPreferences() const;

	/** Gets a readonly ref of current preferences */
	UFUNCTION(BlueprintCallable)
	const FSPPreferencesStruct& GetPreferencesRef() const;

	/** Applies and broadcasts new preferences.
	 * This should not be called for each individual preference update.
	 * Batch only, prefferably when preferences menu is closed or preference changes are applied.
	 */
	UFUNCTION(BlueprintCallable)
	void SetPreferences(const FSPPreferencesStruct& NewPreferences);

private:
	void WritePreferences() const;
	void ReadPreferences();

	UPROPERTY()
	TObjectPtr<class USPLogger> LOG;

	UPROPERTY()
	TObjectPtr<class ASPGameState> GameStateRef;

	FSPPreferencesStruct Preferences;
	const FString LOG_ORIGIN = TEXT("PreferencesManager");
};
