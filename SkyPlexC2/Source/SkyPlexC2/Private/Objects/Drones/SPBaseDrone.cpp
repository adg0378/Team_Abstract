// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Drones/SPBaseDrone.h"
#include "Core/Player/CesiumSpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Objects/Misc/SPFlightTrail.h"
#include "Core/State/SPGameState.h"
#include "Core/State/SPDroneManager.h"
#include "Core/State/SPSelectionManager.h"
#include "Core/SPPreferences.h"
#include "CesiumGlobeAnchorComponent.h"

ASPBaseDrone::ASPBaseDrone() {
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

void ASPBaseDrone::BeginPlay() {
	Super::BeginPlay();

	DeactiveCameras();
	IsoCamera->Activate();
	CurrentDroneCam = EDroneCameraType::Isometric;

	Trail = GetWorld()->SpawnActor<ASPFlightTrail>();

	if (Trail) {
		Trail->SetTrailColor(FLinearColor::Blue);
		Trail->SetTrailLimit(-1);
	}
}

int32 ASPBaseDrone::GetID() const {
	return ID;
}

void ASPBaseDrone::SetID(const int32 InID) {
	ID = InID;
}

int32 ASPBaseDrone::GetSwarmID() const {
	return SwarmID;
}

void ASPBaseDrone::SetSwarmID(const int32 InSwarmID) {
	SwarmID = InSwarmID;
}

void ASPBaseDrone::ReassignSwarm(const int32 NewSwarmID) {
	ASPGameState* GameState = ASPGameState::GetSPGameState(this);
	GameState->DroneManager->ReassignDrone(ID, SwarmID, NewSwarmID);
}

FString ASPBaseDrone::GetName() const {
	return Name;
}

void ASPBaseDrone::SetName(const FString InName) {
	Name = InName;
	ISPInteractionInterface::Execute_OnInteractionBoxTitleChanged(this, FText::FromString(InName));
}

void ASPBaseDrone::RenameDrone(const FString NewName) {
	ASPGameState* GameState = ASPGameState::GetSPGameState(this);
	GameState->DroneManager->RenameDrone(ID, SwarmID, NewName);
}

bool ASPBaseDrone::IsSimulated() const {
	return Simulated;
}

void ASPBaseDrone::SetPosition(const FCCSimPosition& PositionData) {
	Position = PositionData;
	GlobeAnchorComponent->MoveToLongitudeLatitudeHeight(FVector(Position.lon, Position.lat, 300.0f + Position.alt_msl));

	// Subtract 90 from yaw because we are setting EastSouthUp. Subtract -90 to set based on North
	this->GlobeAnchorComponent->SetEastSouthUpRotation(FQuat::MakeFromEuler(FVector(Position.roll, Position.pitch, Position.yaw - 90)));
}

void ASPBaseDrone::SetPreferences(const FCCSimPreferencesStruct& InPrefs, const FDronePreferencesStruct& InDronePrefs) {
	Trail->SetTrailLimit(InDronePrefs.DroneFlightTrailLength);
}

void ASPBaseDrone::SetStatus(const FCCSimStatus& StatusData) {
	Status = StatusData;
	if (_IsSelected) {
		ASPGameState* GameState = ASPGameState::GetSPGameState(this);
		GameState->SelectionManager->RefreshSelected();
	}

	if (Trail) {
		Trail->AddTrailPointCoord(GlobeAnchorComponent->GetLongitudeLatitudeHeight());
	}
}

FCCSimStatus ASPBaseDrone::GetStatus() {
	return Status;
}

void ASPBaseDrone::SetTrailLength(int Length) {
	Trail->SetTrailLimit(Length);
}

FCCSimPosition ASPBaseDrone::GetPosition() const {
	return Position;
}

void ASPBaseDrone::GetInteractionBoxTitle_Implementation(FText& OutTitle) {
	OutTitle = FText::FromString(Name);
}

void ASPBaseDrone::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) {
	ASPGameState* gameState = ASPGameState::GetSPGameState(this);
	FString SwarmName = gameState->DroneManager->GetSwarmName(SwarmID);

	OutKeyVals.Add(TEXT("Swarm"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(SwarmName)
		});

	OutKeyVals.Add(TEXT("Coords"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(FString::Printf(TEXT("%f, %f, %f"), Position.lat, Position.lon, Position.alt_msl))
		});

	OutKeyVals.Add(TEXT("Euler"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(FString::Printf(TEXT("%f, %f, %f"), Position.pitch, Position.roll, Position.yaw))
		});

	OutKeyVals.Add(TEXT("Battery remaining"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(FString::Printf(TEXT("%.2f%%"), Status.battery_remaining))
		});
}

