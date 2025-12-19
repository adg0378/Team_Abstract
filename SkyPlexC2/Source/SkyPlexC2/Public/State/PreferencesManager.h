// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "SPEnvConstants.h"
#include "PreferencesManager.generated.h"

// Forward declarations
class USPLogger;
class ASPGameState;

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

USTRUCT()
struct FHiddenConfigPreferencesStruct {
	GENERATED_BODY()

public:
	UPROPERTY()
	double SimOriginLon = 8.546163739800146;

	UPROPERTY()
	double SimOriginLat = 47.397971057728974;

	UPROPERTY()
	float SimulationRadiusNM = 2.7f;
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

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString PX4AutopilotPath = TEXT("~/PX4-Autopilot");

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString PX4PreCommand = TEXT("");

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString CCSimPath = TEXT("~/cc-simulator");

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString ROS2Path = TEXT("/opt/ros/humble/setup.bash");

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

	UPROPERTY()
	FHiddenConfigPreferencesStruct ConfigPreferences;
};

// Broadcasted when preferences are applied
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPreferencesAppliedDelegate, FSPPreferencesStruct, PrevPreferences, FSPPreferencesStruct, NewPreferences);

/**
 * Class to manage all preferences and user-settings of SkyPlex.
 * Should be initialized first so other managers can initialize based on preferences.
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API UPreferencesManager : public UObject
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
	void SetPreferences(UPARAM(ref) const FSPPreferencesStruct& NewPreferences);

private:
	void WritePreferences() const;
	void ReadPreferences();

	UPROPERTY()
	TObjectPtr<USPLogger> LOG;

	UPROPERTY()
	TObjectPtr<ASPGameState> GameStateRef;

	FSPPreferencesStruct Preferences;

	const FString LOG_ORIGIN = TEXT("PreferencesManager");
};
