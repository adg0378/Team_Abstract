// Copyright (c) 2025 Synetos Aerospace

#include "BasicDrone.h"
#include "SPUtility.h"
#include "State/SPGameState.h"
#include "Util/SPFlightTrail.h"
#include "SPDroneManager.h"

ABasicDrone::ABasicDrone()
{
	IsoSpringArm = CreateDefaultSubobject<UCesiumSpringArmComponent>(TEXT("IsometricSpringArmComponent"));
	DroneEyeSpringArm = CreateDefaultSubobject<UCesiumSpringArmComponent>(TEXT("DroneEyeSpringArmComponent"));
	FrontSpringArm = CreateDefaultSubobject<UCesiumSpringArmComponent>(TEXT("FrontSpringArmComponent"));
	IsoCamZoom = 5000.0f;
	EYECamZoom = -300.0f;
	FrontCamZoom = -700.0f;

	IsoSpringArm->SetupAttachment(RootComponent);
	IsoSpringArm->TargetArmLength = IsoCamZoom;
	IsoSpringArm->bDoCollisionTest = false;
	IsoSpringArm->bEnableCameraLag = true;
	IsoSpringArm->CameraLagSpeed = 4.0f;
	IsoSpringArm->CameraLagMaxDistance = 0.0f;
	IsoSpringArm->SetWorldRotation(IsoCamRotateDefault);
	IsoSpringArm->bUsePawnControlRotation = true;

	IsoCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("IsometricCameraComponent"));
	IsoCamera->SetupAttachment(IsoSpringArm, UCesiumSpringArmComponent::SocketName);
	IsoCamera->bUsePawnControlRotation = false;

	DroneEyeSpringArm->SetupAttachment(RootComponent);
	DroneEyeSpringArm->TargetArmLength = EYECamZoom;
	DroneEyeSpringArm->bDoCollisionTest = false;
	DroneEyeSpringArm->bEnableCameraLag = true;
	DroneEyeSpringArm->CameraLagSpeed = 4.0f;
	DroneEyeSpringArm->CameraLagMaxDistance = 0.0f;
	DroneEyeSpringArm->SetWorldRotation(DroneEyeCamRotateDefault);
	DroneEyeSpringArm->bUsePawnControlRotation = true;

	DroneEyeCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DroneEyeCameraComponent"));
	DroneEyeCamera->SetupAttachment(DroneEyeSpringArm, UCesiumSpringArmComponent::SocketName);
	DroneEyeCamera->bUsePawnControlRotation = false;

	FrontSpringArm->SetupAttachment(RootComponent);
	FrontSpringArm->TargetArmLength = FrontCamZoom;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraLag = true;
	FrontSpringArm->CameraLagSpeed = 4.0f;
	FrontSpringArm->CameraLagMaxDistance = 0.0f;
	FrontSpringArm->SetWorldRotation(FrontCamRotateDefault);
	FrontSpringArm->bUsePawnControlRotation = true;

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FrontCameraComponent"));
	FrontCamera->SetupAttachment(FrontSpringArm, UCesiumSpringArmComponent::SocketName);
	FrontCamera->bUsePawnControlRotation = false;

	idleMode = true;
}

void ABasicDrone::BeginPlay()
{
	Super::BeginPlay();

	DeactiveCameras();
	IsoCamera->Activate();
	CurrentDroneCam = EDroneCameraType::Isometric;

	Trail = GetWorld()->SpawnActor<ASPFlightTrail>();

	if (Trail)
	{
		Trail->SetTrailColor(FLinearColor::Blue);
		Trail->SetTrailLimit(-1);
	}
}

int32 ABasicDrone::GetID() const
{
	return ID;
}

void ABasicDrone::SetID(const int32 InID)
{
	ID = InID;
}

int32 ABasicDrone::GetSwarmID() const
{
	return SwarmID;
}

void ABasicDrone::SetSwarmID(const int32 InSwarmID)
{
	SwarmID = InSwarmID;
}

void ABasicDrone::ReassignSwarm(const int32 NewSwarmID)
{
	ASPGameState *GameState = USPUtility::GetSPGameState(this);
	GameState->droneManagerRef->ReassignDrone(ID, SwarmID, NewSwarmID);
}