void ASPBaseDrone::DestroySelf_Implementation() {
	ASPGameState* GameState = ASPGameState::GetSPGameState(this);
	GameState->DroneManager->RemoveDrone(ID, SwarmID);
	Super::DestroySelf_Implementation();
}

void ASPBaseDrone::ActivateIsoCam() {
	DeactiveCameras();
	IsoCamera->Activate();
	CurrentDroneCam = EDroneCameraType::Isometric;
}

void ASPBaseDrone::ActivateFrontCam() {
	DeactiveCameras();
	FrontCamera->Activate();
	CurrentDroneCam = EDroneCameraType::Front;
}

void ASPBaseDrone::ActivateEYECam() {
	DeactiveCameras();
	DroneEyeCamera->Activate();
	CurrentDroneCam = EDroneCameraType::DroneEye;
}

void ASPBaseDrone::ResetIsoCamToDefault() {
	IsoSpringArm->TargetArmLength = 5000;
	IsoSpringArm->SetWorldRotation(IsoCamRotateDefault);
}

void ASPBaseDrone::ResetFrontCamToDefault() {
	FrontSpringArm->TargetArmLength = -700.f;
	FrontSpringArm->SetWorldRotation(FrontCamRotateDefault);
}

void ASPBaseDrone::ResetEYECamToDefault() {
	DroneEyeSpringArm->TargetArmLength = -300.f;
	DroneEyeSpringArm->SetWorldRotation(DroneEyeCamRotateDefault);
}

void ASPBaseDrone::DeactiveCameras() {
	IsoCamera->Deactivate();
	DroneEyeCamera->Deactivate();
	FrontCamera->Deactivate();
}

EDroneCameraType ASPBaseDrone::GetCurrentDroneCamType() {
	return CurrentDroneCam;
}

void ASPBaseDrone::GetZoomPercentage_Implementation(float& OutZoomPercentage) const {
	switch (CurrentDroneCam) {
	case EDroneCameraType::Isometric:
		OutZoomPercentage = (IsoCamZoom - IsoMinZoom) / (IsoMaxZoom - IsoMinZoom);

		break;
	case EDroneCameraType::DroneEye:
		OutZoomPercentage = (EYECamZoom - EYEMinZoom) / (EYEMaxZoom - EYEMinZoom);

		break;
	case EDroneCameraType::Front:
		OutZoomPercentage = (FrontCamZoom - FrontMinZoom) / (FrontMinZoom - FrontMaxZoom);

		break;
	}
}


void ASPBaseDrone::GetCameraLocation_Implementation(FVector& OutCameraLocation) const {
	switch (CurrentDroneCam) {
	case EDroneCameraType::Isometric:
		OutCameraLocation = IsoSpringArm->GetRelativeLocation();

		break;
	case EDroneCameraType::DroneEye:
		OutCameraLocation = DroneEyeSpringArm->GetRelativeLocation();

		break;
	case EDroneCameraType::Front:
		OutCameraLocation = FrontSpringArm->GetRelativeLocation();

		break;
	}
}


void ASPBaseDrone::GetCameraRotation_Implementation(FRotator& OutCameraRotation) const {
	switch (CurrentDroneCam) {
	case EDroneCameraType::Isometric:
		OutCameraRotation = IsoSpringArm->GetRelativeRotation();

		break;
	case EDroneCameraType::DroneEye:
		OutCameraRotation = DroneEyeSpringArm->GetRelativeRotation();

		break;
	case EDroneCameraType::Front:
		OutCameraRotation = FrontSpringArm->GetRelativeRotation();

		break;
	}
}