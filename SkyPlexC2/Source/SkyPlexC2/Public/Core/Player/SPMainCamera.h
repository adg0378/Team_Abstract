// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SPMainCamera.generated.h"

UENUM(BlueprintType)
enum class ECameraType : uint8
{
	Isometric UMETA(DisplayName = "Isometric"),
	TopDown UMETA(DisplayName = "TopDown"),
	FirstPerson UMETA(DisplayName = "FirstPerson"),
};

USTRUCT()
struct FIsometricCameraProps {
	GENERATED_BODY()

public:
	UPROPERTY()
	double Pitch = 300.0;

	UPROPERTY()
	float ArmLength = 3000.0f;
};

USTRUCT()
struct FCameraLocationData
{
	GENERATED_BODY()
public:
	UPROPERTY()
	double Lon;

	UPROPERTY()
	double Lat;

	UPROPERTY()
	double Height;

	UPROPERTY()
	float ArmLength;

	UPROPERTY()
	double Roll;

	UPROPERTY()
	double Pitch;

	UPROPERTY()
	double Yaw;
};

UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API ASPMainCamera : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASPMainCamera();

	UFUNCTION(BlueprintCallable, Category = "SPMainCamera", meta = (WorldContext = "WorldContextObject"))
	static ASPMainCamera* GetSPMainCamera(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable)
	void SetFOV(float FOVDegrees);

	UFUNCTION(BlueprintCallable)
	void CameraMovement(FVector Magnitude);

	UFUNCTION(BlueprintCallable)
	void CameraZoom(float Magnitude);

	UFUNCTION(BlueprintCallable)
	void CameraOrbit(float Magnitude);

	UFUNCTION(BlueprintCallable)
	void CameraPitch(float Magnitude);

	UFUNCTION(BlueprintCallable)
	void SetCameraTypeFromIndex(uint8 Index);

	UFUNCTION(BlueprintCallable)
	void SetCameraType(ECameraType Type);

	UFUNCTION(BlueprintCallable)
	ECameraType GetCameraType() const;

	/* Calculates the correct height */
	UFUNCTION(BlueprintCallable)
	void FlyTo(float Lon, float Lat);

	/* Flies to height passed in plus restricted height above ellipsoid. Use FlyTo unless the height passed in has been verified with the ellipsoid */
	UFUNCTION(BlueprintCallable)
	void FlyToHeight(FVector LonLatHeight);

	UFUNCTION(BlueprintCallable)
	void InstantTeleport(FVector LonLatHeight);

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CubeComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UCesiumSpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UCesiumFlyToComponent> FlyToComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UCesiumGlobeAnchorComponent> GlobeAnchorComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UCesiumOriginShiftComponent> OriginShiftComponent;

	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	/* Distance the camera anchor should stay above the tileset */
	const float DistanceAboveEarthCM = 150.0f;

	ECameraType CameraType = ECameraType::Isometric;
	float MovementSpeed = 0.2f;
	float Zoom = 3000.0f;
	float MinZoom = 200.0f;
	float MaxZoom = 700000.0f;
	float ZoomSpeed = 300.0f;
	float BaseZoomSpeed = 300.0f;
	float BaseMovementSpeed = 2500.0f;
	float OrbitSpeed = 1.0;
	float MinPitch = 270.001f;
	float MaxPitch = 0.0f;
	float DefaultMaxZoom = 700000.0f;
	float DefaultMaxPitch = 0.0f;
	float FirstPersonMaxZoom = 100000.0f;
	float FirstPersonMaxPitch = 89.999;
	float MaxFlyToSpeed = 400000.0f;
	float MaxFlyToTimeS = 5.0f;
	const FVector DefaultStartLocation = FVector(-92.748316, 44.029555, 352.357261);

	FIsometricCameraProps IsoCamProps;

	UPROPERTY()
	FTimerHandle InstantTeleportDelayHandle;

	void SaveCameraLocationData();
	void LoadCameraLocationData();
	void CalculateZMovement(float ZMagnitude, FVector& WorldDirection, float& Scale, bool& Valid);
	void PerformZoomDependentUpdates();
	void SetIsometricCamera();
	void SetTopDownCamera();
	void SetFirstPersonCamera();
	void OnInstantTeleportDelayFinished();

	UFUNCTION()
	void OnFlightCompleted();
};
