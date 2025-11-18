// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Objects/SPInteractable.h"
#include "Core/Player/SPCameraInterface.h"
#include "Objects/Drones/SPCCSimMessenger.h"
#include "SPBaseDrone.generated.h"


UENUM(BlueprintType)
enum class EDroneCameraType : uint8
{
	Isometric UMETA(DisplayName = "Isometric"),
	DroneEye UMETA(DisplayName = "DroneEye"),
	Front UMETA(DisplayName = "Front"),
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API ASPBaseDrone : public ASPInteractable, public ISPCameraInterface
{
	GENERATED_BODY()
	
public:
	ASPBaseDrone();

	UFUNCTION(BlueprintCallable)
	int32 GetID() const;

	UFUNCTION(BlueprintCallable)
	void SetID(const int32 InID);

	UFUNCTION(BlueprintCallable)
	int32 GetSwarmID() const;

	/** Does not reassign externally. Use ReassignSwarm instead */
	UFUNCTION(BlueprintCallable)
	void SetSwarmID(const int32 InSwarmID);

	UFUNCTION(BlueprintCallable)
	void ReassignSwarm(const int32 NewSwarmID);

	UFUNCTION(BlueprintCallable)
	FString GetName() const;

	/** Does not rename externally. Use RenameDrone instead */
	UFUNCTION(BlueprintCallable)
	void SetName(const FString InName);

	UFUNCTION(BlueprintCallable)
	void RenameDrone(const FString NewName);

	UFUNCTION(BlueprintCallable)
	bool IsSimulated() const;

	virtual void SetPreferences(const struct FCCSimPreferencesStruct& InPrefs, const struct FDronePreferencesStruct& InDronePrefs);

	virtual void SetPosition(const FCCSimPosition& PositionData);
	virtual void SetStatus(const FCCSimStatus& StatusData);

	virtual FCCSimStatus GetStatus();

	UFUNCTION(BlueprintCallable)
	void SetTrailLength(int Length);

	UFUNCTION(BlueprintCallable)
	FCCSimPosition GetPosition() const;

	virtual void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) override;
	virtual void GetInteractionBoxTitle_Implementation(FText& OutTitle) override;
	virtual void DestroySelf_Implementation() override;

	UFUNCTION(BlueprintCallable)
	void ActivateIsoCam();

	UFUNCTION(BlueprintCallable)
	void ActivateFrontCam();

	UFUNCTION(BlueprintCallable)
	void ActivateEYECam();

	UFUNCTION(BlueprintCallable)
	void ResetIsoCamToDefault();

	UFUNCTION(BlueprintCallable)
	void ResetFrontCamToDefault();

	UFUNCTION(BlueprintCallable)
	void ResetEYECamToDefault();

	UFUNCTION(BlueprintCallable)
	void DeactiveCameras();

	UFUNCTION(BlueprintCallable)
	EDroneCameraType GetCurrentDroneCamType();

	virtual void GetZoomPercentage_Implementation(float& OutZoomPercentage) const;
	virtual void GetCameraLocation_Implementation(FVector& OutCameraLocation) const;
	virtual void GetCameraRotation_Implementation(FRotator& OutCameraRotation) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* IsoCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> IsoSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* DroneEyeCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> DroneEyeSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FrontCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> FrontSpringArm;
protected:
	virtual void BeginPlay() override;

	static const bool Simulated = false;

	int32 ID = -1;
	int32 SwarmID = -1;
	FString Name = "Unnamed";

	FCCSimPosition Position;
	FCCSimStatus Status;

	const FRotator IsoCamRotateDefault = FRotator(335, 215, 0);
	const FRotator DroneEyeCamRotateDefault = FRotator(271, 0, 0);
	const FRotator FrontCamRotateDefault = FRotator(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float IsoCamZoom;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float EYECamZoom;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float FrontCamZoom;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float IsoMinZoom = 3000.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float EYEMinZoom = -2000.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float FrontMinZoom = -2500.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float IsoMaxZoom = 12000.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float EYEMaxZoom = -300.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float FrontMaxZoom = -700.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool idleMode;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EDroneCameraType CurrentDroneCam;

	UPROPERTY()
	TObjectPtr<class ASPFlightTrail> Trail;
};
