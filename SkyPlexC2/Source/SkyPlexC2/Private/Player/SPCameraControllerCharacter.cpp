// Copyright (c) 2025 Synetos Aerospace


#include "Player/SPCameraControllerCharacter.h"
#include "FileUtility.h"
#include "JsonObjectConverter.h"
#include "State/SPWorldManager.h"
#include "State/SPGameState.h"
#include "State/SPManager.h"

void ASPCameraControllerCharacter::WriteCameraLocationData(FCameraLocationData Data) {
	FString JsonString;

	if (!FJsonObjectConverter::UStructToJsonObjectString<FCameraLocationData>(Data, JsonString)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to stringify camera location data"));
	}


	if (!FileUtility::WriteStringToFile(*USPEnvConstants::GetCameraLocationPath(), *JsonString)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to save camera location data"));
	}
}
void ASPCameraControllerCharacter::ReadCameraLocationData(FCameraLocationData& OutData, bool& OutFailed) {
	if (FPaths::FileExists(USPEnvConstants::GetCameraLocationPath())) {
		FString FileContents;

		if (!FFileHelper::LoadFileToString(FileContents, *USPEnvConstants::GetCameraLocationPath())) {
			UE_LOG(LogTemp, Error, TEXT("Failed to load camera location data"));
			OutFailed = true;
			return;
		}

		FCameraLocationData ParsedLocations;
		if (!FJsonObjectConverter::JsonObjectStringToUStruct<FCameraLocationData>(FileContents, &ParsedLocations, 0, 0)) {
			UE_LOG(LogTemp, Error, TEXT("Failed to parse camera location data"));
			OutFailed = true;
		}
		else {
			OutData = ParsedLocations;
			OutFailed = false;
		}
	}
	else {
		if (!FileUtility::WriteStringToFile(*USPEnvConstants::GetCameraLocationPath(), TEXT("{}"))) {
			UE_LOG(LogTemp, Error, TEXT("Failed to create camera location file"));
		}
		OutFailed = true;
	}
	
}

void ASPCameraControllerCharacter::SwapToDroneCams_Implementation(int32 droneID, int32 swarmID) {

}

void ASPCameraControllerCharacter::SwapToPlayerCam_Implementation() {

}

void ASPCameraControllerCharacter::GetZoomPercentage_Implementation(float& OutZoomPercentage) const {
	OutZoomPercentage = (Zoom - MinZoom) / (MaxZoom - MinZoom);
}


void ASPCameraControllerCharacter::GetCameraLocation_Implementation(FVector& OutCameraLocation) const {
	OutCameraLocation = GetActorLocation();
}


void ASPCameraControllerCharacter::GetCameraRotation_Implementation(FRotator& OutCameraRotation) const {
	OutCameraRotation = GetControlRotation();
}

void ASPCameraControllerCharacter::GetDevicePitch_Implementation(float& OutDevicePitch) const {
	OutDevicePitch = this->GetActorRotation().Pitch;
}

void ASPCameraControllerCharacter::GetDeviceRoll_Implementation(float& OutDeviceRoll) const {
	OutDeviceRoll = this->GetActorRotation().Roll;
}

void ASPCameraControllerCharacter::GetDeviceYaw_Implementation(float& OutDeviceYaw) const {
	OutDeviceYaw = -90;
}

void ASPCameraControllerCharacter::GetDeviceName_Implementation(FString& OutDeviceName) const {
	OutDeviceName = "World Camera";
}

void ASPCameraControllerCharacter::SwapToWorldCam() {
	CurrActiveCam = CurrWorldCam;
	GameStateRef->WorldManagerRef->SetActiveCamera(this);
}

void ASPCameraControllerCharacter::SwapToDroneCam(int32 droneID, int32 swarmID) {
	CurrDroneID = droneID;
	CurrSwarmID = swarmID;

	ABasicDrone* currDrone = GameStateRef->droneManagerRef->GetDrone(droneID, swarmID);
	GameStateRef->WorldManagerRef->SetActiveCamera(currDrone);

	if (CurrActiveCam != ECameraType::DroneMode) {
		CurrWorldCam = CurrActiveCam;
		CurrActiveCam = ECameraType::DroneMode;
	}
}

