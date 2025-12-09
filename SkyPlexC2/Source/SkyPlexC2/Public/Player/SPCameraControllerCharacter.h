// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SPEnvConstants.h"
#include "SPCameraInterface.h"
#include "SPUtility.h"
#include "State/SPGameState.h"
#include "SPDroneManager.h"
#include "BasicDrone.h"
#include "Math/UnrealMathUtility.h"
#include "SPCameraControllerCharacter.generated.h"

// Forward declarations
class ASPGameState;
class ABasicDrone;

UENUM(BlueprintType)
enum class ECameraType : uint8
{
	Isometric UMETA(DisplayName = "Isometric"),
	TopDown UMETA(DisplayName = "TopDown"),
	FirstPerson UMETA(DisplayName = "FirstPerson"),
	DroneMode UMETA(DisplayName = "DroneMode")
};

USTRUCT(BlueprintType)
struct FIsometricCameraProps {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Pitch;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ArmLength;
};

USTRUCT(BlueprintType)
struct FCameraLocationData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Lon;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Lat;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Height;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ArmLength;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Roll;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Pitch;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Yaw;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FIsometricCameraProps IsometricCameraProps;
};

UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API ASPCameraControllerCharacter : public ACharacter, public ISPCameraInterface
{
	GENERATED_BODY()

public:	
	UFUNCTION(BlueprintCallable)
	void WriteCameraLocationData(FCameraLocationData Data);

	UFUNCTION(BlueprintCallable)
	void ReadCameraLocationData(FCameraLocationData& OutData, bool& OutFailed);

	//to do: remove once camera controller is fully in c++
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SwapToDroneCams(int32 droneID, int32 swarmID);
	void SwapToDroneCams_Implementation(int32 droneID, int32 swarmID);

	//to do: remove once camera controller is fully in c++
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SwapToPlayerCam();
	void SwapToPlayerCam_Implementation();

	UFUNCTION(BlueprintCallable)
	void SwapToWorldCam();

	UFUNCTION(BlueprintCallable)
	void SwapToDroneCam(int32 droneID, int32 swarmID);

	//UFUNCTION(BlueprintCallable)
	//void StartDroneIdle();

	//UFUNCTION(BlueprintCallable)
	//void EndDroneIdle();

	UFUNCTION(BlueprintCallable)
	void UpdateDroneCamOrbit(float actionValue);

	UFUNCTION(BlueprintCallable)
	void UpdateDroneCamPitch(float actionValue);

	UFUNCTION(BlueprintCallable)
	void UpdateDroneCamZoom(float actionValue);

	virtual void GetZoomPercentage_Implementation(float& OutZoomPercentage) const;
	virtual void GetCameraLocation_Implementation(FVector& OutCameraLocation) const;
	virtual void GetCameraRotation_Implementation(FRotator& OutCameraRotation) const;
	virtual void GetDevicePitch_Implementation(float& OutDevicePitch) const;
	virtual void GetDeviceRoll_Implementation(float& OutDeviceRoll) const;
	virtual void GetDeviceYaw_Implementation(float& OutDeviceYaw) const;
	virtual void GetDeviceName_Implementation(FString& OutDeviceName) const;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Zoom = 3000.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MinZoom = 200.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxZoom = 700000.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ECameraType CurrWorldCam;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ECameraType CurrActiveCam;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SinVariableDroneIdle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 CurrDroneID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 CurrSwarmID;

private:
	ASPGameState* GameStateRef = USPUtility::GetSPGameState(this);
};