FString ABasicDrone::GetName() const
{
	return Name;
}

void ABasicDrone::SetName(const FString InName)
{
	Name = InName;
	IInteractionBoxProvider::Execute_OnInteractionBoxTitleChanged(this, FText::FromString(InName));
}

void ABasicDrone::RenameDrone(const FString NewName)
{
	ASPGameState *GameState = USPUtility::GetSPGameState(this);
	GameState->droneManagerRef->RenameDrone(ID, SwarmID, NewName);
}

bool ABasicDrone::IsSimulated() const
{
	return Simulated;
}

void ABasicDrone::Select()
{
	Super::Select();
	USPDroneManager *DroneManager = USPUtility::GetSPGameState(this)->droneManagerRef;
	DroneManager->setSelectedDroneId(ID);
	UE_LOG(LogTemp, Log, TEXT("Selected drone %s"), *Name);
}

void ABasicDrone::SetPosition(const FCCSimPosition &PositionData)
{
	Position = PositionData;
	cesiumGlobeAnchor->MoveToLongitudeLatitudeHeight(FVector(Position.lon, Position.lat, 300.0f + Position.alt_msl));

	// Subtract 90 from yaw because we are setting EastSouthUp. Subtract -90 to set based on North
	this->cesiumGlobeAnchor->SetEastSouthUpRotation(FQuat::MakeFromEuler(FVector(Position.roll, Position.pitch, Position.yaw - 90)));
}

void ABasicDrone::SetPreferences(const FCCSimPreferencesStruct &InPrefs, const FDronePreferencesStruct &InDronePrefs)
{
	Trail->SetTrailLimit(InDronePrefs.DroneFlightTrailLength);
}

void ABasicDrone::SetStatus(const FCCSimStatus &StatusData)
{
	Status = StatusData;
	if (isSelected)
	{
		ASPGameState *GameState = USPUtility::GetSPGameState(this);
		GameState->RefreshSelected();
	}

	if (Trail)
	{
		Trail->AddTrailPointCoord(FVector(Position.lon, Position.lat, cesiumGlobeAnchor->GetLongitudeLatitudeHeight().Z));
	}
}

void ABasicDrone::ToggleCull_Implementation(bool IsCulled, bool FromFlyTo)
{
	Super::ToggleCull_Implementation(IsCulled, FromFlyTo);

	if (!IsCulled && FromFlyTo)
	{
		Trail->ResetTrail();
	}
}

FCCSimStatus ABasicDrone::GetStatus()
{
	return Status;
}

void ABasicDrone::SetTrailLength(int Length)
{
	Trail->SetTrailLimit(Length);
}

FCCSimPosition ABasicDrone::GetPosition() const
{
	return Position;
}

void ABasicDrone::GetInteractionBoxTitle_Implementation(FText &OutTitle)
{
	OutTitle = FText::FromString(Name);
}

void ABasicDrone::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue> &OutKeyVals)
{
	ASPGameState *gameState = USPUtility::GetSPGameState(this);
	FString SwarmName = gameState->droneManagerRef->GetSwarmName(SwarmID);

	OutKeyVals.Add(TEXT("Swarm"), FInteractionBoxValue{
																		.displayType = EKeyValDisplayType::UneditableText,
																		.value = FText::FromString(SwarmName)});

	OutKeyVals.Add(TEXT("Coords"), FInteractionBoxValue{
																		 .displayType = EKeyValDisplayType::UneditableText,
																		 .value = FText::FromString(FString::Printf(TEXT("%f, %f, %f"), Position.lat, Position.lon, Position.alt_msl))});

	OutKeyVals.Add(TEXT("Euler"), FInteractionBoxValue{
																		.displayType = EKeyValDisplayType::UneditableText,
																		.value = FText::FromString(FString::Printf(TEXT("%f, %f, %f"), Position.pitch, Position.roll, Position.yaw))});

	OutKeyVals.Add(TEXT("Battery remaining"), FInteractionBoxValue{
																								.displayType = EKeyValDisplayType::UneditableText,
																								.value = FText::FromString(FString::Printf(TEXT("%.2f%%"), Status.battery_remaining))});
}