void ASPCameraControllerCharacter::UpdateDroneCamOrbit(float actionValue) {
	if (CurrActiveCam != ECameraType::DroneMode) {
		return;
	}

	ABasicDrone* currDrone = GameStateRef->droneManagerRef->GetDrone(CurrDroneID, CurrSwarmID);
	EDroneCameraType currDroneCam = currDrone->GetCurrentDroneCamType();
	FRotator newCamRotation;

	switch (currDroneCam) {
	case EDroneCameraType::Isometric:
		newCamRotation.Roll = currDrone->IsoSpringArm->GetTargetRotation().Roll;
		newCamRotation.Pitch = currDrone->IsoSpringArm->GetTargetRotation().Pitch;
		newCamRotation.Yaw = currDrone->IsoSpringArm->GetTargetRotation().Yaw
			+ (actionValue * 2);

		currDrone->IsoSpringArm->SetWorldRotation(newCamRotation);
		break;
	
	case EDroneCameraType::DroneEye:
		newCamRotation.Roll = currDrone->DroneEyeSpringArm->GetTargetRotation().Roll;
		newCamRotation.Pitch = currDrone->DroneEyeSpringArm->GetTargetRotation().Pitch;
		newCamRotation.Yaw = currDrone->DroneEyeSpringArm->GetTargetRotation().Yaw
			+ (actionValue * 2);

		currDrone->DroneEyeSpringArm->SetWorldRotation(newCamRotation);
		break;
	
	case EDroneCameraType::Front:
		newCamRotation.Roll = currDrone->FrontSpringArm->GetTargetRotation().Roll;
		newCamRotation.Pitch = currDrone->FrontSpringArm->GetTargetRotation().Pitch;
		newCamRotation.Yaw = currDrone->FrontSpringArm->GetTargetRotation().Yaw
			+ (actionValue * 2);

		newCamRotation.Yaw = FMath::ClampAngle(newCamRotation.Yaw, -45.0, 45.0);

		currDrone->FrontSpringArm->SetWorldRotation(newCamRotation);
		break;
	}

	currDrone->idleMode = false;
}

void ASPCameraControllerCharacter::UpdateDroneCamPitch(float actionValue) {
	if (CurrActiveCam != ECameraType::DroneMode) {
		return;
	}

	ABasicDrone* currDrone = GameStateRef->droneManagerRef->GetDrone(CurrDroneID, CurrSwarmID);
	EDroneCameraType currDroneCam = currDrone->GetCurrentDroneCamType();
	FRotator newCamRotation;

	switch (currDroneCam) {
	case EDroneCameraType::Isometric:
		newCamRotation.Roll = currDrone->IsoSpringArm->GetTargetRotation().Roll;
		newCamRotation.Pitch = currDrone->IsoSpringArm->GetTargetRotation().Pitch
			+ (actionValue * 2);
		newCamRotation.Yaw = currDrone->IsoSpringArm->GetTargetRotation().Yaw;

		newCamRotation.Pitch = FMath::ClampAngle(newCamRotation.Pitch, -89.0, 89.0);
		currDrone->IsoSpringArm->SetWorldRotation(newCamRotation);
		break;

	case EDroneCameraType::DroneEye:
		newCamRotation.Roll = currDrone->DroneEyeSpringArm->GetTargetRotation().Roll;
		newCamRotation.Pitch = currDrone->DroneEyeSpringArm->GetTargetRotation().Pitch
			+ (actionValue * 2);
		newCamRotation.Yaw = currDrone->DroneEyeSpringArm->GetTargetRotation().Yaw;

		newCamRotation.Pitch = FMath::ClampAngle(newCamRotation.Pitch, -89.0, -25.0);
		currDrone->DroneEyeSpringArm->SetWorldRotation(newCamRotation);
		break;

	case EDroneCameraType::Front:
		newCamRotation.Roll = currDrone->FrontSpringArm->GetTargetRotation().Roll;
		newCamRotation.Pitch = currDrone->FrontSpringArm->GetTargetRotation().Pitch
			+ (actionValue * 2);
		newCamRotation.Yaw = currDrone->FrontSpringArm->GetTargetRotation().Yaw;

		newCamRotation.Pitch = FMath::ClampAngle(newCamRotation.Pitch, -45.0, 45.0);
		currDrone->FrontSpringArm->SetWorldRotation(newCamRotation);
		break;
	}

	currDrone->idleMode = false;
}

void ASPCameraControllerCharacter::UpdateDroneCamZoom(float actionValue) {
	if (CurrActiveCam != ECameraType::DroneMode) {
		return;
	}

	ABasicDrone* currDrone = GameStateRef->droneManagerRef->GetDrone(CurrDroneID, CurrSwarmID);
	EDroneCameraType currDroneCam = currDrone->GetCurrentDroneCamType();
	float newCamArmLen;

	switch (currDroneCam) {
	case EDroneCameraType::Isometric:
		newCamArmLen = currDrone->IsoSpringArm->TargetArmLength
			+ (actionValue * -300.0f);

		newCamArmLen = FMath::Clamp(newCamArmLen, currDrone->IsoMinZoom, currDrone->IsoMaxZoom);
		currDrone->IsoCamZoom = newCamArmLen;
		currDrone->IsoSpringArm->TargetArmLength = newCamArmLen;
		break;

	case EDroneCameraType::DroneEye:
		newCamArmLen = currDrone->DroneEyeSpringArm->TargetArmLength
			+ (actionValue * -300.0f);

		newCamArmLen = FMath::Clamp(newCamArmLen, currDrone->EYEMinZoom, currDrone->EYEMaxZoom);
		currDrone->EYECamZoom = newCamArmLen;
		currDrone->DroneEyeSpringArm->TargetArmLength = newCamArmLen;
		break;

	case EDroneCameraType::Front:
		newCamArmLen = currDrone->FrontSpringArm->TargetArmLength
			+ (actionValue * -300.0f);

		newCamArmLen = FMath::Clamp(newCamArmLen, currDrone->FrontMinZoom, currDrone->FrontMaxZoom);
		currDrone->FrontCamZoom = newCamArmLen;
		currDrone->FrontSpringArm->TargetArmLength = newCamArmLen;
		break;
	}
}