void ABasicDrone::DestroySelf_Implementation()
{
	ASPGameState *GameState = USPUtility::GetSPGameState(this);
	GameState->droneManagerRef->RemoveDrone(ID, SwarmID);
	Super::DestroySelf_Implementation();
}

void ABasicDrone::ActivateIsoCam()
{
	DeactiveCameras();
	IsoCamera->Activate();
	CurrentDroneCam = EDroneCameraType::Isometric;
}

void ABasicDrone::ActivateFrontCam()
{
	DeactiveCameras();
	FrontCamera->Activate();
	CurrentDroneCam = EDroneCameraType::Front;
}

void ABasicDrone::ActivateEYECam()
{
	DeactiveCameras();
	DroneEyeCamera->Activate();
	CurrentDroneCam = EDroneCameraType::DroneEye;
}

void ABasicDrone::ResetIsoCamToDefault()
{
	IsoSpringArm->TargetArmLength = 5000;
	IsoSpringArm->SetWorldRotation(IsoCamRotateDefault);
}

void ABasicDrone::ResetFrontCamToDefault()
{
	FrontSpringArm->TargetArmLength = -700.f;
	FrontSpringArm->SetWorldRotation(FrontCamRotateDefault);
}

void ABasicDrone::ResetEYECamToDefault()
{
	DroneEyeSpringArm->TargetArmLength = -300.f;
	DroneEyeSpringArm->SetWorldRotation(DroneEyeCamRotateDefault);
}

void ABasicDrone::DeactiveCameras()
{
	IsoCamera->Deactivate();
	DroneEyeCamera->Deactivate();
	FrontCamera->Deactivate();
}

EDroneCameraType ABasicDrone::GetCurrentDroneCamType()
{
	return CurrentDroneCam;
}

void ABasicDrone::GetZoomPercentage_Implementation(float &OutZoomPercentage) const
{
	switch (CurrentDroneCam)
	{
	case EDroneCameraType::Isometric:
		OutZoomPercentage = (IsoCamZoom - IsoMinZoom) / (IsoMaxZoom - IsoMinZoom);
		break;

	case EDroneCameraType::DroneEye:
		OutZoomPercentage = (-EYECamZoom + EYEMaxZoom) / (EYEMaxZoom - EYEMinZoom);
		break;

	case EDroneCameraType::Front:
		OutZoomPercentage = (-FrontCamZoom + FrontMaxZoom) / (FrontMaxZoom - FrontMinZoom);
		break;
	}
}

void ABasicDrone::GetCameraLocation_Implementation(FVector &OutCameraLocation) const
{
	switch (CurrentDroneCam)
	{
	case EDroneCameraType::Isometric:
		OutCameraLocation = IsoSpringArm->GetComponentLocation();
		break;

	case EDroneCameraType::DroneEye:
		OutCameraLocation = DroneEyeSpringArm->GetComponentLocation();
		break;

	case EDroneCameraType::Front:
		OutCameraLocation = FrontSpringArm->GetComponentLocation();
		break;
	}
}

void ABasicDrone::GetCameraRotation_Implementation(FRotator &OutCameraRotation) const
{
	switch (CurrentDroneCam)
	{
	case EDroneCameraType::Isometric:
		OutCameraRotation = IsoSpringArm->GetTargetRotation();
		break;

	case EDroneCameraType::DroneEye:
		OutCameraRotation = DroneEyeSpringArm->GetTargetRotation();
		break;

	case EDroneCameraType::Front:
		OutCameraRotation = FrontSpringArm->GetTargetRotation();
		break;
	}
}

void ABasicDrone::GetDevicePitch_Implementation(float &OutDevicePitch) const
{
	OutDevicePitch = this->GetActorRotation().Pitch / 100.0f;
}

void ABasicDrone::GetDeviceRoll_Implementation(float &OutDeviceRoll) const
{
	OutDeviceRoll = this->GetActorRotation().Roll;
}

void ABasicDrone::GetDeviceYaw_Implementation(float &OutDeviceYaw) const
{
	OutDeviceYaw = this->GetActorRotation().Yaw;
}

void ABasicDrone::GetDeviceName_Implementation(FString &OutDeviceName) const
{
	OutDeviceName = GetName();
}